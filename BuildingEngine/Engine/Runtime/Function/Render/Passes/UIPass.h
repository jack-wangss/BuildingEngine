#pragma once

#include "Runtime/Function/Render/RenderPass.h"


namespace BE
{
	class WindowUI;

    struct UIPassInitInfo : RenderPassInitInfo
    {
        VkRenderPass RenderPass;
    };

    class UIPass : public RenderPass
    {
    public:
        void Initialize(const RenderPassInitInfo* initInfo) override final;
        void InitializeUIRenderBackend(WindowUI* windowUI) override final;
        void Draw() override final;

    private:
        void UploadFonts();

    private:
        WindowUI* m_WindowUI;
    };


}