#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_Tutorial03.h"
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>
#include <memory>

struct SwapchainImage
{
	VkImage m_image{ VK_NULL_HANDLE };
	VkImageView m_imageView{ VK_NULL_HANDLE };
};

struct VertexBuffer
{
	VkBuffer m_buffer{ VK_NULL_HANDLE };
	VkDeviceMemory m_deviceMemory{ VK_NULL_HANDLE };
	uint32_t m_size{ 0 };
};

struct UniformBuffer
{
	VkBuffer m_buffer{ VK_NULL_HANDLE };
	VkDeviceMemory m_deviceMemory{ VK_NULL_HANDLE };
	uint32_t m_size{ 0 };
};

struct RenderingResource
{
	VkFramebuffer m_framebuffer{ VK_NULL_HANDLE };
	VkCommandBuffer m_commandBuffer{ VK_NULL_HANDLE };
	VkSemaphore m_imageAvailableSemaphore{ VK_NULL_HANDLE };
	VkSemaphore m_renderingFinishedSemaphore{ VK_NULL_HANDLE };
	VkFence m_fence{ VK_NULL_HANDLE };
};

struct StagingBuffer
{
	VkBuffer m_buffer{ VK_NULL_HANDLE };
	VkDeviceMemory m_deviceMemory{ VK_NULL_HANDLE };
	uint32_t m_size{ 0 };
};

struct Texture
{
	VkImage m_image;
	VkImageView m_imageView;
	VkSampler m_sampler;
	VkDeviceMemory m_deviceMemory;
};

struct DescriptorSet
{
	VkDescriptorPool m_descriptorPool;
	VkDescriptorSetLayout m_descriptorSetLayout;
	VkDescriptorSet m_descriptorSet;
};

class Tutorial03 : public QMainWindow
{
    Q_OBJECT

public:
    Tutorial03(QWidget *parent = nullptr);
    ~Tutorial03();
private:
	virtual void resizeEvent(QResizeEvent *) override;
	virtual void timerEvent(QTimerEvent *event) override;
private:
	bool init();
	bool createSwapChain();
	bool createRenderingResources();
	bool createRenderPass();
	bool createStagingBuffer();
	bool createTexture();
	bool createVertexBuffer();
	bool createUniformBuffer();
	bool createDescriptorSet();
	bool createPipeline();
	void clear();
	bool draw();
	bool onSizeWindow();
	VkShaderModule createShaderModule(const char* fileName);
private:
	VkSurfaceKHR m_surface{ VK_NULL_HANDLE };
	VkSwapchainKHR m_swapChain{ VK_NULL_HANDLE };
	VkFormat m_swapChainFormat{ VK_FORMAT_UNDEFINED };
	VkExtent2D m_swapChainExtent{0,0};
	VkPhysicalDevice m_physicalDevice{ VK_NULL_HANDLE };
	VkDevice m_device{ VK_NULL_HANDLE };
	uint32_t m_graphicsQueueFamilyIndex;
	uint32_t m_presentQueueFamilyIndex;
	VkQueue m_graphicsQueue{ VK_NULL_HANDLE };
	VkQueue m_presentQueue{ VK_NULL_HANDLE };
	VkCommandPool m_graphicsCommandPool{ VK_NULL_HANDLE };
	std::vector<SwapchainImage> m_swapChainImages;
	static const uint32_t rendering_resource_count = 3;
	RenderingResource  m_renderingResources[rendering_resource_count];
	VkPipelineLayout m_pipelineLayout{ VK_NULL_HANDLE };
	StagingBuffer m_stagingBuffer;
	VertexBuffer m_vertexBuffer;
	UniformBuffer m_uniformBuffer;
	Texture m_texture;
	DescriptorSet m_descriptorSet;
	VkRenderPass m_renderPass{ VK_NULL_HANDLE };
	VkPipeline m_pipeline{ VK_NULL_HANDLE };
private:
    Ui::Tutorial03Class ui;
};
