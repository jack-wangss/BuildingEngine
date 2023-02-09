#pragma once

#include "Runtime/Core/Math/Vector2.h"
#include "Runtime/Core/Math/Vector3.h"
#include "Runtime/Core/Math/Vector4.h"

namespace BE
{
	struct MeshVertex
	{
        struct VulkanMeshVertexPostition
        {
            Vector3 Position;
        };

        struct VulkanMeshVertexVaryingEnableBlending
        {
            Vector3 Normal;
            Vector3 Tangent;
        };

        struct VulkanMeshVertexVarying
        {
            Vector2 Texcoord;
        };

        struct VulkanMeshVertexJointBinding
        {
            int Indices[4];
            Vector4  Weights;
        };

        static std::array<VkVertexInputBindingDescription, 3> getBindingDescriptions()
        {
            std::array<VkVertexInputBindingDescription, 3> binding_descriptions{};

            // position
            binding_descriptions[0].binding = 0;
            binding_descriptions[0].stride = sizeof(VulkanMeshVertexPostition);
            binding_descriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
            // varying blending
            binding_descriptions[1].binding = 1;
            binding_descriptions[1].stride = sizeof(VulkanMeshVertexVaryingEnableBlending);
            binding_descriptions[1].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
            // varying
            binding_descriptions[2].binding = 2;
            binding_descriptions[2].stride = sizeof(VulkanMeshVertexVarying);
            binding_descriptions[2].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
            return binding_descriptions;
        }

        static std::array<VkVertexInputAttributeDescription, 4> getAttributeDescriptions()
        {
            std::array<VkVertexInputAttributeDescription, 4> attribute_descriptions{};

            // position
            attribute_descriptions[0].binding = 0;
            attribute_descriptions[0].location = 0;
            attribute_descriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
            attribute_descriptions[0].offset = offsetof(VulkanMeshVertexPostition, Position);

            // varying blending
            attribute_descriptions[1].binding = 1;
            attribute_descriptions[1].location = 1;
            attribute_descriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
            attribute_descriptions[1].offset = offsetof(VulkanMeshVertexVaryingEnableBlending, Normal);
            attribute_descriptions[2].binding = 1;
            attribute_descriptions[2].location = 2;
            attribute_descriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
            attribute_descriptions[2].offset = offsetof(VulkanMeshVertexVaryingEnableBlending, Tangent);

            // varying
            attribute_descriptions[3].binding = 2;
            attribute_descriptions[3].location = 3;
            attribute_descriptions[3].format = VK_FORMAT_R32G32_SFLOAT;
            attribute_descriptions[3].offset = offsetof(VulkanMeshVertexVarying, Texcoord);

            return attribute_descriptions;
        }
	};
}