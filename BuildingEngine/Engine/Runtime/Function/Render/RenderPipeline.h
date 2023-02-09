#pragma once

#include "Runtime/Function/Render/RenderPipelineBase.h"

namespace BE
{
    class RenderPipeline : public RenderPipelineBase
    {
    public:
        virtual void Initialize(RenderPipelineInitInfo initInfo) override final;

        virtual void DeferredRender(std::shared_ptr<Rhi>                rhi,
            std::shared_ptr<RenderResourceBase> renderResource) override final;
    };
}