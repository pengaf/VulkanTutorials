#include "Tutorial02.h"

#include<qmessagebox.h>
#include <QAbstractEventDispatcher>
#include <QDebug>
#include <vector>
#include <fstream>
#include <iostream>

std::vector<char> GetBinaryFileContents(char const* filename) {

	std::ifstream file(filename, std::ios::binary);
	if (file.fail()) {
		std::cout << "Could not open \"" << filename << "\" file!" << std::endl;
		return std::vector<char>();
	}

	std::streampos begin, end;
	begin = file.tellg();
	file.seekg(0, std::ios::end);
	end = file.tellg();

	std::vector<char> result(static_cast<size_t>(end - begin));
	file.seekg(0, std::ios::beg);
	file.read(&result[0], end - begin);
	file.close();

	return result;
}

bool CheckExtensionAvailability(const char* desired, std::vector<VkExtensionProperties>& availableExtensions)
{
	for (VkExtensionProperties& extension : availableExtensions)
	{
		if (strcmp(desired, extension.extensionName) == 0)
		{
			return true;
		}
	}
	return false;
}

bool CheckPhysicalDevice(uint32_t& graphicsQueueFamilyIndex, uint32_t& presentQueueFamilyIndex, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
{
	VkResult result;
	uint32_t extensionCount;
	result = vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr);
	if (result != VK_SUCCESS || extensionCount == 0)
	{
		return false;
	}
	std::vector<VkExtensionProperties> extensions(extensionCount);
	result = vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, extensions.data());
	if (result != VK_SUCCESS)
	{
		return false;
	}

	const char* desiredDeviceExtensions[]=
	{
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	for (auto desired : desiredDeviceExtensions)
	{
		if (!CheckExtensionAvailability(desired, extensions))
		{
			QMessageBox::critical(nullptr, "error", QString("could not find extension ") + desired);
			return false;
		}
	}

	VkPhysicalDeviceProperties physicalDeviceProperties;
	VkPhysicalDeviceFeatures physicalDeviceFeatures;
	vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);
	vkGetPhysicalDeviceFeatures(physicalDevice, &physicalDeviceFeatures);
	uint32_t major_version = VK_API_VERSION_MAJOR(physicalDeviceProperties.apiVersion);
	uint32_t minor_version = VK_API_VERSION_MINOR(physicalDeviceProperties.apiVersion);
	uint32_t patch_version = VK_API_VERSION_PATCH(physicalDeviceProperties.apiVersion);
	if (major_version < 1)
	{
		return false;
	}
	if (1 == major_version)
	{
		if (minor_version < 0)
		{
			return false;
		}
	}
	uint32_t queueFamilyCount;
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
	if (0 == queueFamilyCount)
	{
		return false;
	}
	
	std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamilyCount);
	std::vector<VkBool32> queueFamilyPresentSupport(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilyProperties.data());
	for (uint32_t i = 0; i < queueFamilyCount; ++i)
	{
		vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &queueFamilyPresentSupport[i]);
	}

	for (uint32_t i = 0; i < queueFamilyCount; ++i)
	{
		if (queueFamilyProperties[i].queueCount > 0 &&
			queueFamilyProperties[i].queueFlags & (VK_QUEUE_GRAPHICS_BIT) &&
			queueFamilyPresentSupport[i])
		{
			graphicsQueueFamilyIndex = i;
			presentQueueFamilyIndex = i;
			return true;
		}
	}

	graphicsQueueFamilyIndex = UINT32_MAX;
	presentQueueFamilyIndex = UINT32_MAX;

	for (uint32_t i = 0; i < queueFamilyCount; ++i)
	{
		if (queueFamilyProperties[i].queueCount > 0 &&
			queueFamilyProperties[i].queueFlags & (VK_QUEUE_GRAPHICS_BIT))
		{
			graphicsQueueFamilyIndex = i;
			break;
		}
	}
	for (uint32_t i = 0; i < queueFamilyCount; ++i)
	{
		if (queueFamilyPresentSupport[i])
		{
			presentQueueFamilyIndex = i;
			break;
		}
	}

	return graphicsQueueFamilyIndex != UINT32_MAX && presentQueueFamilyIndex != UINT32_MAX;
}


struct VertexData
{
	float x, y, z, w;
	float r, g, b, a;
};

Tutorial02::Tutorial02(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
	startTimer(0);


	VkResult result;
	uint32_t instanceExtensionCount;
	result = vkEnumerateInstanceExtensionProperties(nullptr, &instanceExtensionCount, nullptr);
	if (result != VK_SUCCESS || instanceExtensionCount == 0)
	{
		QMessageBox::critical(nullptr, "error", "enumerate instance extension failed");
		return;
	}
	std::vector<VkExtensionProperties> instanceExtensions(instanceExtensionCount);
	result = vkEnumerateInstanceExtensionProperties(nullptr, &instanceExtensionCount, instanceExtensions.data());
	if (result != VK_SUCCESS || instanceExtensionCount == 0)
	{
		QMessageBox::critical(nullptr, "error", "enumerate instance extension failed");
		return;
	}

	const char* desiredInstanceExtensions[] =
	{
		VK_KHR_SURFACE_EXTENSION_NAME,
#if defined(VK_USE_PLATFORM_WIN32_KHR)
	  VK_KHR_WIN32_SURFACE_EXTENSION_NAME
#elif defined(VK_USE_PLATFORM_XCB_KHR)
	  VK_KHR_XCB_SURFACE_EXTENSION_NAME
#elif defined(VK_USE_PLATFORM_XLIB_KHR)
	  VK_KHR_XLIB_SURFACE_EXTENSION_NAME
#endif
	};

	for (auto desired : desiredInstanceExtensions)
	{
		if (!CheckExtensionAvailability(desired, instanceExtensions))
		{
			QMessageBox::critical(nullptr, "error", QString("could not find extension ") + desired);
			return;
		}
	}


	VkApplicationInfo applicationInfo =
	{
		VK_STRUCTURE_TYPE_APPLICATION_INFO,
		nullptr,
		"Tutorial02",
		VK_MAKE_VERSION(1,0,0),
		"VulkanTutorials",
		VK_MAKE_VERSION(1,0,0),
		VK_API_VERSION_1_0,
	};

	VkInstanceCreateInfo instanceCreateInfo = 
	{
		VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		nullptr,
		0,
		&applicationInfo,
		0,
		nullptr,
		sizeof(desiredInstanceExtensions)/sizeof(desiredInstanceExtensions[0]),
		desiredInstanceExtensions
	};

	VkInstance instance;
	result = vkCreateInstance(&instanceCreateInfo, nullptr, &instance);
	if (result != VK_SUCCESS)
	{
		QMessageBox::critical(nullptr, "error", "create instance failed");
		return;
	}

#if defined(VK_USE_PLATFORM_WIN32_KHR)
	VkWin32SurfaceCreateInfoKHR surfaceCreateInfo =
	{
		VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
		nullptr,
		0,
		GetModuleHandle(NULL),
		(HWND)winId(),
	};
#endif

	VkSurfaceKHR surface;
	result = vkCreateWin32SurfaceKHR(instance, &surfaceCreateInfo, nullptr, &surface);
	if (result != VK_SUCCESS)
	{
		QMessageBox::critical(nullptr, "error", "create surface failed");
		return;
	}

	uint32_t physicalDeviceCount;
	result = vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, nullptr);
	if (result != VK_SUCCESS || 0 == physicalDeviceCount)
	{
		QMessageBox::critical(nullptr, "error", "enumrate physical device failed");
		return;
	}
	std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
	result = vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, physicalDevices.data());
	if (result != VK_SUCCESS || 0 == physicalDeviceCount)
	{
		QMessageBox::critical(nullptr, "error", "enumrate physical device failed");
		return;
	}

	struct CandidatePhysicalDevice
	{
		VkPhysicalDevice physicalDevice;
		uint32_t graphicsQueueFamilyIndex;
		uint32_t presentQueueFamilyIndex;
	};
	std::vector<CandidatePhysicalDevice> candidatePhysicalDevices;
	for (uint32_t i = 0; i < physicalDeviceCount; ++i)
	{
		uint32_t graphicsQueueFamilyIndex, presentQueueFamilyIndex;
		if (CheckPhysicalDevice(graphicsQueueFamilyIndex, presentQueueFamilyIndex, physicalDevices[i],surface))
		{
			CandidatePhysicalDevice candidatePhysicalDevice = { physicalDevices[i], graphicsQueueFamilyIndex, presentQueueFamilyIndex };
			candidatePhysicalDevices.push_back(candidatePhysicalDevice);
		}
	}
	if (candidatePhysicalDevices.empty())
	{
		QMessageBox::critical(nullptr, "error", "find physical device failed");
		return;
	}

	CandidatePhysicalDevice selectedPhysicalDevice = candidatePhysicalDevices[0];

	bool separateGraphicsPresentQueue = selectedPhysicalDevice.graphicsQueueFamilyIndex != selectedPhysicalDevice.presentQueueFamilyIndex;
	float queuePriority = 1.0f;
	VkDeviceQueueCreateInfo deviceQueueCreateInfo[2] =
	{
		{
			VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
			nullptr,
			0,
			selectedPhysicalDevice.graphicsQueueFamilyIndex,
			1,
			&queuePriority
		},
		{
			VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
			nullptr,
			0,
			selectedPhysicalDevice.presentQueueFamilyIndex,
			1,
			&queuePriority
		},
	};
	const char* deviceExtensions[] =
	{
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};


	VkDeviceCreateInfo deviceCreateInfo =
	{
		VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		nullptr,
		0,
		separateGraphicsPresentQueue ? 2 : 1,
		deviceQueueCreateInfo,
		0,
		nullptr,
		sizeof(deviceExtensions)/sizeof(deviceExtensions[0]),
		deviceExtensions,
		nullptr
	};

	VkDevice device;
	result = vkCreateDevice(selectedPhysicalDevice.physicalDevice, &deviceCreateInfo, nullptr, &device);
	if (result != VK_SUCCESS)
	{
		QMessageBox::critical(nullptr, "error", "create device failed");
		return;
	}
	VkQueue graphicsQueue, presentQueue;
	vkGetDeviceQueue(device, selectedPhysicalDevice.graphicsQueueFamilyIndex, 0, &graphicsQueue);
	vkGetDeviceQueue(device, selectedPhysicalDevice.presentQueueFamilyIndex, 0, &presentQueue);


	m_surface = surface;
	m_physicalDevice = selectedPhysicalDevice.physicalDevice;
	m_device = device;
	m_graphicsQueueFamilyIndex = selectedPhysicalDevice.graphicsQueueFamilyIndex;
	m_presentQueueFamilyIndex = selectedPhysicalDevice.presentQueueFamilyIndex;
	m_graphicsQueue = graphicsQueue;
	m_presentQueue = presentQueue;

	init();
}

Tutorial02::~Tutorial02()
{
	clear();
}

bool Tutorial02::init()
{
	if (!createSwapChain())
	{
		return false;
	}
	if (!createRenderingResources())
	{
		return false;
	}
	if (!createRenderPass())
	{
		return false;
	}
	if (!createPipeline())
	{
		return false;
	}
	if (!createVertexBuffer2())
	{
		return false;
	}
	return true;
}

void Tutorial02::clear()
{
	if (m_device != VK_NULL_HANDLE)
	{
		vkDeviceWaitIdle(m_device);
		VkCommandBuffer commandBuffers[rendering_resource_count];
		for (uint32_t i =0; i< rendering_resource_count;++i)
		{
			commandBuffers[i] = m_renderingResources[i].m_commandBuffer;
		}
		vkFreeCommandBuffers(m_device, m_graphicsCommandPool, rendering_resource_count, commandBuffers);
		for (uint32_t i = 0; i < rendering_resource_count; ++i)
		{
			vkDestroyFramebuffer(m_device, m_renderingResources[i].m_framebuffer, nullptr);
			vkDestroySemaphore(m_device, m_renderingResources[i].m_imageAvailableSemaphore, nullptr);
			vkDestroySemaphore(m_device, m_renderingResources[i].m_renderingFinishedSemaphore, nullptr);
			vkDestroyFence(m_device, m_renderingResources[i].m_fence, nullptr);
		}

		if (m_graphicsCommandPool != VK_NULL_HANDLE)
		{
			vkDestroyCommandPool(m_device, m_graphicsCommandPool, nullptr);
			m_graphicsCommandPool = VK_NULL_HANDLE;
		}
		if (m_pipeline != VK_NULL_HANDLE)
		{
			vkDestroyPipeline(m_device, m_pipeline, nullptr);
			m_pipeline = VK_NULL_HANDLE;
		}
		if (m_renderPass != VK_NULL_HANDLE)
		{
			vkDestroyRenderPass(m_device, m_renderPass, nullptr);
			m_renderPass = VK_NULL_HANDLE;
		}
	}
}


bool Tutorial02::createSwapChain()
{
	VkResult result;
	if (m_device != VK_NULL_HANDLE)
	{
		vkDeviceWaitIdle(m_device);
	}

	for (size_t i = 0; i < m_swapChainImages.size(); ++i)
	{
		if (m_swapChainImages[i].m_imageView != VK_NULL_HANDLE)
		{
			vkDestroyImageView(m_device, m_swapChainImages[i].m_imageView, nullptr);
			m_swapChainImages[i].m_imageView = VK_NULL_HANDLE;
		}
	}
	m_swapChainImages.clear();

	VkSurfaceCapabilitiesKHR surfaceCapabilities;
	result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_physicalDevice, m_surface, &surfaceCapabilities);
	if (result != VK_SUCCESS)
	{
		QMessageBox::critical(nullptr, "error", "get surface capabilities failed");
		return false;
	}
	if (0 == (surfaceCapabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT))
	{
		return false;
	}
	uint32_t formatCount;
	result = vkGetPhysicalDeviceSurfaceFormatsKHR(m_physicalDevice, m_surface, &formatCount, nullptr);
	if (result != VK_SUCCESS || formatCount == 0)
	{
		QMessageBox::critical(nullptr, "error", "get surface formats failed");
		return false;
	}
	std::vector<VkSurfaceFormatKHR> formats(formatCount);
	result = vkGetPhysicalDeviceSurfaceFormatsKHR(m_physicalDevice, m_surface, &formatCount, formats.data());
	if (result != VK_SUCCESS)
	{
		QMessageBox::critical(nullptr, "error", "get surface formats failed");
		return false;
	}
	uint32_t presentModeCount;
	result = vkGetPhysicalDeviceSurfacePresentModesKHR(m_physicalDevice, m_surface, &presentModeCount, nullptr);
	if (result != VK_SUCCESS || presentModeCount == 0)
	{
		QMessageBox::critical(nullptr, "error", "get surface present modes failed");
		return false;
	}
	std::vector<VkPresentModeKHR> presentModes(presentModeCount);
	result = vkGetPhysicalDeviceSurfacePresentModesKHR(m_physicalDevice, m_surface, &presentModeCount, presentModes.data());
	if (result != VK_SUCCESS)
	{
		QMessageBox::critical(nullptr, "error", "get surface present modes failed");
		return false;
	}

	uint32_t desiredImageCount = 3;
	desiredImageCount = min(desiredImageCount, surfaceCapabilities.maxImageCount);
	desiredImageCount = max(desiredImageCount, surfaceCapabilities.minImageCount);

	VkSurfaceFormatKHR desiredFormat = formats[0];
	for (VkSurfaceFormatKHR &format : formats) 
	{
		if (format.format == VK_FORMAT_R8G8B8A8_UNORM) 
		{
			desiredFormat = format;
			break;
		}
	}
	if (desiredFormat.format == VK_FORMAT_UNDEFINED)
	{
		desiredFormat.format = VK_FORMAT_R8G8B8A8_UNORM;
		desiredFormat.colorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
	}
	VkExtent2D desiredExtent = surfaceCapabilities.currentExtent;
	desiredExtent.width = min(desiredExtent.width, surfaceCapabilities.maxImageExtent.width);
	desiredExtent.width = max(desiredExtent.width, surfaceCapabilities.minImageExtent.width);
	desiredExtent.height = min(desiredExtent.height, surfaceCapabilities.maxImageExtent.height);
	desiredExtent.height = max(desiredExtent.height, surfaceCapabilities.minImageExtent.height);
	if (0 == desiredExtent.width || 0 == desiredExtent.height)
	{
		return true;
	}

	VkImageUsageFlags desiredUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	
	VkSurfaceTransformFlagBitsKHR desiredTransform = surfaceCapabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR ? VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR : surfaceCapabilities.currentTransform;
	
	VkPresentModeKHR desiredPresentMode = presentModes[0];
	for (VkPresentModeKHR presentMode : presentModes)
	{
		if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			desiredPresentMode = VK_PRESENT_MODE_MAILBOX_KHR;
			break;
		}
		else if (presentMode == VK_PRESENT_MODE_FIFO_KHR)
		{
			desiredPresentMode = VK_PRESENT_MODE_FIFO_KHR;
		}
	}
	VkSwapchainKHR oldSwapChain = m_swapChain;

	VkSwapchainCreateInfoKHR swapChainCreateInfo =
	{
		VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		nullptr,
		0,
		m_surface,
		desiredImageCount,
		desiredFormat.format,
		desiredFormat.colorSpace,
		desiredExtent,
		1,
		desiredUsage,
		VK_SHARING_MODE_EXCLUSIVE,
		0,
		nullptr,
		desiredTransform,
		VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
		desiredPresentMode,
		VK_TRUE,
		oldSwapChain,
	};

	result = vkCreateSwapchainKHR(m_device, &swapChainCreateInfo, nullptr, &m_swapChain);
	if (result != VK_SUCCESS)
	{
		m_swapChain = VK_NULL_HANDLE;
		return false;
	}
	m_swapChainFormat = desiredFormat.format;
	m_swapChainExtent = desiredExtent;

	if (oldSwapChain != VK_NULL_HANDLE)
	{
		vkDestroySwapchainKHR(m_device, oldSwapChain, nullptr);
	}

	uint32_t imageCount = 0;
	result = vkGetSwapchainImagesKHR(m_device, m_swapChain, &imageCount, nullptr);
	if (result != VK_SUCCESS || imageCount == 0)
	{
		return false;
	}
	std::vector<VkImage> images(imageCount);
	result = vkGetSwapchainImagesKHR(m_device, m_swapChain, &imageCount, images.data());
	if (result != VK_SUCCESS)
	{
		return false;
	}
	m_swapChainImages.resize(imageCount);
	for (uint32_t i = 0; i < imageCount; ++i)
	{
		m_swapChainImages[i].m_image = images[i];
	}
	for (uint32_t i = 0; i < imageCount; ++i)
	{
		VkImageViewCreateInfo imageViewCreateInfo =
		{
			VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			nullptr,
			0,
			images[i],
			VK_IMAGE_VIEW_TYPE_2D,
			m_swapChainFormat,
			{
			VK_COMPONENT_SWIZZLE_IDENTITY,
			VK_COMPONENT_SWIZZLE_IDENTITY,
			VK_COMPONENT_SWIZZLE_IDENTITY,
			VK_COMPONENT_SWIZZLE_IDENTITY,
			},
			{
				VK_IMAGE_ASPECT_COLOR_BIT,
				0,
				1,
				0,
				1,
			},
		};
		result = vkCreateImageView(m_device, &imageViewCreateInfo, nullptr, &m_swapChainImages[i].m_imageView);
		if (result != VK_SUCCESS)
		{
			return false;
		}
	}
	return true;
}

bool Tutorial02::createRenderPass()
{
	VkResult result;
	VkAttachmentDescription attachmentDescription =
	{
		0,
		m_swapChainFormat,
		VK_SAMPLE_COUNT_1_BIT,
		VK_ATTACHMENT_LOAD_OP_CLEAR,
		VK_ATTACHMENT_STORE_OP_STORE,
		VK_ATTACHMENT_LOAD_OP_DONT_CARE,
		VK_ATTACHMENT_STORE_OP_DONT_CARE,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
	};

	VkAttachmentReference attachmentReference =
	{
		0,
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
	};

	VkSubpassDescription subpassDescription =
	{
		0,
		VK_PIPELINE_BIND_POINT_GRAPHICS,
		0,
		nullptr,
		1,
		&attachmentReference,
		nullptr,
		nullptr,
		0,
		nullptr,
	};

	VkSubpassDependency subpassDependencies[]=
	{
		{
			VK_SUBPASS_EXTERNAL,
			0,
			VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			VK_ACCESS_MEMORY_READ_BIT,
			VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
			VK_DEPENDENCY_BY_REGION_BIT,
		},
		{
			0,
			VK_SUBPASS_EXTERNAL,
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
			VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
			VK_ACCESS_MEMORY_READ_BIT,
			VK_DEPENDENCY_BY_REGION_BIT,
		},
	};

	VkRenderPassCreateInfo renderPassCreateInfo =
	{
		VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
		nullptr,
		0,
		1,
		&attachmentDescription,
		1,
		&subpassDescription,
		sizeof(subpassDependencies)/sizeof(subpassDependencies[0]),
		subpassDependencies,
	};

	VkRenderPass renderPass;
	result = vkCreateRenderPass(m_device, &renderPassCreateInfo, nullptr, &renderPass);
	if (result != VK_SUCCESS)
	{
		return false;
	}
	m_renderPass = renderPass;
	return true;
}

bool Tutorial02::createRenderingResources()
{
	VkResult result;

	VkCommandPoolCreateInfo commandPoolCreateInfo =
	{
		VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		nullptr,
		0,
		m_graphicsQueueFamilyIndex
	};

	result = vkCreateCommandPool(m_device, &commandPoolCreateInfo, nullptr, &m_graphicsCommandPool);
	if (result != VK_SUCCESS)
	{
		QMessageBox::critical(nullptr, "error", "create command pool failed");
		return false;
	}

	VkCommandBuffer commandBuffers[rendering_resource_count];
	VkCommandBufferAllocateInfo commandBufferAllocateInfo =
	{
		VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		nullptr,
		m_graphicsCommandPool,
		VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		rendering_resource_count,
	};
	result = vkAllocateCommandBuffers(m_device, &commandBufferAllocateInfo, commandBuffers);
	if (result != VK_SUCCESS)
	{
		QMessageBox::critical(nullptr, "error", "allocate command buffers failed");
		return false;
	}

	for (uint32_t i = 0; i < rendering_resource_count; ++i)
	{
		m_renderingResources[i].m_commandBuffer = commandBuffers[i];
	}

	VkSemaphoreCreateInfo semaphoreCreateInfo =
	{
		VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
		nullptr,
		0
	};
	for (uint32_t i = 0; i < rendering_resource_count; ++i)
	{
		vkCreateSemaphore(m_device, &semaphoreCreateInfo, nullptr, &m_renderingResources[i].m_imageAvailableSemaphore);
		vkCreateSemaphore(m_device, &semaphoreCreateInfo, nullptr, &m_renderingResources[i].m_renderingFinishedSemaphore);
	}

	VkFenceCreateInfo fenceCreateInfo =
	{
		VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
		nullptr, 
		VK_FENCE_CREATE_SIGNALED_BIT
	};
	for (uint32_t i = 0; i < rendering_resource_count; ++i)
	{
		vkCreateFence(m_device, &fenceCreateInfo, nullptr, &m_renderingResources[i].m_fence);
	}
	return true;
}


VkShaderModule Tutorial02::createShaderModule(const char* fileName)
{
	VkResult result;
	std::vector<char> shaderCode = GetBinaryFileContents(fileName);
	if (shaderCode.empty())
	{
		return VK_NULL_HANDLE;
	}

	VkShaderModuleCreateInfo shaderModuleCreateInfo =
	{
		VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		nullptr,
		0,
		shaderCode.size(),
		(const uint32_t*)shaderCode.data(),
	};

	VkShaderModule shaderModule;
	result = vkCreateShaderModule(m_device, &shaderModuleCreateInfo, nullptr, &shaderModule);
	if (result != VK_SUCCESS)
	{
		return VK_NULL_HANDLE;
	}
	return shaderModule;
}

bool Tutorial02::createVertexBuffer()
{
	VkResult result;
	VertexData vertexData[] = 
	{
	  {
		-0.7f, -0.7f, 0.0f, 1.0f,
		1.0f, 0.0f, 0.0f, 0.0f
	  },
	  {
		-0.7f, 0.7f, 0.0f, 1.0f,
		0.0f, 1.0f, 0.0f, 0.0f
	  },
	  {
		0.7f, -0.7f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f, 0.0f
	  },
	  {
		0.7f, 0.7f, 0.0f, 1.0f,
		0.3f, 0.3f, 0.3f, 0.0f
	  }
	};
	m_vertexBuffer.m_size = sizeof(vertexData);
	
	VkBufferCreateInfo bufferCreateInfo =
	{
		VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		nullptr,
		0,
		m_vertexBuffer.m_size,
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		VK_SHARING_MODE_EXCLUSIVE,
		0,
		nullptr,
	};

	result = vkCreateBuffer(m_device, &bufferCreateInfo, nullptr, &m_vertexBuffer.m_buffer);
	if (result != VK_SUCCESS)
	{
		return false;
	}

	VkMemoryRequirements memoryRequirements;
	vkGetBufferMemoryRequirements(m_device, m_vertexBuffer.m_buffer, &memoryRequirements);

	VkPhysicalDeviceMemoryProperties memoryProperties;
	vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &memoryProperties);
	for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; ++i)
	{
		if (memoryRequirements.memoryTypeBits & 1 << i 
			&&
			memoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
		{
			VkMemoryAllocateInfo allocateInfo =
			{
				VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
				nullptr,
				m_vertexBuffer.m_size,
				i
			};
			result = vkAllocateMemory(m_device, &allocateInfo, nullptr, &m_vertexBuffer.m_deviceMemory);
			if (result == VK_SUCCESS)
			{
				break;
			}
		}
	}
	if (m_vertexBuffer.m_deviceMemory == VK_NULL_HANDLE)
	{
		return false;
	}
	result = vkBindBufferMemory(m_device, m_vertexBuffer.m_buffer, m_vertexBuffer.m_deviceMemory, 0);
	if (result != VK_SUCCESS)
	{
		return false;
	}
	void* mappedPtr;
	result = vkMapMemory(m_device, m_vertexBuffer.m_deviceMemory, 0, m_vertexBuffer.m_size, 0, &mappedPtr);
	if (result != VK_SUCCESS)
	{
		return false;
	}
	memcpy(mappedPtr, vertexData, m_vertexBuffer.m_size);
	VkMappedMemoryRange mappedMemoryRange =
	{
		VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
		nullptr,
		m_vertexBuffer.m_deviceMemory,
		0,
		m_vertexBuffer.m_size,
	};
	vkFlushMappedMemoryRanges(m_device, 1, &mappedMemoryRange);
	vkUnmapMemory(m_device, m_vertexBuffer.m_deviceMemory);
	return true;
}

bool Tutorial02::createVertexBuffer2()
{
	VkResult result;
	VertexData vertexData[] =
	{
	  {
		-0.7f, -0.7f, 0.0f, 1.0f,
		1.0f, 0.0f, 0.0f, 0.0f
	  },
	  {
		-0.7f, 0.7f, 0.0f, 1.0f,
		0.0f, 1.0f, 0.0f, 0.0f
	  },
	  {
		0.7f, -0.7f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f, 0.0f
	  },
	  {
		0.7f, 0.7f, 0.0f, 1.0f,
		0.3f, 0.3f, 0.3f, 0.0f
	  }
	};

	VkMemoryRequirements memoryRequirements;
	VkPhysicalDeviceMemoryProperties memoryProperties;
	vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &memoryProperties);

	m_vertexBuffer.m_size = sizeof(vertexData);
	
	VkBufferCreateInfo deviceBufferCreateInfo =
	{
		VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		nullptr,
		0,
		m_vertexBuffer.m_size,
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		VK_SHARING_MODE_EXCLUSIVE,
		0,
		nullptr,
	};
	result = vkCreateBuffer(m_device, &deviceBufferCreateInfo, nullptr, &m_vertexBuffer.m_buffer);
	if (result != VK_SUCCESS)
	{
		return false;
	}
	vkGetBufferMemoryRequirements(m_device, m_vertexBuffer.m_buffer, &memoryRequirements);

	for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; ++i)
	{
		if (memoryRequirements.memoryTypeBits & 1 << i
			&&
			memoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
		{
			VkMemoryAllocateInfo allocateInfo =
			{
				VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
				nullptr,
				m_vertexBuffer.m_size,
				i
			};
			result = vkAllocateMemory(m_device, &allocateInfo, nullptr, &m_vertexBuffer.m_deviceMemory);
			if (result == VK_SUCCESS)
			{
				break;
			}
		}
	}
	if (m_vertexBuffer.m_deviceMemory == VK_NULL_HANDLE)
	{
		return false;
	}
	result = vkBindBufferMemory(m_device, m_vertexBuffer.m_buffer, m_vertexBuffer.m_deviceMemory, 0);
	if (result != VK_SUCCESS)
	{
		return false;
	}

	VkBufferCreateInfo stagingBufferCreateInfo =
	{
		VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		nullptr,
		0,
		m_vertexBuffer.m_size,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_SHARING_MODE_EXCLUSIVE,
		0,
		nullptr,
	};
	
	VkBuffer stagingBuffer;
	result = vkCreateBuffer(m_device, &deviceBufferCreateInfo, nullptr, &stagingBuffer);
	if (result != VK_SUCCESS)
	{
		return false;
	}

	vkGetBufferMemoryRequirements(m_device, stagingBuffer, &memoryRequirements);

	VkDeviceMemory stagingMemory = VK_NULL_HANDLE;
	for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; ++i)
	{
		if (memoryRequirements.memoryTypeBits & 1 << i
			&&
			memoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
		{
			VkMemoryAllocateInfo allocateInfo =
			{
				VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
				nullptr,
				m_vertexBuffer.m_size,
				i
			};
			result = vkAllocateMemory(m_device, &allocateInfo, nullptr, &stagingMemory);
			if (result == VK_SUCCESS)
			{
				break;
			}
		}
	}
	if (stagingMemory == VK_NULL_HANDLE)
	{
		return false;
	}
	result = vkBindBufferMemory(m_device, stagingBuffer, stagingMemory, 0);
	if (result != VK_SUCCESS)
	{
		return false;
	}

	void* mappedPtr;
	result = vkMapMemory(m_device, stagingMemory, 0, m_vertexBuffer.m_size, 0, &mappedPtr);
	if (result != VK_SUCCESS)
	{
		return false;
	}
	memcpy(mappedPtr, vertexData, m_vertexBuffer.m_size);
	VkMappedMemoryRange mappedMemoryRange =
	{
		VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
		nullptr,
		stagingMemory,
		0,
		m_vertexBuffer.m_size,
	};
	vkFlushMappedMemoryRanges(m_device, 1, &mappedMemoryRange);
	vkUnmapMemory(m_device, stagingMemory);

	VkCommandBufferBeginInfo commandBufferBeginInfo =
	{
		VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		nullptr,
		VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
		nullptr,
	};

	VkCommandBuffer commandBuffer = m_renderingResources[0].m_commandBuffer;

	vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo);
	
	VkBufferCopy bufferCopy = 
	{
		0,
		0,
		m_vertexBuffer.m_size,
	};
	vkCmdCopyBuffer(commandBuffer, stagingBuffer, m_vertexBuffer.m_buffer, 1, &bufferCopy);

	VkBufferMemoryBarrier bufferMemoryBarrier =
	{
		VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
		nullptr,
		VK_ACCESS_MEMORY_WRITE_BIT,
		VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT,
		VK_QUEUE_FAMILY_IGNORED,
		VK_QUEUE_FAMILY_IGNORED,
		m_vertexBuffer.m_buffer,
		0,
		VK_WHOLE_SIZE,
	};
	vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, 0, 0, nullptr, 1, &bufferMemoryBarrier, 0, nullptr);
	vkEndCommandBuffer(commandBuffer);
	VkSubmitInfo submitInfo =
	{
		VK_STRUCTURE_TYPE_SUBMIT_INFO,
		nullptr,
		0,
		nullptr,
		0,
		1,
		&commandBuffer,
		0,
		nullptr,
	};
	result = vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
	if (result != VK_SUCCESS)
	{
		return false;
	}
	vkDeviceWaitIdle(m_device);
	return true;
}

bool Tutorial02::createPipeline()
{
	std::string path(QCoreApplication::applicationDirPath().toStdString());

	VkResult result;
	VkShaderModule vertexShaderModule = createShaderModule((path + "/shader02.vert.spv").c_str());
	VkShaderModule fragmentShaderModule = createShaderModule((path + "/shader02.frag.spv").c_str());
	if (VK_NULL_HANDLE == vertexShaderModule || VK_NULL_HANDLE == fragmentShaderModule)
	{
		return false;
	}

	VkPipelineShaderStageCreateInfo shaderStageCreateInfos[]=
	{
		{
			VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			nullptr,
			0,
			VK_SHADER_STAGE_VERTEX_BIT,
			vertexShaderModule,
			"main",
			nullptr,
		},
		{
			VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			nullptr,
			0,
			VK_SHADER_STAGE_FRAGMENT_BIT,
			fragmentShaderModule,
			"main",
			nullptr,
		},
	};

	VkVertexInputBindingDescription vertexInputBindingDescriptions[] =
	{
		{
			0,
			sizeof(VertexData),
			VK_VERTEX_INPUT_RATE_VERTEX,
		},
	};

	VkVertexInputAttributeDescription vertexInputAttributeDescriptions[]=
	{
		{
			0,
			0,
			VK_FORMAT_R32G32B32A32_SFLOAT,
			0,
		},
		{
			1,
			0,
			VK_FORMAT_R32G32B32A32_SFLOAT,
			sizeof(float) * 4,
		},
	};

	VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo =
	{
		VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		nullptr,
		0,
		sizeof(vertexInputBindingDescriptions)/sizeof(vertexInputBindingDescriptions[0]),
		vertexInputBindingDescriptions,
		sizeof(vertexInputAttributeDescriptions)/sizeof(vertexInputAttributeDescriptions[0]),
		vertexInputAttributeDescriptions,
	};

	VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo =
	{
		VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
		nullptr,
		0,
		VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,
		VK_FALSE,
	};


	VkPipelineViewportStateCreateInfo viewportStateCreateInfo =
	{
		VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
		nullptr,
		0,
		1,
		nullptr,
		1,
		nullptr,
	};

	VkPipelineRasterizationStateCreateInfo rasterizationStateCreateInfo =
	{
		VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
		nullptr,
		0,
		VK_FALSE,
		VK_FALSE,
		VK_POLYGON_MODE_FILL,
		VK_CULL_MODE_BACK_BIT,
		VK_FRONT_FACE_COUNTER_CLOCKWISE,
		VK_FALSE,
		0,
		0,
		0,
		1,
	};
	
	VkPipelineMultisampleStateCreateInfo multisampleStateCreateInfo =
	{
		VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
		nullptr,
		0,
		VK_SAMPLE_COUNT_1_BIT,
		VK_FALSE,
		1,
		nullptr,
		VK_FALSE,
		VK_FALSE,
	};

	VkPipelineColorBlendAttachmentState colorBlendAttachmentState =
	{
		VK_FALSE,
		VK_BLEND_FACTOR_ONE,
		VK_BLEND_FACTOR_ZERO,
		VK_BLEND_OP_ADD,
		VK_BLEND_FACTOR_ONE,
		VK_BLEND_FACTOR_ZERO,
		VK_BLEND_OP_ADD,
		VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT| VK_COLOR_COMPONENT_B_BIT| VK_COLOR_COMPONENT_A_BIT,
	};

	VkPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo =
	{
		VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
		nullptr,
		0,
		VK_FALSE,
		VK_LOGIC_OP_COPY,
		1,
		&colorBlendAttachmentState,
		{0,0,0,0},
	};

	VkDynamicState dynamicStates[] =
	{
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR,
	};

	VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo =
	{
		VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
		nullptr,
		0,
		sizeof(dynamicStates) / sizeof(dynamicStates[0]),
		dynamicStates,
	};

	VkPipelineLayoutCreateInfo layoutCreateInfo =
	{
		VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		nullptr,
		0,
		0,
		nullptr,
		0,
		nullptr,
	};

	VkPipelineLayout pipelineLayout;
	result = vkCreatePipelineLayout(m_device, &layoutCreateInfo, nullptr, &pipelineLayout);
	if (result != VK_SUCCESS)
	{
		return false;
	}

	VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo =
	{
		VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
		nullptr,
		0,
		sizeof(shaderStageCreateInfos)/sizeof(shaderStageCreateInfos[0]),
		shaderStageCreateInfos,
		&vertexInputStateCreateInfo,
		&inputAssemblyStateCreateInfo,
		nullptr,
		&viewportStateCreateInfo,
		&rasterizationStateCreateInfo,
		&multisampleStateCreateInfo,
		nullptr,
		&colorBlendStateCreateInfo,
		&dynamicStateCreateInfo,
		pipelineLayout,
		m_renderPass,
		0,
		VK_NULL_HANDLE,
		-1,
	};

	result = vkCreateGraphicsPipelines(m_device, VK_NULL_HANDLE, 1, &graphicsPipelineCreateInfo, nullptr, &m_pipeline);
	if (result != VK_SUCCESS)
	{
		return false;
	}
	return true;
}

bool Tutorial02::draw()
{
	VkResult result;
	static uint32_t s_resourceIndex = 0;
	RenderingResource& renderingResource = m_renderingResources[s_resourceIndex];
	uint32_t imageIndex;
	s_resourceIndex = (s_resourceIndex + 1) % rendering_resource_count;

	result = vkWaitForFences(m_device, 1, &renderingResource.m_fence, VK_FALSE, 1000000000ULL);
	if (result != VK_SUCCESS)
	{
		return false;
	}
	vkResetFences(m_device, 1, &renderingResource.m_fence);

	result = vkAcquireNextImageKHR(m_device, m_swapChain, UINT64_MAX, renderingResource.m_imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);
	switch (result)
	{
	case VK_SUCCESS:
	case VK_SUBOPTIMAL_KHR:
		break;
	case VK_ERROR_OUT_OF_DATE_KHR:
		return onSizeWindow();
	default:
		return false;
	}

	SwapchainImage& swapchainImage = m_swapChainImages[imageIndex];

	if (renderingResource.m_framebuffer != VK_NULL_HANDLE)
	{
		vkDestroyFramebuffer(m_device, renderingResource.m_framebuffer, nullptr);
	}

	VkFramebufferCreateInfo framebufferCreateInfo =
	{
		VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
		nullptr,
		0,
		m_renderPass,
		1,
		&swapchainImage.m_imageView,
		m_swapChainExtent.width,
		m_swapChainExtent.height,
		1,
	};

	result = vkCreateFramebuffer(m_device, &framebufferCreateInfo, nullptr, &renderingResource.m_framebuffer);
	if (result != VK_SUCCESS)
	{
		return false;
	}

	VkCommandBufferBeginInfo commandBufferBeginInfo =
	{
		VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		nullptr,
		VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
		nullptr,
	};
	vkBeginCommandBuffer(renderingResource.m_commandBuffer, &commandBufferBeginInfo);

	VkImageSubresourceRange imageSubresourceRange =
	{
		VK_IMAGE_ASPECT_COLOR_BIT,
		0,
		1,
		0,
		1,
	};

	if (m_graphicsQueue != m_presentQueue)
	{
		VkImageMemoryBarrier presentToDrawBarrier =
		{
			VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
			nullptr,
			VK_ACCESS_MEMORY_READ_BIT,
			VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
			m_presentQueueFamilyIndex,
			m_graphicsQueueFamilyIndex,
			swapchainImage.m_image,
			imageSubresourceRange
		};
		vkCmdPipelineBarrier(renderingResource.m_commandBuffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0, nullptr, 0, nullptr, 1, &presentToDrawBarrier);
	}


	VkClearValue clearValue =
	{
		{0,0,0,0},
	};
	VkRenderPassBeginInfo renderPassBeginInfo =
	{
		VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
		nullptr,
		m_renderPass,
		renderingResource.m_framebuffer,
		{
			{
				0,
				0,
			},
			{
				m_swapChainExtent.width,
				m_swapChainExtent.height,
			},
		},
		1,
		&clearValue,
	};

	vkCmdBeginRenderPass(renderingResource.m_commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
	vkCmdBindPipeline(renderingResource.m_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);

	VkViewport viewport =
	{
		0,
		0,
		m_swapChainExtent.width,
		m_swapChainExtent.height,
		0,
		1,
	};
	
	VkRect2D scissor =
	{
		{
			0,
			0
		},
		{
			m_swapChainExtent.width,
			m_swapChainExtent.height,
		},
	};

	vkCmdSetViewport(renderingResource.m_commandBuffer, 0, 1, &viewport);
	vkCmdSetScissor(renderingResource.m_commandBuffer, 0, 1, &scissor);
	VkDeviceSize offset = 0;
	vkCmdBindVertexBuffers(renderingResource.m_commandBuffer, 0, 1, &m_vertexBuffer.m_buffer, &offset);
	vkCmdDraw(renderingResource.m_commandBuffer, 4, 1, 0, 0);
	vkCmdEndRenderPass(renderingResource.m_commandBuffer);

	if (m_graphicsQueue != m_presentQueue)
	{
		VkImageMemoryBarrier drawToPresentBarrier =
		{
			VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
			nullptr,
			VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
			VK_ACCESS_MEMORY_READ_BIT,
			VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
			VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
			m_graphicsQueueFamilyIndex,
			m_presentQueueFamilyIndex,
			swapchainImage.m_image,
			imageSubresourceRange
		};
		vkCmdPipelineBarrier(renderingResource.m_commandBuffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1, &drawToPresentBarrier);
	}

	result = vkEndCommandBuffer(renderingResource.m_commandBuffer);
	if (result != VK_SUCCESS)
	{
		return false;
	}

	VkPipelineStageFlags waitDstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	VkSubmitInfo submitInfo =
	{
		VK_STRUCTURE_TYPE_SUBMIT_INFO,
		nullptr,
		1,
		&renderingResource.m_imageAvailableSemaphore,
		&waitDstStageMask,
		1,
		&renderingResource.m_commandBuffer,
		1,
		&renderingResource.m_renderingFinishedSemaphore,
	};
	result = vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, renderingResource.m_fence);
	if (result != VK_SUCCESS)
	{
		return false;
	}

	VkPresentInfoKHR presentInfo =
	{
		VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
		nullptr,
		1,
		&renderingResource.m_renderingFinishedSemaphore,
		1,
		&m_swapChain,
		&imageIndex,
		nullptr,
	};

	result = vkQueuePresentKHR(m_presentQueue, &presentInfo);
	switch (result) 
	{
	case VK_SUCCESS:
		break;
	case VK_ERROR_OUT_OF_DATE_KHR:
	case VK_SUBOPTIMAL_KHR:
		return onSizeWindow();
	default:
		return false;
	}
	return true;
}


bool Tutorial02::onSizeWindow()
{
	if (!createSwapChain())
	{
		return false;
	}
	return true;
}

void Tutorial02::resizeEvent(QResizeEvent* e)
{
	onSizeWindow();
}

void Tutorial02::timerEvent(QTimerEvent *event)
{
	draw();
}
