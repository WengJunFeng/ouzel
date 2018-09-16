// Copyright 2015-2018 Elviss Strazdins. All rights reserved.

#pragma once

#include "graphics/RenderResource.hpp"

namespace ouzel
{
    namespace graphics
    {
        class RenderResourceOGL: public RenderResource
        {
        public:
            RenderResourceOGL(RenderDevice& renderDevice):
                RenderResource(renderDevice) {}

            virtual void reload() = 0;
        };
    } // namespace graphics
} // namespace ouzel
