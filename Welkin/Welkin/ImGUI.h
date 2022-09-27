#pragma once

//#include <imgui.h>
//#include <backends/imgui_impl_glfw.h>
//#include <backends/imgui_impl_vulkan.h>
#include "Input.h"
#include <array>

#define VK_CONTROL 0x11
#define VK_SHIFT 0x10
#define VK_MENU 0x12

class VulkanCore;
class WkWindow;

class ImGUI
{
public:
	ImGUI(VulkanCore* vCore, WkWindow* wWindow, Input* input);
	~ImGUI();

	void Update(float deltaTime);
private:
	VulkanCore* vCore;
	WkWindow* wWindow;
	Input* input;
	VkDescriptorPool imguiPool;
	VkRenderPass ImGuiRenderPass;
	unsigned int width, height;

	void CreateImRenderPass();
};