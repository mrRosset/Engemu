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
	static bool initialized;
	GLFWwindow* window;
	int window_width, window_height;
	ImVec4 clear_color = ImColor(114, 144, 154);

public:
	Gui(std::string& additional_title, int width, int height);
	~Gui();
	void Resize_callback(GLFWwindow * window, int width, int height);
	virtual bool render() = 0;
};