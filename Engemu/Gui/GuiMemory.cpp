#include "GuiMemory.h"
#include "../Memory.h"

GuiMemory::GuiMemory(Memory& mem_, std::string& additional_title) : mem(mem_), Gui(additional_title, 600, 600) {}

bool GuiMemory::render() {
	glfwMakeContextCurrent(window);

	if (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
		ImGui_ImplGlfwGL3_NewFrame();

		ImGuiStyle& style = ImGui::GetStyle();
		style.WindowTitleAlign.x = 0.5;
		style.WindowRounding = 0.0;

		if (true) {
			ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiSetCond_Always);
			ImGui::SetNextWindowSize(ImVec2((float)window_width, (float)window_height), ImGuiSetCond_Always);
			memory_editor.Draw("Code And Constant Section", mem.ram.data(), mem.ram.size(), 0x6000'0000);
		}


		ImGui::SetNextWindowPos(ImVec2(795, 10), ImGuiSetCond_FirstUseEver);
		ImGui::SetNextWindowSize(ImVec2(478, 475), ImGuiSetCond_FirstUseEver);



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