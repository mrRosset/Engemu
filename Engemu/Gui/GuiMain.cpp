#include "GuiMain.h"

#include "MemoryEditor.h"
#include "../CPU/CPU.h"
#include "../CPU/Decoder/Decoder.h"
#include "../CPU/Disassembler/Disassembler.h"
#include "../CPU/Decoder/IR.h"


GuiMain::GuiMain(CPU & cpu_, std::string& additional_title): Gui(additional_title), cpu(cpu_) {}

bool GuiMain::render() {
	if (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
		ImGui_ImplGlfwGL3_NewFrame();

		ImGuiStyle& style = ImGui::GetStyle();
		style.WindowTitleAlign.x = 0.5;
		style.WindowRounding = 0.0;

		if (show_cpu_window) {
			ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiSetCond_Always);
			ImGui::SetNextWindowSize(ImVec2((float)window_width, (float)window_height), ImGuiSetCond_Always);
			ImGui::Begin("CPU", &show_cpu_window, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
			ImGui::GetWindowHeight();
			render_cpu();
			ImGui::End();
		}

		if (show_memory_window) {
			ImGui::SetNextWindowPos(ImVec2(10, 530), ImGuiSetCond_FirstUseEver);
			ImGui::SetNextWindowSize(ImVec2(240, 240), ImGuiSetCond_FirstUseEver);
			ImGui::Begin("Memory", &show_memory_window);
			//render_flags_window(image.header);
			ImGui::End();
		}


		//ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiSetCond_FirstUseEver);
		//ImGui::ShowTestWindow(&show_test_window);

		// Rendering
		int display_w, display_h;
		glfwGetFramebufferSize(window, &display_w, &display_h);
		glViewport(0, 0, display_w, display_h);
		glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
		glClear(GL_COLOR_BUFFER_BIT);
		ImGui::Render();
		glfwSwapBuffers(window);

		return true;
	}
	else {
		return false;
	}
}

void GuiMain::render_cpu() {
	bool thumb = cpu.cpsr.flag_T;
	u8 instruction_bytes = thumb ? 2 : 4;

	bool display_disassembly = true;

	ImGui::BeginChild("MemoryNav", ImVec2(800, 50), false);
	ImDrawList* draw_list = ImGui::GetWindowDrawList();
	ImGuiStyle& style = ImGui::GetStyle();
	ImColor col = style.Colors[ImGuiCol_Border];
	const ImVec2 p = ImGui::GetCursorScreenPos();
	
	draw_list->AddLine(ImVec2(p.x, p.y + 17), ImVec2(p.x + 800, p.y + 17), col, 1);
	draw_list->AddLine(ImVec2(p.x, p.y + 10), ImVec2(p.x, p.y + 24), col, 1);
	draw_list->AddLine(ImVec2(p.x + 799, p.y + 10), ImVec2(p.x + 799, p.y + 24), col, 1);

	draw_list->AddText(ImVec2(p.x, p.y + 30), ImColor(255, 255, 255, 255), "0x0");
	draw_list->AddText(ImVec2(p.x + 720, p.y + 30), ImColor(255, 255, 255, 255), "0xFFFF'FFFF");
	
	
	//draw_list->AddLine(ImVec2(p.x + 150, p.y + 10), ImVec2(p.x + 150, p.y + 24), col, 1);
	draw_list->AddLine(ImVec2(p.x + 250, p.y + 10), ImVec2(p.x + 250, p.y + 24), col, 1);
	draw_list->AddLine(ImVec2(p.x + 275, p.y + 10), ImVec2(p.x + 275, p.y + 24), col, 1);
	draw_list->AddText(ImVec2(p.x + 252, p.y - 3), ImColor(255, 255, 255, 255), "Rom");

	float cursor_x = p.x + u64(cpu.gprs[Regs::PC]) / float(0xFFFF'FFFF) * 800.0f;
	float cursor_y = p.y + 19;
	draw_list->AddTriangleFilled(ImVec2(cursor_x, cursor_y), ImVec2(cursor_x - 4, cursor_y + 7), ImVec2(cursor_x + 4, cursor_y + 7), ImColor(255, 255, 255, 255));

	ImGui::EndChild();

	ImGui::BeginChild("Controls", ImVec2(800, 35), true);

	// Controls
	if (ImGui::Button("Run"))
	{
		//emulator.emulating = true;
	}

	ImGui::SameLine();

	if (ImGui::Button("Pause"))
	{
		//emulator.emulating = false;
	}

	ImGui::SameLine();

	if (ImGui::Button("Step"))
	{
		cpu.Step();
	}

	ImGui::SameLine();

	// The scrolling needs to be performed later, once we have already created the disassembly window
	bool scroll_to_pc = false;

	if (ImGui::Button("Go to PC"))
	{
		scroll_to_pc = true;
	}

	ImGui::SameLine();

	ImGui::Checkbox("Track PC", &track_pc);

	ImGui::EndChild();
	ImGui::BeginChild("Disassembly", ImVec2(592, 720), true);

	ImGui::Columns(4, "Disassembly");
	ImGui::SetColumnOffset(1, 26);
	ImGui::SetColumnOffset(2, 98);
	ImGui::SetColumnOffset(3, thumb ? 148.f : 190.f);
	ImGui::Text(""); ImGui::NextColumn(); // To indicate a breakpoint
	ImGui::Text("Address"); ImGui::NextColumn();
	ImGui::Text("Bytes"); ImGui::NextColumn();
	ImGui::Text("Instruction"); ImGui::NextColumn();
	ImGui::Separator();

	u32 number_instructions_displayed = 0x1000000;
	u32 offset = (s32(cpu.gprs[Regs::PC]) - s32(number_instructions_displayed / 2)) < 0 ? 0 : cpu.gprs[Regs::PC] - (number_instructions_displayed / 2);

	ImGuiListClipper clipper(number_instructions_displayed / instruction_bytes, ImGui::GetTextLineHeight()); // Bytes are grouped by four (the alignment for instructions)
	//ImDrawList* draw_list = ImGui::GetWindowDrawList();
	ImColor breakpoint_fill = ImColor(ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
	ImColor breakpoint_border = ImColor(ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
	ImVec2 screen_cursor = ImGui::GetCursorScreenPos();

	// Perform scrolling, if necessary
	if (track_pc || scroll_to_pc)
	{
		ImGui::SetScrollFromPosY((( (cpu.gprs[Regs::PC] - offset) / instruction_bytes) * ImGui::GetTextLineHeight()) - ImGui::GetScrollY(), 0.35f);
	}

	for (s32 i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
	{
		u32 cur_address = i * instruction_bytes + offset;

		screen_cursor = ImGui::GetCursorScreenPos();

		if (ImGui::Selectable("", cpu.gprs[Regs::PC] == cur_address, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowDoubleClick))
		{
			//emulator.cpu->breakpoint = cur_address;
		}

		/*if (emulator.cpu->breakpoint == cur_address)
		{
			// We use custom rendering for drawing the breakpoint
			draw_list->AddCircleFilled(ImVec2(screen_cursor.x + 6, screen_cursor.y + 7), 7, breakpoint_fill);
			draw_list->AddCircle(ImVec2(screen_cursor.x + 6, screen_cursor.y + 7), 7, breakpoint_border);
		}*/

		ImGui::NextColumn();
		ImGui::Text("0x%X", cur_address); ImGui::NextColumn();

		if (thumb)
		{
			try {
				ImGui::Text("%02X %02X", cpu.mem.read8(cur_address + 1), cpu.mem.read8(cur_address));
			}
			catch (...) {} // when in invalid memory space
		}
		else
		{
			try {
				ImGui::Text("%02X %02X %02X %02X", cpu.mem.read8(cur_address + 3), cpu.mem.read8(cur_address + 2), cpu.mem.read8(cur_address + 1), cpu.mem.read8(cur_address));
			}
			catch (...) {} // when in invalid memory space
		}

		ImGui::NextColumn();

		if (thumb)
		{

			IR_Thumb ir;
			ir.instr = TInstructions::SWI;
			std::string text = "";
			try {
				Decoder::Decode(ir, cpu.mem.read16(cur_address));
				text = Disassembler::Disassemble(ir);
			} catch(...){}
			ImGui::Text("%s", text.c_str());
		}
		else
		{
			IR_ARM ir;
			ir.instr = AInstructions::SWI;
			std::string text = "";
			try {
				Decoder::Decode(ir, cpu.mem.read32(cur_address));
				text = Disassembler::Disassemble(ir);
			}
			catch (...) {}
			ImGui::Text("%s", text.c_str());
		}

		ImGui::NextColumn();

	}

	clipper.End();
	ImGui::EndChild();
	ImGui::SameLine();
	ImGui::BeginGroup();
	ImGui::BeginChild("Registers", ImVec2(200, 300), true);

	for (u8 i = 0; i <= 0xF; i++)
	{
		ImGui::Text("%s: ", Disassembler::Disassemble_Reg(i).c_str());
		ImGui::SameLine(38);
		ImGui::Text("0x%X", cpu.gprs[i]);
	}

	ImGui::Text("CPSR: 0x%X", cpu.cpsr);

	ImGui::EndChild();
	ImGui::BeginChild("Stack", ImVec2(200, 200), true);

	ImGui::Columns(2, "Stack");
	ImGui::SetColumnOffset(1, 72);
	ImGui::Text("Address"); ImGui::NextColumn();
	ImGui::Text("Bytes"); ImGui::NextColumn();
	ImGui::Separator();

	ImGuiListClipper stack_clipper((0x1000000 - cpu.gprs[Regs::SP]) / instruction_bytes, ImGui::GetTextLineHeight()); // Bytes are grouped by four (the alignment for instructions)

	for (s32 i = stack_clipper.DisplayStart; i < stack_clipper.DisplayEnd; i++)
	{
		s32 clipper_i = i;
		i *= 4;
		i = 0xFFFFFC - i;

		ImGui::Text("0x%X", i); ImGui::NextColumn();
		ImGui::Text("%02X %02X %02X %02X", cpu.mem.read8(i + 3), cpu.mem.read8(i + 2), cpu.mem.read8(i + 1), cpu.mem.read8(i));
		ImGui::NextColumn();

		i = clipper_i;
	}

	stack_clipper.End();
	ImGui::EndChild();
	ImGui::EndGroup();
}