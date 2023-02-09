#pragma once

#include "Runtime/Function/Render/RenderResourceBase.h"

namespace BE
{
	struct IBLResource
	{
		VkImage       _brdfLUT_texture_image{ VK_NULL_HANDLE };
		VkImageView   _brdfLUT_texture_image_view{ VK_NULL_HANDLE };
		VkSampler     _brdfLUT_texture_sampler{ VK_NULL_HANDLE };
		VmaAllocation _brdfLUT_texture_image_allocation;

		VkImage       _irradiance_texture_image{ VK_NULL_HANDLE };
		VkImageView   _irradiance_texture_image_view{ VK_NULL_HANDLE };
		VkSampler     _irradiance_texture_sampler{ VK_NULL_HANDLE };
		VmaAllocation _irradiance_texture_image_allocation;

		VkImage       _specular_texture_image{ VK_NULL_HANDLE };
		VkImageView   _specular_texture_image_view{ VK_NULL_HANDLE };
		VkSampler     _specular_texture_sampler{ VK_NULL_HANDLE };
		VmaAllocation _specular_texture_image_allocation;
	};

	struct ColorGradingResource
	{
		VkImage       _color_grading_LUT_texture_image{ VK_NULL_HANDLE };
		VkImageView   _color_grading_LUT_texture_image_view{ VK_NULL_HANDLE };
		VmaAllocation _color_grading_LUT_texture_image_allocation;
	};



	struct StorageBuffer
	{
		// limits
		uint32_t _min_uniform_buffer_offset_alignment{ 256 };
		uint32_t _min_storage_buffer_offset_alignment{ 256 };
		uint32_t _max_storage_buffer_range{ 1 << 27 };
		uint32_t _non_coherent_atom_size{ 256 };

		VkBuffer              _global_upload_ringbuffer;
		VkDeviceMemory        _global_upload_ringbuffer_memory;
		void* _global_upload_ringbuffer_memory_pointer;
		std::vector<uint32_t> _global_upload_ringbuffers_begin;
		std::vector<uint32_t> _global_upload_ringbuffers_end;
		std::vector<uint32_t> _global_upload_ringbuffers_size;

		VkBuffer       _global_null_descriptor_storage_buffer;
		VkDeviceMemory _global_null_descriptor_storage_buffer_memory;

		// axis
		VkBuffer       _axis_inefficient_storage_buffer;
		VkDeviceMemory _axis_inefficient_storage_buffer_memory;
		void* _axis_inefficient_storage_buffer_memory_pointer;
	};
	struct GlobalRenderResource
	{
		IBLResource          _ibl_resource;
		ColorGradingResource _color_grading_resource;
		StorageBuffer        _storage_buffer;
	};
	class RenderResource : public RenderResourceBase
	{

	public:
		// global rendering resource, include IBL data, global storage buffer
		GlobalRenderResource m_GlobalRenderResource;
	};
}