#include "Tutorial01.h"

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

Tutorial01::Tutorial01(QWidget *parent)
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
		"Tutorial01",
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

	VkSemaphoreCreateInfo semaphoreCreateInfo =
	{
		VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
		nullptr,
		0
	};
	VkSemaphore imageAvailableSemaphore, renderingFinishedSemaphore;
	vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &imageAvailableSemaphore);
	vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &renderingFinishedSemaphore);

	m_surface = surface;
	m_physicalDevice = selectedPhysicalDevice.physicalDevice;
	m_device = device;
	m_graphicsQueueFamilyIndex = selectedPhysicalDevice.graphicsQueueFamilyIndex;
	m_presentQueueFamilyIndex = selectedPhysicalDevice.presentQueueFamilyIndex;
	m_graphicsQueue = graphicsQueue;
	m_presentQueue = presentQueue;
	m_imageAvailableSemaphore = imageAvailableSemaphore;
	m_renderingFinishedSemaphore = renderingFinishedSemaphore;

}

Tutorial01::~Tutorial01()
{}

bool Tutorial01::createSwapChain()
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

bool Tutorial01::createRenderPass()
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

	VkRenderPassCreateInfo renderPassCreateInfo =
	{
		VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
		nullptr,
		0,
		1,
		&attachmentDescription,
		1,
		&subpassDescription,
		0,
		nullptr,
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

bool Tutorial01::createFrameBuffers()
{
	for (size_t i=0;i<m_frameBuffers.size();++i)
	{
		VkFramebuffer frameBuffer = m_frameBuffers[i];
		if (frameBuffer != VK_NULL_HANDLE)
		{
			vkDestroyFramebuffer(m_device, frameBuffer, nullptr);
		}
		m_frameBuffers[i] = VK_NULL_HANDLE;
	}
		
	VkResult result;
	uint32_t imageCount = m_swapChainImages.size();
	m_frameBuffers.resize(imageCount, VK_NULL_HANDLE);
	for (uint32_t i = 0; i < imageCount; ++i)
	{
		VkFramebufferCreateInfo frameBufferCreateInfo =
		{
			VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
			nullptr,
			0,
			m_renderPass,
			1,
			&m_swapChainImages[i].m_imageView,
			m_swapChainExtent.width,
			m_swapChainExtent.height,
			1,
		};
		result = vkCreateFramebuffer(m_device, &frameBufferCreateInfo, nullptr, &m_frameBuffers[i]);
		if (result != VK_SUCCESS)
		{
			return false;
		}
	}
	return true;
}

VkShaderModule Tutorial01::createShaderModule(const char* fileName)
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

bool Tutorial01::createPipeline()
{
	std::string path(QCoreApplication::applicationDirPath().toStdString());

	VkResult result;
	VkShaderModule vertexShaderModule = createShaderModule((path + "/shader.vert.spv").c_str());
	VkShaderModule fragmentShaderModule = createShaderModule((path + "/shader.frag.spv").c_str());
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

	VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo =
	{
		VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		nullptr,
		0,
		0,
		nullptr,
		0,
		nullptr,
	};

	VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo =
	{
		VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
		nullptr,
		0,
		VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
		VK_FALSE,
	};

	VkViewport viewport = 
	{
		0,
		0,
		m_swapChainExtent.width,
		m_swapChainExtent.height,
		0,
		1
	};

	VkRect2D scissor =
	{
		{
			0,0
		},
		{
			m_swapChainExtent.width, m_swapChainExtent.height,
		}
	};

	VkPipelineViewportStateCreateInfo viewportStateCreateInfo =
	{
		VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
		nullptr,
		0,
		1,
		&viewport,
		1,
		&scissor,
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
		nullptr,
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

bool Tutorial01::createCommandBuffers()
{
	VkResult result;
	//uint32_t imageCount = 0;
	//result = vkGetSwapchainImagesKHR(m_device, m_swapChain, &imageCount, nullptr);
	//if (result != VK_SUCCESS || imageCount == 0)
	//{
	//	QMessageBox::critical(nullptr, "error", "get swapchain images failed");
	//	return false;
	//}

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

	uint32_t imageCount = m_swapChainImages.size();
	m_graphicsCommandBuffers.resize(imageCount, VK_NULL_HANDLE);
	VkCommandBufferAllocateInfo commandBufferAllocateInfo =
	{
		VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		nullptr,
		m_graphicsCommandPool,
		VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		imageCount,
	};
	result = vkAllocateCommandBuffers(m_device, &commandBufferAllocateInfo, m_graphicsCommandBuffers.data());
	if (result != VK_SUCCESS)
	{
		QMessageBox::critical(nullptr, "error", "allocate command buffers failed");
		return false;
	}
	return true;
}

bool Tutorial01::recordCommands()
{
	VkResult result;
	//uint32_t imageCount = m_presentQueueCommandBuffers.size();
	//std::vector<VkImage> images(imageCount);
	//result = vkGetSwapchainImagesKHR(m_device, m_swapChain, &imageCount, images.data());
	//if (result != VK_SUCCESS)
	//{
	//	QMessageBox::critical(nullptr, "error", "get swapchain images failed");
	//	return false;
	//}

	VkCommandBufferBeginInfo commandBufferBeginInfo =
	{
		VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		nullptr,
		VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT,
		nullptr,
	};

	VkClearValue clearValue =
	{
		{0,0,0,0},
	};

	VkImageSubresourceRange imageSubresourceRange =
	{
		VK_IMAGE_ASPECT_COLOR_BIT,
		0,
		1,
		0,
		1,
	};

	uint32_t imageCount = m_swapChainImages.size();

	for (uint32_t i = 0; i < imageCount; ++i)
	{
		VkImage image = m_swapChainImages[i].m_image;
		VkCommandBuffer commandBuffer = m_graphicsCommandBuffers[i];
		vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo);

		if(m_graphicsQueue != m_presentQueue)
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
				image,
				imageSubresourceRange
			};
			vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0, nullptr, 0, nullptr, 1, &presentToDrawBarrier);

		}

		VkRenderPassBeginInfo renderPassBeginInfo =
		{
			VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
			nullptr,
			m_renderPass,
			m_frameBuffers[i],
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
		vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);
		vkCmdDraw(commandBuffer, 3, 1, 0, 0);
		vkCmdEndRenderPass(commandBuffer);

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
				image,
				imageSubresourceRange
			};
			vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1, &drawToPresentBarrier);
		}
		result = vkEndCommandBuffer(commandBuffer);
		if (result != VK_SUCCESS)
		{
			return false;
		}
	}
	return true;
}

bool Tutorial01::draw()
{
	VkResult result;
	uint32_t imageIndex;
	result = vkAcquireNextImageKHR(m_device, m_swapChain, UINT64_MAX, m_imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);
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

	VkPipelineStageFlags waitDstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	VkSubmitInfo submitInfo =
	{
		VK_STRUCTURE_TYPE_SUBMIT_INFO,
		nullptr,
		1,
		&m_imageAvailableSemaphore,
		&waitDstStageMask,
		1,
		&m_graphicsCommandBuffers[imageIndex],
		1,
		&m_renderingFinishedSemaphore,
	};
	result = vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
	if (result != VK_SUCCESS)
	{
		return false;
	}

	VkPresentInfoKHR presentInfo =
	{
		VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
		nullptr,
		1,
		&m_renderingFinishedSemaphore,
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

void Tutorial01::clear()
{
	if (m_device != VK_NULL_HANDLE)
	{
		vkDeviceWaitIdle(m_device);
		if (!m_graphicsCommandBuffers.empty())
		{
			vkFreeCommandBuffers(m_device, m_graphicsCommandPool, m_graphicsCommandBuffers.size(), m_graphicsCommandBuffers.data());
			m_graphicsCommandBuffers.clear();
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
		for(auto frameBuffer: m_frameBuffers)
		{
			vkDestroyFramebuffer(m_device, frameBuffer, nullptr);
		}
		m_frameBuffers.clear();
	}
}


bool Tutorial01::onSizeWindow()
{
	clear();

	if (!createSwapChain())
	{
		return false;
	}
	if (!createRenderPass())
	{
		return false;
	}
	if (!createFrameBuffers())
	{
		return false;
	}
	if (!createPipeline())
	{
		return false;
	}
	if (!createCommandBuffers())
	{
		return false;
	}
	if (!recordCommands())
	{
		return false;
	}
	return true;
}

void Tutorial01::resizeEvent(QResizeEvent* e)
{
	onSizeWindow();
}

void Tutorial01::timerEvent(QTimerEvent *event)
{
	draw();
}
