#include "WkWindow.h"

WkWindow::WkWindow(unsigned int w, unsigned int h, string name): width(w), height(h), windowName(name)
{
	InitWindow();
}

WkWindow::~WkWindow()
{
	glfwDestroyWindow(window);
	glfwTerminate();
}

void WkWindow::InitWindow()
{
	Helper::Cout("WkWindow", true);
	if (!glfwInit())
	{
		throw std::runtime_error("failed to initalize window named: " + windowName);
	}

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_NO_API);

	//Monitor nullptr can be changed for fullscreen mode
	Helper::Cout("Created Window: " + windowName);
	window = glfwCreateWindow(width, height, windowName.c_str(), nullptr, nullptr);
	glfwSetWindowUserPointer(window, this);
	Helper::Cout("Window Created");
}

void WkWindow::SetVCore(VulkanCore* core)
{
	this->vCore = core;
}

GLFWwindow* WkWindow::GetWindow()
{
	return this->window;
}

string WkWindow::GetWindowName()
{
	return this->windowName;
}
