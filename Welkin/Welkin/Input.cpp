#include "Input.h"

void Input::InitInput(WkWindow* window)
{
	this->wWindow = window;

	memset(kbState, 0, sizeof(unsigned char) * 256);
	memset(prevKbState, 0, sizeof(unsigned char) * 256);

	glfwSetKeyCallback(wWindow->GetWindow(), key_callback);
	glfwSetMouseButtonCallback(wWindow->GetWindow(), mouse_button_callback);
	glfwSetScrollCallback(wWindow->GetWindow(), scroll_callback);

	wheelDelta = 0.0f;
	mouseX = 0; mouseY = 0;
	prevMouseX = 0; prevMouseY = 0;
	mouseXDelta = 0; mouseYDelta = 0;

	guiWantsKeyboard = false;
	guiWantsMouse = false;
}

Input::~Input()
{
	//TODO do you need this with the auto setup I have enabled?
	//_CrtDumpMemoryLeaks();
}


void Input::Update()
{
	// Copy the old keys so we have last frame's data
	memcpy(prevKbState, kbState, sizeof(unsigned char) * 256);
	memcpy(prevMouseState, mouseState, sizeof(unsigned char) * 256);

	// Save the previous mouse position, then the current mouse 
	// position and finally calculate the change from the previous frame
	prevMouseX = mouseX;
	prevMouseY = mouseY;

	// Get the current mouse position then make it relative to the window
	glfwGetCursorPos(wWindow->GetWindow(), &mouseX, &mouseY);

	mouseXDelta = mouseX - prevMouseX;
	mouseYDelta = mouseY - prevMouseY;
}

// ----------------------------------------------------------
//  Resets the mouse wheel value at the end of the frame.
//  This cannot occur earlier in the frame, since the wheel
//  input comes from Win32 windowing messages, which are
//  handled between frames.
// ----------------------------------------------------------
void Input::EndOfFrame()
{
	// Reset wheel value
	wheelDelta = 0;

	for (auto& c : kbState)
		c = 0;
}

// 'a' or ' VK_ESCAPE or VK_SHIFT'
bool Input::KeyDown(int key)
{
	if (key < 0 || key > 255) return false;
	//TODO use bitwise to calculate faster
	return kbState[key] != 0 && !guiWantsKeyboard;
}


bool Input::KeyUp(int key)
{
	if (key < 0 || key > 255) return false;

	return kbState[key] == 0 && !guiWantsKeyboard;
}

bool Input::KeyPress(int key)
{
	if (key < 0 || key > 255) return false;

	return
		kbState[key] != 0 &&			// Down now
		prevKbState[key] == 0		// Up last frame
		&& !guiWantsKeyboard;
}

bool Input::KeyRelease(int key)
{
	if (key < 0 || key > 255) return false;

	return
		kbState[key] == 0 &&	// Up now
		prevKbState[key] != 0	// Down last frame
		&& !guiWantsKeyboard;
}


// ----------------------------------------------------------
//  A utility function to fill a given array of booleans 
//  with the current state of the keyboard.  This is most
//  useful when hooking the engine's input up to another
//  system, such as a user interface library.
// 
//  keyArray - pointer to a boolean array which will be
//             filled with the current keyboard state
//  size - the size of the boolean array (up to 256)
// 
//  Returns true if the size parameter was valid and false
//  if it was <= 0 or > 256
// ----------------------------------------------------------
bool Input::GetKeyArray(bool* keyArray, int size)
{
	if (size <= 0 || size > 256) return false;

	// Loop through the given size and fill the
	// boolean array.  Note that the double exclamation
	// point is on purpose; it's a quick way to
	// convert any number to a boolean.
	for (int i = 0; i < size; i++)
		keyArray[i] = !!(kbState[i] & 0x80);

	return true;
}

bool Input::MouseLeftDown() { return (kbState[1] & 0x80) != 0 && !guiWantsMouse; }
bool Input::MouseRightDown() { return (kbState[2] & 0x80) != 0 && !guiWantsMouse; }
bool Input::MouseMiddleDown() { return (kbState[4] & 0x80) != 0 && !guiWantsMouse; }