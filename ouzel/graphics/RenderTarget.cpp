// Copyright 2015-2019 Elviss Strazdins. All rights reserved.

#include "RenderTarget.hpp"
#include "Renderer.hpp"
#include "RenderDevice.hpp"

namespace ouzel
{
    namespace graphics
    {
        RenderTarget::RenderTarget(Renderer& initRenderer,
                                   const std::vector<std::shared_ptr<Texture>>& initColorTextures,
                                   const std::shared_ptr<Texture>& initDepthTexture):
            resource(initRenderer),
            colorTextures(initColorTextures),
            depthTexture(initDepthTexture)
        {
            std::set<uintptr_t> colorTextureIds;

            for (const auto& colorTexture : colorTextures)
                colorTextureIds.insert(colorTexture ? colorTexture->getResource() : 0);

            initRenderer.addCommand(std::unique_ptr<Command>(new InitRenderTargetCommand(resource.getId(),
                                                                                         colorTextureIds,
                                                                                         depthTexture ? depthTexture->getResource() : 0)));
        }
    } // namespace graphics
} // namespace ouzel
