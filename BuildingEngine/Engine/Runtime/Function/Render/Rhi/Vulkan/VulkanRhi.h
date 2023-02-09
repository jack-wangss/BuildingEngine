#pragma once

#include "Runtime/Function/Render/Rhi/Rhi.h"

namespace BE
{
	struct QueueFamilyIndices
	{
		std::optional<uint32_t> m_GraphicsFamily;
		std::optional<uint32_t> m_PresentFamily;
		std::optional<uint32_t> m_ComputeFamily;

		bool IsComplete() const
		{
			return m_GraphicsFamily.has_value() && m_PresentFamily.has_value() && m_ComputeFamily.has_value();
		}
	};

	struct SwapChainSupportDetails
	{
		VkSurfaceCapabilitiesKHR        m_Capabilities{};
		std::vector<VkSurfaceFormatKHR> m_Formats;
		std::vector<VkPresentModeKHR>   m_PresentModes;
	};

	class VulkanRhi final : public Rhi
	{
	public:
		// override functions
		virtual ~VulkanRhi() override final;
		virtual void Initialize(RhiInitInfo initInfo) override final;
		virtual void PrepareContext() override final;


		// swapchain
		void CreateSwapchain();
		void ClearSwapchain();
		void RecreateSwapchain();

		void CreateSwapchainImageViews();
		void CreateFramebufferImageAndView();

		// debug utilities label
		PFN_vkCmdBeginDebugUtilsLabelEXT m_VKCmdBeginDebugUtilsLabelExt;
		PFN_vkCmdEndDebugUtilsLabelEXT   m_VKCmdEndDebugUtilsLabelExt;
	private:
		void CreateInstance();
		void InitializeDebugMessenger();
		void CreateWindowSurface();
		void InitializePhysicalDevice();
		void CreateLogicalDevice();
		void CreateCommandPool();
		void CreateCommandBuffers();
		void CreateDescriptorPool();
		void CreateSyncPrimitives();
		void CreateAssetAllocator();

		bool                     CheckValidationLayerSupport();
		std::vector<const char*> GetRequiredExtensions();
		void                     PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

		VkResult CreateDebugUtilsMessengerEXT(VkInstance                                instance,
			const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
			const VkAllocationCallbacks* pAllocator,
			VkDebugUtilsMessengerEXT* pDebugMessenger);
		void     DestroyDebugUtilsMessengerEXT(VkInstance                   instance,
			VkDebugUtilsMessengerEXT     debugMessenger,
			const VkAllocationCallbacks* pAllocator);

		bool                    IsDeviceSuitable(VkPhysicalDevice physicalDevice);
		QueueFamilyIndices      FindQueueFamilies(VkPhysicalDevice physicalDevice);
		bool                    CheckDeviceExtensionSupport(VkPhysicalDevice physicalDevice);
		SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice physicalDevice);

		VkFormat FindDepthFormat();
		VkFormat FindSupportedFormat(const std::vector<VkFormat>& candidates,
			VkImageTiling                tiling,
			VkFormatFeatureFlags         features);

		VkSurfaceFormatKHR
			ChooseSwapchainSurfaceFormatFromDetails(const std::vector<VkSurfaceFormatKHR>& availableSurfaceFormats);
		VkPresentModeKHR
			ChooseSwapchainPresentModeFromDetails(const std::vector<VkPresentModeKHR>& availablePresentModes);
		VkExtent2D ChooseSwapchainExtentFromDetails(const VkSurfaceCapabilitiesKHR& capabilities);
	
	public:
		GLFWwindow* m_Window{ nullptr };

		VkInstance         m_Instance{ VK_NULL_HANDLE };
		VkSurfaceKHR       m_Surface{ VK_NULL_HANDLE };
		VkPhysicalDevice   m_PhysicalDevice{ VK_NULL_HANDLE };
		QueueFamilyIndices m_QueueIndices;
		VkDevice           m_Device{ VK_NULL_HANDLE };
		VkFormat           m_SepthImageFormat{ VK_FORMAT_UNDEFINED };
		VkQueue            m_GraphicsQueue{ VK_NULL_HANDLE };
		VkQueue            m_PresentQueue{ VK_NULL_HANDLE };
		VkQueue            m_ComputeQueue{ VK_NULL_HANDLE };
		VkCommandPool      m_CommandPool{ VK_NULL_HANDLE };
		VkFormat		   m_DepthImageFormat{ VK_FORMAT_UNDEFINED };

		VkSwapchainKHR           m_Swapchain{ VK_NULL_HANDLE };
		VkFormat                 m_SwapchainImageFormat{ VK_FORMAT_UNDEFINED };
		VkExtent2D               m_SwapchainExtent;
		std::vector<VkImage>     m_SwapchainImages;
		std::vector<VkImageView> m_SwapchainImageviews;

		VkImage        m_DepthImage{ VK_NULL_HANDLE };
		VkDeviceMemory m_DepthImageMemory{ VK_NULL_HANDLE };
		VkImageView    m_DepthImageView{ VK_NULL_HANDLE };

		// asset allocator use VMA library
		VmaAllocator				m_AssetsAllocator;

		// function pointers
		PFN_vkWaitForFences         m_VKWaitForFences;
		PFN_vkResetFences           m_VKResetFences;
		PFN_vkResetCommandPool      m_VKResetCommandPool;
		PFN_vkBeginCommandBuffer    m_VKBeginCommandBuffer;
		PFN_vkEndCommandBuffer      m_VKEndCommandBuffer;
		PFN_vkCmdBeginRenderPass    m_VKCmdBeginRenderPass;
		PFN_vkCmdNextSubpass        m_VKCmdNextSubpass;
		PFN_vkCmdEndRenderPass      m_VKCmdEndRenderPass;
		PFN_vkCmdBindPipeline       m_VKCmdBindPipeline;
		PFN_vkCmdSetViewport        m_VKCmdSetViewport;
		PFN_vkCmdSetScissor         m_VKCmdSetScissor;
		PFN_vkCmdBindVertexBuffers  m_VKCmdBindVertexBuffers;
		PFN_vkCmdBindIndexBuffer    m_VKCmdBindIndexBuffer;
		PFN_vkCmdBindDescriptorSets m_VKCmdBindDescriptorSets;
		PFN_vkCmdDrawIndexed        m_VKCmdDrawIndexed;
		PFN_vkCmdClearAttachments   m_VKCmdClearAttachments;

		// global descriptor pool
		VkDescriptorPool   m_DescriptorPool;

		// command pool and buffers
		static uint8_t const s_MaxFramesInFlight{ 3 };

		VkCommandPool	m_CommandPools[s_MaxFramesInFlight];
		VkCommandBuffer m_CommandBuffers[s_MaxFramesInFlight];
		VkSemaphore     m_ImageAvailableForRenderSemaphores[s_MaxFramesInFlight];
		VkSemaphore     m_ImageAvailableForTexturescopySemaphores[s_MaxFramesInFlight];
		VkSemaphore     m_ImageFinishedForPresentationSemaphores[s_MaxFramesInFlight];
		VkFence         m_IsFrameInFlightFences[s_MaxFramesInFlight];


		//VkCommandBuffer  m_current_command_buffer;
		//uint8_t* m_p_current_frame_index{ nullptr };
		//VkCommandPool* m_p_command_pools{ nullptr };
		//VkCommandBuffer* m_p_command_buffers{ nullptr };
		VkViewport       m_Viewport;
		VkRect2D         m_Scissor;



	private:
		const std::vector<char const*> m_ValidationLayers{ "VK_LAYER_KHRONOS_validation" };
		uint32_t                       m_VulkanApiVersion{ VK_API_VERSION_1_0 };
		
		std::vector<char const*> m_DeviceExtensions{ VK_KHR_SWAPCHAIN_EXTENSION_NAME };
	
		VkDebugUtilsMessengerEXT m_DebugMessenger{ VK_NULL_HANDLE };
};
}