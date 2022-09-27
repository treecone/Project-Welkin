#include "ImGUI.h"
/*
#include "VulkanCore.h"
#include "WkWindow.h"

ImGUI::ImGUI(VulkanCore* vCore, WkWindow* wWindow, Input* input) : vCore{vCore}, wWindow{wWindow}, input{input}
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();

	ImGuiIO& io = ImGui::GetIO(); (void)io;

	#pragma region Creating Descriptor Pool
		VkDescriptorPoolSize pool_sizes[] =
		{
			{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
		};

		VkDescriptorPoolCreateInfo pool_info = {};
		pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		pool_info.maxSets = 1000;
		pool_info.poolSizeCount = std::size(pool_sizes);
		pool_info.pPoolSizes = pool_sizes;

		if (vkCreateDescriptorPool(*vCore->GetLogicalDevice(), &pool_info, nullptr, &imguiPool) != VK_SUCCESS)
		{
			throw std::exception("Failed to create descriptor set for IMGUI");
		}
	#pragma endregion

	ImGui_ImplGlfw_InitForVulkan(wWindow->GetWindow(), true);
	ImGui_ImplVulkan_InitInfo vInfo = {};
	vInfo.Instance = *vCore->GetInstance();
	vInfo.PhysicalDevice = *vCore->GetPhysicalDevice();
	vInfo.Device = *vCore->GetLogicalDevice();
	vInfo.Queue = *vCore->GetQueue(0);
	vInfo.DescriptorPool = imguiPool;
	vInfo.MinImageCount = 2;
	vInfo.ImageCount = vCore->GetFramebufferCount();
	vInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

	ImGui_ImplVulkan_Init(&vInfo, *vCore->GetRenderPass());
}

void ImGUI::CreateImRenderPass()
{
	VkAttachmentDescription attachment = {};
	attachment.format = vCore->GetSwapchainImageFormat();
	attachment.samples = VK_SAMPLE_COUNT_1_BIT;
	attachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
	attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference color_attachment = {};
	color_attachment.attachment = 0;
	color_attachment.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &color_attachment;

	//TODO https://frguthmann.github.io/posts/vulkan_imgui/ at the depednacy part 
}

ImGUI::~ImGUI()
{
	ImGui_ImplVulkan_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	vkDestroyDescriptorPool(*vCore->GetLogicalDevice(), imguiPool, nullptr);
}

void ImGUI::Update(float deltaTime)
{
	// Set io info
	ImGuiIO& io = ImGui::GetIO();

	io.DeltaTime = deltaTime;
	io.DisplaySize.x = width;
	io.DisplaySize.y = height;
	
	io.KeyCtrl = input->KeyDown(VK_CONTROL);
	io.KeyShift = input->KeyDown(VK_SHIFT);
	io.KeyAlt = input->KeyDown(VK_MENU);
	io.MousePos.x = (float)input->GetMouseX();
	io.MousePos.y = (float)input->GetMouseY();

	//TODO check if this is right
	io.MouseDown[0] = input->MouseLeftDown();
	io.MouseDown[1] = input->MouseRightDown();
	io.MouseDown[2] = input->MouseMiddleDown();
	io.MouseWheel = input->GetMouseWheel();
	//input->GetKeyArray(io.KeysDown, 256);
	

	// Reset the frame
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	// Determine new input capture (you’ll uncomment later)
	input->SetGuiKeyboardCapture(io.WantCaptureKeyboard);
	input->SetGuiMouseCapture(io.WantCaptureMouse);

	// Show the demo window
	ImGui::ShowDemoWindow();

}
*/