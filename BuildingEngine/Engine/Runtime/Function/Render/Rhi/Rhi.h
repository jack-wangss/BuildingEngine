#pragma once

namespace BE
{
	class WindowSystem;

	struct RhiInitInfo
	{
		std::shared_ptr<WindowSystem> WindowSystem;
	};

	class Rhi
	{
	public:
		virtual ~Rhi() = 0;

		virtual void Initialize(RhiInitInfo initializeInfo) = 0;
		virtual void PrepareContext();

	protected:
		bool m_EnableValidationLayers{ true };
		bool m_EnableDebugUtilsLabel{ true };
		bool m_EnablePointLightShadow{ true };

		// used in descriptor pool creation
		uint32_t m_MaxVertexBlendingMeshCount{ 256 };
		uint32_t m_MaxMaterialCount{ 256 };
	};
	inline Rhi::~Rhi() = default;
}