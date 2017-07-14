#include "Gui.h"
#include <string>
#include <iostream>
#include <sstream>
#include <inttypes.h>


static void error_callback(int error, const char* description)
{
	fprintf(stderr, "Error %d: %s\n", error, description);
}

Gui::Gui(std::string& additional_title) {
	// Setup window
	glfwSetErrorCallback(error_callback);
	if (!glfwInit()) {
		exit(-1);
	}
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
#if __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	window_width = 815;
	window_height = 830;

	window = glfwCreateWindow(window_width, window_height, ("Engemu - " + additional_title).c_str(), NULL, NULL);
	//center windows
	const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
	int window_width, window_height;
	glfwGetWindowSize(window, &window_width, &window_height);
	glfwSetWindowPos(window, (mode->width - window_width)/2, (mode->height - window_height) / 2);

	glfwMakeContextCurrent(window);
	gl3wInit();

	// Setup ImGui binding
	ImGui_ImplGlfwGL3_Init(window, true);
}

Gui::~Gui() {
	ImGui_ImplGlfwGL3_Shutdown();
	glfwTerminate();
}