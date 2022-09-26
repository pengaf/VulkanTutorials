#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_Tutorial01.h"
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>
#include <memory>

struct SwapchainImage
{
	VkImage m_image{ VK_NULL_HANDLE };
	VkImageView m_imageView{ VK_NULL_HANDLE };
};

class Tutorial01 : public QMainWindow
{
    Q_OBJECT

public:
    Tutorial01(QWidget *parent = nullptr);
    ~Tutorial01();
private:
	virtual void resizeEvent(QResizeEvent *) override;
	virtual void timerEvent(QTimerEvent *event) override;
private:
	bool createSwapChain();
	bool createCommandBuffers();
	bool createRenderPass();
	bool createFrameBuffers();
	bool createPipeline();
	bool recordCommands();
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
	VkSemaphore m_imageAvailableSemaphore{ VK_NULL_HANDLE };
	VkSemaphore m_renderingFinishedSemaphore{ VK_NULL_HANDLE };
	VkCommandPool m_graphicsCommandPool{ VK_NULL_HANDLE };
	std::vector<SwapchainImage> m_swapChainImages;
	std::vector<VkFramebuffer> m_frameBuffers;
	std::vector<VkCommandBuffer> m_graphicsCommandBuffers;
	VkRenderPass m_renderPass{ VK_NULL_HANDLE };
	VkPipeline m_pipeline{ VK_NULL_HANDLE };
private:
    Ui::Tutorial01Class ui;
};
