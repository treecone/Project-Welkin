#pragma once
#include <string>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <iostream>
#include "VulkanCore.h"
#include "Helper.h"

using namespace std;

class WkWindow
{
public:
	WkWindow(unsigned int w, unsigned int h, string name);
	~WkWindow();
	void InitWindow();
	void SetVCore(VulkanCore* core);
	GLFWwindow* GetWindow();
	string GetWindowName();
	bool shouldClose() {return glfwWindowShouldClose(window);};
	vec2 GetWidthHeight() { return vec2(width, height); }
	vec2 GetMousePos();

#pragma region Prevents copying the pointer and then having two pointers, deleting only one
	WkWindow(const WkWindow&) = delete;
	WkWindow& operator=(const WkWindow&) = delete;
#pragma endregion


private:
	unsigned int width, height;
	string windowName;
	VulkanCore* vCore;
	GLFWwindow* window;

	static void FrameBufferResizeCallback(GLFWwindow* window, int width, int height);
};

