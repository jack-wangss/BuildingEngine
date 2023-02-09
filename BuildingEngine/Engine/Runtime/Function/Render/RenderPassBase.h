#pragma once


namespace BE
{
	class Rhi;
	class RenderResourceBase;
	class WindowUI;

    struct RenderPassInitInfo
    {};

    struct RenderPassCommonInfo
    {
        std::shared_ptr<Rhi>                Rhi;
        std::shared_ptr<RenderResourceBase> RenderResource;
    };

    class RenderPassBase
    {
    public:
        virtual void Initialize(const RenderPassInitInfo* initInfo) = 0;
        virtual void PostInitialize();
        virtual void SetCommonInfo(RenderPassCommonInfo commonInfo);
        virtual void PreparePassData(std::shared_ptr<RenderResourceBase> renderResource);
        virtual void InitializeUIRenderBackend(WindowUI* windowUI);

    protected:
        std::shared_ptr<Rhi>                m_Rhi;
        std::shared_ptr<RenderResourceBase> m_RenderResource;
    };
}