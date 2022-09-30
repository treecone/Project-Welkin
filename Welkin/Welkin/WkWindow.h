#pragma once
#define GLFW_INCLUDE_VULKAN
#include <string>
#include <GLFW/glfw3.h>
#include <iostream>
#include "Helper.h"

using namespace std;

class WkWindow
{
public:
	WkWindow(unsigned int w, unsigned int h, string name);
	~WkWindow();
	void InitWindow();
	GLFWwindow* GetWindow();
	string GetWindowName();
	bool shouldClose() {return glfwWindowShouldClose(window);};
	glm::vec2 GetWidthHeight() { return glm::vec2(width, height); }
	glm::vec2 GetMousePos();

private:
	unsigned int width, height;
	string windowName;
	GLFWwindow* window;
};