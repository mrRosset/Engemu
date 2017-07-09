#pragma once

#include "../imgui/imgui.h"
#include "../imgui/imgui_impl_glfw_gl3.h"
#include <stdio.h>
#include <cstdlib>
#include <string>
#include <GL/gl3w.h>
#include <GLFW/glfw3.h>

class Gui {
protected:
	GLFWwindow* window;
	ImVec4 clear_color = ImColor(114, 144, 154);

public:
	Gui(std::string additional_title);
	~Gui();
	virtual bool render() = 0;
	void imgui_print_hex(uint8_t u);
	void imgui_print_hex(uint16_t u);
	void imgui_print_hex(uint32_t u);
	void imgui_print_hex(int32_t i);
	void imgui_print_hex(int64_t i);

};