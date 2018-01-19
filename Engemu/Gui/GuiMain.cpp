#include <fstream>
#include <iostream>
#include "GuiMain.h"

#include "MemoryEditor.h"
#include "../CPU/Tharm/CPU.h"
#include "../CPU/Tharm/Decoder/Decoder.h"
#include "../CPU/Tharm/Disassembler/Disassembler.h"
#include "../CPU/Tharm/Decoder/IR.h"
#include "../CPU/Tharm/Utils.h"
#include "../Symbols/SymbolsManager.h"


GuiMain::GuiMain(CPU* cpu_, std::string& additional_title) : Gui(additional_title, 815, 920) {
	cpu = cpu_;
}

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
			ImGui::Begin("CPU", &show_cpu_window, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse  | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
			render_cpu();
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
	bool thumb = cpu->cpsr.flag_T;
	u8 instruction_bytes = thumb ? 2 : 4;

	ImGui::BeginChild("MemoryNav", ImVec2(800, 50), false);
	render_memoryNav();
	ImGui::EndChild();

	ImGui::BeginChild("Controls", ImVec2(800, 35), true);
	bool scroll_to_pc = render_controls();
	ImGui::EndChild();

	ImGui::BeginChild("Disassembly", ImVec2(592, 700), true);
	render_disassembly(scroll_to_pc);
	ImGui::EndChild();

	ImGui::SameLine();

	ImGui::BeginGroup();
	
	ImGui::BeginChild("Registers", ImVec2(200, 420), true);
	render_registers();
	ImGui::EndChild();

	/*ImGui::BeginChild("Stack", ImVec2(200, 200), true);
	render_stack();
	ImGui::EndChild();*/

	ImGui::EndGroup();

	ImGui::BeginChild("Call Stack", ImVec2(800, 105), true);
	render_call_stack();
	ImGui::EndChild();
}

void GuiMain::render_memoryNav() {

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

	float cursor_x = p.x + u64(cpu->gprs.RealPC()) / float(0xFFFF'FFFF) * 800.0f;
	float cursor_y = p.y + 19;
	draw_list->AddTriangleFilled(ImVec2(cursor_x, cursor_y), ImVec2(cursor_x - 4, cursor_y + 7), ImVec2(cursor_x + 4, cursor_y + 7), ImColor(255, 255, 255, 255));
}

bool GuiMain::render_controls() {
	if (ImGui::Button("Run"))
	{
		cpu->state = CPU::State::Running;
	}

	ImGui::SameLine();

	if (ImGui::Button("Pause"))
	{
		cpu->state = CPU::State::Stopped;
	}

	ImGui::SameLine();

	if (ImGui::Button("Step"))
	{
		cpu->state = CPU::State::Step;
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

	ImGui::SameLine();

	ImGui::Text(" %.3f ms/frame (%.1f fps)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);


	return scroll_to_pc;
}

void GuiMain::render_disassembly(bool scroll_to_pc) {
	bool thumb = cpu->cpsr.flag_T;
	u8 instruction_bytes = thumb ? 2 : 4;

	ImGui::Columns(4, "Disassembly");
	ImGui::SetColumnOffset(1, 26);
	ImGui::SetColumnOffset(2, 108);
	ImGui::SetColumnOffset(3, thumb ? 158.f : 200.f);
	ImGui::Text(""); ImGui::NextColumn(); // To indicate a breakpoint
	ImGui::Text("Address"); ImGui::NextColumn();
	ImGui::Text("Bytes"); ImGui::NextColumn();
	ImGui::Text("Instruction"); ImGui::NextColumn();
	ImGui::Separator();

	u32 number_instructions_displayed = 0x2000;
	u32 offset = (s32(cpu->gprs.RealPC()) - s32(number_instructions_displayed / 2)) < 0 ? 0 : cpu->gprs.RealPC() - (number_instructions_displayed / 2);

	ImGuiListClipper clipper(number_instructions_displayed / instruction_bytes, ImGui::GetTextLineHeight()); // Bytes are grouped by four (the alignment for instructions
	
	/*ImDrawList* draw_list = ImGui::GetWindowDrawList();
	ImColor breakpoint_fill = ImColor(ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
	ImColor breakpoint_border = ImColor(ImVec4(1.0f, 1.0f, 1.0f, 1.0f));*/
	ImVec2 screen_cursor = ImGui::GetCursorScreenPos();

	// Perform scrolling, if necessary
	if (track_pc || scroll_to_pc)
	{
		ImGui::SetScrollFromPosY((((cpu->gprs.RealPC() - offset) / instruction_bytes) * ImGui::GetTextLineHeight()) - ImGui::GetScrollY(), 0.35f);
	}

	int label_lines = 0;

	for (s32 i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
	{
		u32 cur_address = (i - label_lines) * instruction_bytes + offset ;

		screen_cursor = ImGui::GetCursorScreenPos();

		u32 symbol_addr = cur_address | (thumb ? 1 : 0);
		//Print symbols if found
		if (Symbols::hasFunctionName(symbol_addr)) {
			//empty line
			ImGui::Selectable("", false, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowDoubleClick);
			ImGui::NextColumn();
			ImGui::Text("0x%X", cur_address); ImGui::NextColumn();
			ImGui::Text(""); ImGui::NextColumn();
			ImGui::Text(""); ImGui::NextColumn();

			//line with function name
			ImGui::Selectable("", false, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowDoubleClick);
			ImGui::NextColumn();
			ImGui::Text("0x%X", cur_address); ImGui::NextColumn();
			ImGui::Text(""); ImGui::NextColumn();
			ImGui::Text(Symbols::getFunctionName(symbol_addr).c_str());  ImGui::NextColumn();
			label_lines+=2;
			i+=2;
		}

		if (ImGui::Selectable("", cpu->gprs.RealPC() == cur_address, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowDoubleClick))
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
				ImGui::Text("%02X %02X", cpu->mem.read8(cur_address + 1), cpu->mem.read8(cur_address));
			}
			catch (...) {} // when in invalid memory space
		}
		else
		{
			try {
				ImGui::Text("%02X %02X %02X %02X", cpu->mem.read8(cur_address + 3), cpu->mem.read8(cur_address + 2), cpu->mem.read8(cur_address + 1), cpu->mem.read8(cur_address));
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
				Decoder::Decode(ir, cpu->mem.read16(cur_address));
				text = Disassembler::Disassemble(ir);
			}
			catch (...) {}
			ImGui::Text("%s", text.c_str());
		}
		else
		{
			IR_ARM ir;
			ir.instr = AInstructions::SWI;
			std::string text = "";
			try {
				Decoder::Decode(ir, cpu->mem.read32(cur_address));
				text = Disassembler::Disassemble(ir);
			}
			catch (...) {}
			ImGui::Text("%s", text.c_str());
		}

		ImGui::NextColumn();

	}

	clipper.End();

}

void GuiMain::render_registers() {
	for (u8 i = 0; i < 0xF; i++)
	{
		ImGui::Text("%s: ", Disassembler::Disassemble_Reg(i).c_str());
		ImGui::SameLine(38);
		ImGui::Text("0x%X", cpu->gprs[i] + 0);
	}

	ImGui::Text("%s: ", Disassembler::Disassemble_Reg(0xF).c_str());
	ImGui::SameLine(38);
	ImGui::Text("0x%X", cpu->gprs.RealPC() + 0);

	ImGui::Text("CPSR: 0x%X", PSR_to_u32(cpu->cpsr));
	ImGui::Text("N : %s", cpu->cpsr.flag_N ? "true" : "false");
	ImGui::Text("Z : %s", cpu->cpsr.flag_Z ? "true" : "false");
	ImGui::Text("C : %s", cpu->cpsr.flag_C ? "true" : "false");
	ImGui::Text("V : %s", cpu->cpsr.flag_V ? "true" : "false");
	ImGui::Text("I : %s", cpu->cpsr.flag_inter_I ? "true" : "false");
	ImGui::Text("F : %s", cpu->cpsr.flag_inter_F ? "true" : "false");
	ImGui::Text("T : %s", cpu->cpsr.flag_T ? "true" : "false");

}

void GuiMain::render_stack() {
	bool thumb = cpu->cpsr.flag_T;
	u8 instruction_bytes = thumb ? 2 : 4;

	ImGui::Columns(2, "Stack");
	ImGui::SetColumnOffset(1, 72);
	ImGui::Text("Address"); ImGui::NextColumn();
	ImGui::Text("Bytes"); ImGui::NextColumn();
	ImGui::Separator();

	ImGuiListClipper stack_clipper((0x1000000 - cpu->gprs[Regs::SP]) / instruction_bytes, ImGui::GetTextLineHeight()); // Bytes are grouped by four (the alignment for instructions)

	for (s32 i = stack_clipper.DisplayStart; i < stack_clipper.DisplayEnd; i++)
	{
		s32 clipper_i = i;
		i *= 4;
		i = 0xFFFFFC - i;

		ImGui::Text("0x%X", i); ImGui::NextColumn();
		ImGui::Text("%02X %02X %02X %02X", cpu->mem.read8(i + 3), cpu->mem.read8(i + 2), cpu->mem.read8(i + 1), cpu->mem.read8(i));
		ImGui::NextColumn();

		i = clipper_i;
	}

	stack_clipper.End();

}

void GuiMain::render_call_stack() {
	ImGuiListClipper clipper(cpu->call_stack.size(), ImGui::GetTextLineHeight()); // Bytes are grouped by four (the alignment for instructions

	ImGui::Columns(2, "Call Stack");
	ImGui::SetColumnOffset(1, 50);
	ImGui::Text("#"); ImGui::NextColumn();
	ImGui::Text("Address"); ImGui::NextColumn();
	ImGui::Separator();

	for (int i = clipper.DisplayEnd -1 ; i >= clipper.DisplayStart; i--)
	{
		ImGui::Selectable(std::to_string(i).c_str(), false, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowDoubleClick);
		ImGui::NextColumn();
		if (i < cpu->call_stack.size()) {
			ImGui::Text(cpu->call_stack[i].c_str());
		}
		else {
			ImGui::Text("");
		}
		ImGui::NextColumn();
	}
	///ImGui::Selectable(" ", false, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowDoubleClick);
	//ImGui::NextColumn();

	clipper.End();

}