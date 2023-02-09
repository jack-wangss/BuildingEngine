#include "pch.h"
#include "VulkanRhi.h"

#include "Runtime/Function/Render/WindowSystem.h"
#include "Runtime/Function/Render/Rhi/Vulkan/VulkanUtil.h"

namespace BE
{
	VulkanRhi::~VulkanRhi()
	{
	}

	void VulkanRhi::Initialize(RhiInitInfo initInfo)
	{
		m_Window = initInfo.WindowSystem->GetWindow();

		std::array<int, 2> windowSize = initInfo.WindowSystem->GetWindowSize();

		m_Viewport = { 0.0f, 0.0f, (float)windowSize[0], (float)windowSize[1], 0.0f, 1.0f };
		m_Scissor = { {0, 0}, {(uint32_t)windowSize[0], (uint32_t)windowSize[1]} };

#ifdef _DEBUG
		m_EnableDebugUtilsLabel = true;
		m_EnableValidationLayers = true;
#else
		m_EnableDebugUtilsLabel = false;
		m_EnableValidationLayers = false;
#endif // DEBUG

		CreateInstance();
		InitializeDebugMessenger();
		CreateWindowSurface();
		InitializePhysicalDevice();
		CreateLogicalDevice();
		CreateCommandPool();
		CreateCommandBuffers();
		CreateDescriptorPool();
		CreateSyncPrimitives();

		CreateSwapchain();
		CreateSwapchainImageViews();
		CreateFramebufferImageAndView();

		CreateAssetAllocator();
	}

	void VulkanRhi::PrepareContext()
	{
		//m_p_current_frame_index = &m_current_frame_index;
		//m_current_command_buffer = m_command_buffers[m_current_frame_index];
		//m_p_command_buffers = m_command_buffers;
		//m_p_command_pools = m_CommandPools;
	}

	void VulkanRhi::CreateSwapchain()
	{
		// query all supports of this physical device
		SwapChainSupportDetails swapchainSupportDetails = QuerySwapChainSupport(m_PhysicalDevice);

		// choose the best or fitting format
		VkSurfaceFormatKHR chosenSurfaceFormat =
			ChooseSwapchainSurfaceFormatFromDetails(swapchainSupportDetails.m_Formats);
		// choose the best or fitting present mode
		VkPresentModeKHR chosenPresentMode =
			ChooseSwapchainPresentModeFromDetails(swapchainSupportDetails.m_PresentModes);
		// choose the best or fitting extent
		VkExtent2D chosenExtent = ChooseSwapchainExtentFromDetails(swapchainSupportDetails.m_Capabilities);

		uint32_t imageCount = swapchainSupportDetails.m_Capabilities.minImageCount + 1;
		if (swapchainSupportDetails.m_Capabilities.maxImageCount > 0 &&
			imageCount > swapchainSupportDetails.m_Capabilities.maxImageCount)
		{
			imageCount = swapchainSupportDetails.m_Capabilities.maxImageCount;
		}

		VkSwapchainCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = m_Surface;

		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = chosenSurfaceFormat.format;
		createInfo.imageColorSpace = chosenSurfaceFormat.colorSpace;
		createInfo.imageExtent = chosenExtent;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		uint32_t queueFamilyIndices[] = { m_QueueIndices.m_GraphicsFamily.value(),
										 m_QueueIndices.m_PresentFamily.value() };

		if (m_QueueIndices.m_GraphicsFamily != m_QueueIndices.m_PresentFamily)
		{
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
		}
		else
		{
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		}

		createInfo.preTransform = swapchainSupportDetails.m_Capabilities.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.presentMode = chosenPresentMode;
		createInfo.clipped = VK_TRUE;

		createInfo.oldSwapchain = VK_NULL_HANDLE;

		if (vkCreateSwapchainKHR(m_Device, &createInfo, nullptr, &m_Swapchain) != VK_SUCCESS)
		{
			throw std::runtime_error("vk create swapchain khr");
		}

		vkGetSwapchainImagesKHR(m_Device, m_Swapchain, &imageCount, nullptr);
		m_SwapchainImages.resize(imageCount);
		vkGetSwapchainImagesKHR(m_Device, m_Swapchain, &imageCount, m_SwapchainImages.data());

		m_SwapchainImageFormat = chosenSurfaceFormat.format;
		m_SwapchainExtent = chosenExtent;

		m_Scissor = { {0, 0}, {m_SwapchainExtent.width, m_SwapchainExtent.height} };
	}

	void VulkanRhi::ClearSwapchain()
	{
	}

	void VulkanRhi::RecreateSwapchain()
	{
	}

	void VulkanRhi::CreateSwapchainImageViews()
	{
		m_SwapchainImageviews.resize(m_SwapchainImages.size());

		// create imageview (one for each this time) for all swapchain images
		for (size_t i = 0; i < m_SwapchainImages.size(); i++)
		{
			m_SwapchainImageviews[i] = VulkanUtil::CreateImageView(m_Device,
				m_SwapchainImages[i],
				m_SwapchainImageFormat,
				VK_IMAGE_ASPECT_COLOR_BIT,
				VK_IMAGE_VIEW_TYPE_2D,
				1,
				1);
		}
	}

	void VulkanRhi::CreateFramebufferImageAndView()
	{
		VulkanUtil::CreateImage(m_PhysicalDevice,
			m_Device,
			m_SwapchainExtent.width,
			m_SwapchainExtent.height,
			m_DepthImageFormat,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT |
			VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			m_DepthImage,
			m_DepthImageMemory,
			0,
			1,
			1);

		m_DepthImageView = VulkanUtil::CreateImageView(
			m_Device, m_DepthImage, m_DepthImageFormat, VK_IMAGE_ASPECT_DEPTH_BIT, VK_IMAGE_VIEW_TYPE_2D, 1, 1);
	}

	void VulkanRhi::CreateInstance()
	{
		// validation layer will be enabled in debug mode
		if (m_EnableValidationLayers && !CheckValidationLayerSupport())
		{
			throw std::runtime_error("validation layers requested, but not available!");
		}

		m_VulkanApiVersion = VK_API_VERSION_1_0;

		// app info
		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "BuildingEditor";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "BuildingEngine";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = m_VulkanApiVersion;

		// create info
		VkInstanceCreateInfo instance_create_info{};
		instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		instance_create_info.pApplicationInfo = &appInfo; // the appInfo is stored here

		auto extensions = GetRequiredExtensions();
		instance_create_info.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		instance_create_info.ppEnabledExtensionNames = extensions.data();

		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
		if (m_EnableValidationLayers)
		{
			instance_create_info.enabledLayerCount = static_cast<uint32_t>(m_ValidationLayers.size());
			instance_create_info.ppEnabledLayerNames = m_ValidationLayers.data();

			PopulateDebugMessengerCreateInfo(debugCreateInfo);
			instance_create_info.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
		}
		else
		{
			instance_create_info.enabledLayerCount = 0;
			instance_create_info.pNext = nullptr;
		}

		// create m_vulkan_context._instance
		if (vkCreateInstance(&instance_create_info, nullptr, &m_Instance) != VK_SUCCESS)
		{
			throw std::runtime_error("vk create instance");
		}
	}

	void VulkanRhi::InitializeDebugMessenger()
	{
		if (m_EnableValidationLayers)
		{
			VkDebugUtilsMessengerCreateInfoEXT createInfo;
			PopulateDebugMessengerCreateInfo(createInfo);
			if (VK_SUCCESS != CreateDebugUtilsMessengerEXT(m_Instance, &createInfo, nullptr, &m_DebugMessenger))
			{
				throw std::runtime_error("failed to set up debug messenger!");
			}
		}

		if (m_EnableDebugUtilsLabel)
		{
			m_VKCmdBeginDebugUtilsLabelExt =
				(PFN_vkCmdBeginDebugUtilsLabelEXT)vkGetInstanceProcAddr(m_Instance, "vkCmdBeginDebugUtilsLabelEXT");
			m_VKCmdEndDebugUtilsLabelExt =
				(PFN_vkCmdEndDebugUtilsLabelEXT)vkGetInstanceProcAddr(m_Instance, "vkCmdEndDebugUtilsLabelEXT");
		}
	}

	void VulkanRhi::CreateWindowSurface()
	{
		if (glfwCreateWindowSurface(m_Instance, m_Window, nullptr, &m_Surface) != VK_SUCCESS)
		{
			throw std::runtime_error("glfwCreateWindowSurface");
		}
	}

	void VulkanRhi::InitializePhysicalDevice()
	{
		uint32_t physicalDevice_count;
		vkEnumeratePhysicalDevices(m_Instance, &physicalDevice_count, nullptr);
		if (physicalDevice_count == 0)
		{
			throw std::runtime_error("enumerate physical devices");
		}
		else
		{
			// find one device that matches our requirement
			// or find which is the best
			std::vector<VkPhysicalDevice> physicalDevices(physicalDevice_count);
			vkEnumeratePhysicalDevices(m_Instance, &physicalDevice_count, physicalDevices.data());

			std::vector<std::pair<int, VkPhysicalDevice>> ranked_physicalDevices;
			for (const auto& device : physicalDevices)
			{
				VkPhysicalDeviceProperties physicalDevice_properties;
				vkGetPhysicalDeviceProperties(device, &physicalDevice_properties);
				int score = 0;

				if (physicalDevice_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
				{
					score += 1000;
				}
				else if (physicalDevice_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
				{
					score += 100;
				}

				ranked_physicalDevices.push_back({ score, device });
			}

			std::sort(ranked_physicalDevices.begin(),
				ranked_physicalDevices.end(),
				[](const std::pair<int, VkPhysicalDevice>& p1, const std::pair<int, VkPhysicalDevice>& p2) {
					return p1 > p2;
				});

			for (const auto& device : ranked_physicalDevices)
			{
				if (IsDeviceSuitable(device.second))
				{
					m_PhysicalDevice = device.second;
					break;
				}
			}

			if (m_PhysicalDevice == VK_NULL_HANDLE)
			{
				throw std::runtime_error("failed to find suitable physical device");
			}
		}
	}

	void VulkanRhi::CreateLogicalDevice()
	{
		m_QueueIndices = FindQueueFamilies(m_PhysicalDevice);

		std::vector<VkDeviceQueueCreateInfo> queue_create_infos; // all queues that need to be created
		std::set<uint32_t>                   queue_families = { m_QueueIndices.m_GraphicsFamily.value(),
											 m_QueueIndices.m_PresentFamily.value(),
											 m_QueueIndices.m_ComputeFamily.value() };

		float queue_priority = 1.0f;
		for (uint32_t queue_family : queue_families) // for every queue family
		{
			// queue create info
			VkDeviceQueueCreateInfo queue_create_info{};
			queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queue_create_info.queueFamilyIndex = queue_family;
			queue_create_info.queueCount = 1;
			queue_create_info.pQueuePriorities = &queue_priority;
			queue_create_infos.push_back(queue_create_info);
		}

		// physical device features
		VkPhysicalDeviceFeatures physicalDevice_features = {};

		physicalDevice_features.samplerAnisotropy = VK_TRUE;

		// support inefficient readback storage buffer
		physicalDevice_features.fragmentStoresAndAtomics = VK_TRUE;

		// support independent blending
		physicalDevice_features.independentBlend = VK_TRUE;

		// support geometry shader
		if (m_EnablePointLightShadow)
		{
			physicalDevice_features.geometryShader = VK_TRUE;
		}

		// device create info
		VkDeviceCreateInfo device_create_info{};
		device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		device_create_info.pQueueCreateInfos = queue_create_infos.data();
		device_create_info.queueCreateInfoCount = static_cast<uint32_t>(queue_create_infos.size());
		device_create_info.pEnabledFeatures = &physicalDevice_features;
		device_create_info.enabledExtensionCount = static_cast<uint32_t>(m_DeviceExtensions.size());
		device_create_info.ppEnabledExtensionNames = m_DeviceExtensions.data();
		device_create_info.enabledLayerCount = 0;

		if (vkCreateDevice(m_PhysicalDevice, &device_create_info, nullptr, &m_Device) != VK_SUCCESS)
		{
			throw std::runtime_error("vk create device");
		}

		// initialize queues of this device
		vkGetDeviceQueue(m_Device, m_QueueIndices.m_GraphicsFamily.value(), 0, &m_GraphicsQueue);
		vkGetDeviceQueue(m_Device, m_QueueIndices.m_PresentFamily.value(), 0, &m_PresentQueue);
		vkGetDeviceQueue(m_Device, m_QueueIndices.m_ComputeFamily.value(), 0, &m_ComputeQueue);

		// more efficient pointer
		m_VKWaitForFences = (PFN_vkWaitForFences)vkGetDeviceProcAddr(m_Device, "vkWaitForFences");
		m_VKResetFences = (PFN_vkResetFences)vkGetDeviceProcAddr(m_Device, "vkResetFences");
		m_VKResetCommandPool = (PFN_vkResetCommandPool)vkGetDeviceProcAddr(m_Device, "vkResetCommandPool");
		m_VKBeginCommandBuffer = (PFN_vkBeginCommandBuffer)vkGetDeviceProcAddr(m_Device, "vkBeginCommandBuffer");
		m_VKEndCommandBuffer = (PFN_vkEndCommandBuffer)vkGetDeviceProcAddr(m_Device, "vkEndCommandBuffer");
		m_VKCmdBeginRenderPass = (PFN_vkCmdBeginRenderPass)vkGetDeviceProcAddr(m_Device, "vkCmdBeginRenderPass");
		m_VKCmdNextSubpass = (PFN_vkCmdNextSubpass)vkGetDeviceProcAddr(m_Device, "vkCmdNextSubpass");
		m_VKCmdEndRenderPass = (PFN_vkCmdEndRenderPass)vkGetDeviceProcAddr(m_Device, "vkCmdEndRenderPass");
		m_VKCmdBindPipeline = (PFN_vkCmdBindPipeline)vkGetDeviceProcAddr(m_Device, "vkCmdBindPipeline");
		m_VKCmdSetViewport = (PFN_vkCmdSetViewport)vkGetDeviceProcAddr(m_Device, "vkCmdSetViewport");
		m_VKCmdSetScissor = (PFN_vkCmdSetScissor)vkGetDeviceProcAddr(m_Device, "vkCmdSetScissor");
		m_VKCmdBindVertexBuffers = (PFN_vkCmdBindVertexBuffers)vkGetDeviceProcAddr(m_Device, "vkCmdBindVertexBuffers");
		m_VKCmdBindIndexBuffer = (PFN_vkCmdBindIndexBuffer)vkGetDeviceProcAddr(m_Device, "vkCmdBindIndexBuffer");
		m_VKCmdBindDescriptorSets = (PFN_vkCmdBindDescriptorSets)vkGetDeviceProcAddr(m_Device, "vkCmdBindDescriptorSets");
		m_VKCmdDrawIndexed = (PFN_vkCmdDrawIndexed)vkGetDeviceProcAddr(m_Device, "vkCmdDrawIndexed");
		m_VKCmdClearAttachments = (PFN_vkCmdClearAttachments)vkGetDeviceProcAddr(m_Device, "vkCmdClearAttachments");

		m_DepthImageFormat = FindDepthFormat();
		
	}

	void VulkanRhi::CreateCommandPool()
	{
		// default graphics command pool
		{
			VkCommandPoolCreateInfo command_pool_create_info{};
			command_pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
			command_pool_create_info.pNext = NULL;
			command_pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
			command_pool_create_info.queueFamilyIndex = m_QueueIndices.m_GraphicsFamily.value();

			if (vkCreateCommandPool(m_Device, &command_pool_create_info, nullptr, &m_CommandPool) != VK_SUCCESS)
			{
				throw std::runtime_error("vk create command pool");
			}
		}

		// other command pools
		{
			VkCommandPoolCreateInfo command_pool_create_info;
			command_pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
			command_pool_create_info.pNext = NULL;
			command_pool_create_info.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
			command_pool_create_info.queueFamilyIndex = m_QueueIndices.m_GraphicsFamily.value();

			for (uint32_t i = 0; i < s_MaxFramesInFlight; ++i)
			{
				if (vkCreateCommandPool(m_Device, &command_pool_create_info, NULL, &m_CommandPools[i]) != VK_SUCCESS)
				{
					throw std::runtime_error("vk create command pool");
				}
			}
		}
	}

	void VulkanRhi::CreateCommandBuffers()
	{
		VkCommandBufferAllocateInfo command_buffer_allocate_info{};
		command_buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		command_buffer_allocate_info.commandBufferCount = 1U;

		for (uint32_t i = 0; i < s_MaxFramesInFlight; ++i)
		{
			command_buffer_allocate_info.commandPool = m_CommandPools[i];

			if (vkAllocateCommandBuffers(m_Device, &command_buffer_allocate_info, &m_CommandBuffers[i]) != VK_SUCCESS)
			{
				throw std::runtime_error("vk allocate command buffers");
			}
		}
	}

	void VulkanRhi::CreateDescriptorPool()
	{
		// Since DescriptorSet should be treated as asset in Vulkan, DescriptorPool
		// should be big enough, and thus we can sub-allocate DescriptorSet from
		// DescriptorPool merely as we sub-allocate Buffer/Image from DeviceMemory.

		VkDescriptorPoolSize pool_sizes[6];
		pool_sizes[0].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
		pool_sizes[0].descriptorCount = 3 + 2 + 2 + 2 + 1 + 1 + 3 + 3;
		pool_sizes[1].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		pool_sizes[1].descriptorCount = 1 + 1 + 1 * m_MaxVertexBlendingMeshCount;
		pool_sizes[2].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		pool_sizes[2].descriptorCount = 1 * m_MaxMaterialCount;
		pool_sizes[3].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		pool_sizes[3].descriptorCount = 3 + 5 * m_MaxMaterialCount + 1 + 1; // ImGui_ImplVulkan_CreateDeviceObjects
		pool_sizes[4].type = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
		pool_sizes[4].descriptorCount = 4 + 1 + 1 + 2;
		pool_sizes[5].type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		pool_sizes[5].descriptorCount = 1;

		VkDescriptorPoolCreateInfo pool_info{};
		pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		pool_info.poolSizeCount = sizeof(pool_sizes) / sizeof(pool_sizes[0]);
		pool_info.pPoolSizes = pool_sizes;
		pool_info.maxSets = 1 + 1 + 1 + m_MaxMaterialCount + m_MaxVertexBlendingMeshCount + 1 +
			1; // +skybox + axis descriptor set
		pool_info.flags = 0U;

		if (vkCreateDescriptorPool(m_Device, &pool_info, nullptr, &m_DescriptorPool) != VK_SUCCESS)
		{
			throw std::runtime_error("create descriptor pool");
		}
	}

	void VulkanRhi::CreateSyncPrimitives()
	{
		VkSemaphoreCreateInfo semaphore_create_info{};
		semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fence_create_info{};
		fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT; // the fence is initialized as signaled

		for (uint32_t i = 0; i < s_MaxFramesInFlight; i++)
		{
			if (vkCreateSemaphore(
				m_Device, &semaphore_create_info, nullptr, &m_ImageAvailableForRenderSemaphores[i]) !=
				VK_SUCCESS ||
				vkCreateSemaphore(
					m_Device, &semaphore_create_info, nullptr, &m_ImageFinishedForPresentationSemaphores[i]) !=
				VK_SUCCESS ||
				vkCreateSemaphore(
					m_Device, &semaphore_create_info, nullptr, &m_ImageAvailableForTexturescopySemaphores[i]) !=
				VK_SUCCESS ||
				vkCreateFence(m_Device, &fence_create_info, nullptr, &m_IsFrameInFlightFences[i]) != VK_SUCCESS)
			{
				throw std::runtime_error("vk create semaphore & fence");
			}
		}
	}

	void VulkanRhi::CreateAssetAllocator()
	{
		VmaVulkanFunctions vulkanFunctions = {};
		vulkanFunctions.vkGetInstanceProcAddr = &vkGetInstanceProcAddr;
		vulkanFunctions.vkGetDeviceProcAddr = &vkGetDeviceProcAddr;

		VmaAllocatorCreateInfo allocatorCreateInfo = {};
		allocatorCreateInfo.vulkanApiVersion = m_VulkanApiVersion;
		allocatorCreateInfo.physicalDevice = m_PhysicalDevice;
		allocatorCreateInfo.device = m_Device;
		allocatorCreateInfo.instance = m_Instance;
		allocatorCreateInfo.pVulkanFunctions = &vulkanFunctions;

		vmaCreateAllocator(&allocatorCreateInfo, &m_AssetsAllocator);
	}

	bool VulkanRhi::CheckValidationLayerSupport()
	{
		uint32_t layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

		for (const char* layerName : m_ValidationLayers)
		{
			bool layerFound = false;

			for (const auto& layerProperties : availableLayers)
			{
				if (strcmp(layerName, layerProperties.layerName) == 0)
				{
					layerFound = true;
					break;
				}
			}

			if (!layerFound)
			{
				return false;
			}
		}

		return true;
	}

	std::vector<const char*> VulkanRhi::GetRequiredExtensions()
	{
		uint32_t     glfwExtensionCount = 0;
		const char** glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

		if (m_EnableValidationLayers || m_EnableValidationLayers)
		{
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}
		return extensions;
	}

	// debug callback
	static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT,
		VkDebugUtilsMessageTypeFlagsEXT,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void*)
	{
		std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
		return VK_FALSE;
	}
	void VulkanRhi::PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
	{
		createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo.messageSeverity =
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		createInfo.messageType =
			VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		createInfo.pfnUserCallback = DebugCallback;
	}

	VkResult VulkanRhi::CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
	{
		auto func =
			(PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
		if (func != nullptr)
		{
			return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
		}
		else
		{
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}
	}

	void VulkanRhi::DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
	{
		auto func =
			(PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
		if (func != nullptr)
		{
			func(instance, debugMessenger, pAllocator);
		}
	}

	bool VulkanRhi::IsDeviceSuitable(VkPhysicalDevice physicalDevice)
	{
		auto queue_indices = FindQueueFamilies(physicalDevice);
		bool isExtensionsSupported = CheckDeviceExtensionSupport(physicalDevice);
		bool isSwapchainAdequate = false;
		if (isExtensionsSupported)
		{
			SwapChainSupportDetails swapchainSupportDetails = QuerySwapChainSupport(physicalDevice);
			isSwapchainAdequate =
				!swapchainSupportDetails.m_Formats.empty() && !swapchainSupportDetails.m_PresentModes.empty();
		}

		VkPhysicalDeviceFeatures physicalDeviceFeatures;
		vkGetPhysicalDeviceFeatures(physicalDevice, &physicalDeviceFeatures);

		if (!queue_indices.IsComplete() || !isSwapchainAdequate || !physicalDeviceFeatures.samplerAnisotropy)
		{
			return false;
		}

		return true;
	}

	QueueFamilyIndices VulkanRhi::FindQueueFamilies(VkPhysicalDevice physicalDevice)
	{
		QueueFamilyIndices indices;
		uint32_t           queue_family_count = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queue_family_count, nullptr);
		std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queue_family_count, queue_families.data());

		int i = 0;
		for (const auto& queue_family : queue_families)
		{
			if (queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT) // if support graphics command queue
			{
				indices.m_GraphicsFamily = i;
			}

			if (queue_family.queueFlags & VK_QUEUE_COMPUTE_BIT) // if support compute command queue
			{
				indices.m_ComputeFamily = i;
			}

			VkBool32 is_present_support = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice,
				i,
				m_Surface,
				&is_present_support); // if support surface presentation
			if (is_present_support)
			{
				indices.m_PresentFamily = i;
			}

			if (indices.IsComplete())
			{
				break;
			}
			i++;
		}
		return indices;
	}

	bool VulkanRhi::CheckDeviceExtensionSupport(VkPhysicalDevice physicalDevice)
	{
		uint32_t extension_count;
		vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extension_count, nullptr);

		std::vector<VkExtensionProperties> available_extensions(extension_count);
		vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extension_count, available_extensions.data());

		std::set<std::string> required_extensions(m_DeviceExtensions.begin(), m_DeviceExtensions.end());
		for (const auto& extension : available_extensions)
		{
			required_extensions.erase(extension.extensionName);
		}

		return required_extensions.empty();
	}

	SwapChainSupportDetails VulkanRhi::QuerySwapChainSupport(VkPhysicalDevice physicalDevice)
	{
		SwapChainSupportDetails detailsResult;

		// capabilities
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, m_Surface, &detailsResult.m_Capabilities);

		// formats
		uint32_t format_count;
		vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, m_Surface, &format_count, nullptr);
		if (format_count != 0)
		{
			detailsResult.m_Formats.resize(format_count);
			vkGetPhysicalDeviceSurfaceFormatsKHR(
				physicalDevice, m_Surface, &format_count, detailsResult.m_Formats.data());
		}

		// present modes
		uint32_t presentmode_count;
		vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, m_Surface, &presentmode_count, nullptr);
		if (presentmode_count != 0)
		{
			detailsResult.m_PresentModes.resize(presentmode_count);
			vkGetPhysicalDeviceSurfacePresentModesKHR(
				physicalDevice, m_Surface, &presentmode_count, detailsResult.m_PresentModes.data());
		}

		return detailsResult;
	}

	VkFormat VulkanRhi::FindDepthFormat()
	{
		return FindSupportedFormat({ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
			VK_IMAGE_TILING_OPTIMAL,
			VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
	}

	VkFormat VulkanRhi::FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
	{
		for (VkFormat format : candidates)
		{
			VkFormatProperties props;
			vkGetPhysicalDeviceFormatProperties(m_PhysicalDevice, format, &props);

			if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
			{
				return format;
			}
			else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
			{
				return format;
			}
		}

		throw std::runtime_error("findSupportedFormat failed");
	}

	VkSurfaceFormatKHR VulkanRhi::ChooseSwapchainSurfaceFormatFromDetails(const std::vector<VkSurfaceFormatKHR>& availableSurfaceFormats)
	{
		for (const auto& surface_format : availableSurfaceFormats)
		{
			// TODO: select the VK_FORMAT_B8G8R8A8_SRGB surface format,
			// there is no need to do gamma correction in the fragment shader
			if (surface_format.format == VK_FORMAT_B8G8R8A8_UNORM &&
				surface_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			{
				return surface_format;
			}
		}
		return availableSurfaceFormats[0];
	}

	VkPresentModeKHR VulkanRhi::ChooseSwapchainPresentModeFromDetails(const std::vector<VkPresentModeKHR>& availablePresentModes)
	{
		for (VkPresentModeKHR present_mode : availablePresentModes)
		{
			if (VK_PRESENT_MODE_MAILBOX_KHR == present_mode)
			{
				return VK_PRESENT_MODE_MAILBOX_KHR;
			}
		}

		return VK_PRESENT_MODE_FIFO_KHR;
	}

	VkExtent2D VulkanRhi::ChooseSwapchainExtentFromDetails(const VkSurfaceCapabilitiesKHR& capabilities)
	{
		if (capabilities.currentExtent.width != UINT32_MAX)
		{
			return capabilities.currentExtent;
		}
		else
		{
			int width, height;
			glfwGetFramebufferSize(m_Window, &width, &height);

			VkExtent2D actualExtent = { static_cast<uint32_t>(width), static_cast<uint32_t>(height) };

			actualExtent.width =
				std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
			actualExtent.height =
				std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

			return actualExtent;
		}
	}

}