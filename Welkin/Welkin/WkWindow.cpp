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
	if (!glfwInit())
	{
		throw std::runtime_error("failed to initalize window named: " + windowName);
	}

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	//Monitor nullptr can be changed for fullscreen mode
	cout << "Created Window: " + windowName << endl;
	window = glfwCreateWindow(width, height, windowName.c_str(), nullptr, nullptr);
}

string WkWindow::GetWindowName()
{
	return this->windowName;
}
