#pragma once
#include "WkWindow.h"
#include "Helper.h"

class Input
{
public:
	static Input& GetInstance()
	{
		static Input    instance; // Guaranteed to be destroyed.
		// Instantiated on first use.
		return instance;
	}

	//Input(WkWindow* wWindow);
	~Input();

	Input(Input const&) = delete;
	void operator=(Input const&) = delete;

	void Update();
	void EndOfFrame();

	double GetMouseX() { return mouseY; };
	double GetMouseY() { return mouseX; };
	double GetMouseXDelta() { return mouseXDelta; };
	double GetMouseYDelta() { return mouseYDelta; };
	//float GetMouseWheel() { return wheelDelta; };

	bool MouseLeftDown();
	bool MouseRightDown();
	bool MouseMiddleDown();

	bool KeyDown(int key);
	bool KeyUp(int key);

	bool KeyPress(int key);
	bool KeyRelease(int key);

	bool GetKeyArray(bool* keyArray, int size = 256);

	void SetGuiKeyboardCapture(bool capture) { guiWantsKeyboard = capture; }
	void SetGuiMouseCapture(bool capture) { guiWantsMouse = capture; }

	void InitInput(WkWindow* window);

private:
	Input() {};
	// Mouse position and wheel data
	double mouseX;
	double mouseY;
	double prevMouseX;
	double prevMouseY;
	double mouseXDelta;
	double mouseYDelta;

	WkWindow* wWindow;

	bool guiWantsKeyboard;
	bool guiWantsMouse;
};

static unsigned char kbState[sizeof(unsigned char) * 256];
static unsigned char prevKbState[sizeof(unsigned char) * 256];

static unsigned char mouseState[sizeof(unsigned char) * 256];
static unsigned char prevMouseState[sizeof(unsigned char) * 256];
static double wheelDelta;

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	//if (action == GLFW_PRESS)
	//{
		kbState[key] = '1';
		Helper::Cout("[Input]Pressed:" + std::to_string(key));
	//}
}

static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	if (action == GLFW_PRESS)
	{
		mouseState[button] = '1';
		Helper::Cout("[Input]Mouse Press:" + std::to_string(button));
	}
	else if (action == GLFW_RELEASE)
	{
		mouseState[button] = '0';
		Helper::Cout("[Input]Mouse Release:" + std::to_string(button));
	}
}
static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	wheelDelta = yoffset;
}