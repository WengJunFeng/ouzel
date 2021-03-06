// Copyright 2015-2019 Elviss Strazdins. All rights reserved.

#ifndef OUZEL_GRAPHICS_OGLRENDERTARGET_HPP
#define OUZEL_GRAPHICS_OGLRENDERTARGET_HPP

#include "core/Setup.h"

#if OUZEL_COMPILE_OPENGL

#include "graphics/opengl/OGL.h"

#if OUZEL_OPENGLES
#  include "GLES/gl.h"
#  include "GLES2/gl2.h"
#  include "GLES2/gl2ext.h"
#  include "GLES3/gl3.h"
#else
#  include "GL/glcorearb.h"
#  include "GL/glext.h"
#endif

#include <set>
#include "graphics/opengl/OGLRenderResource.hpp"
#include "math/Color.hpp"

namespace ouzel
{
    namespace graphics
    {
        class OGLRenderDevice;
        class OGLTexture;

        class OGLRenderTarget final: public OGLRenderResource
        {
        public:
            OGLRenderTarget(OGLRenderDevice& renderDeviceOGL,
                            const std::set<OGLTexture*>& initColorTextures,
                            OGLTexture* initDepthTexture);
            ~OGLRenderTarget();

            void reload() override;

            void setClearColorBuffer(bool clear);
            void setClearDepthBuffer(bool clear);
            void setClearColor(Color color);
            void setClearDepth(float newClearDepth);

            inline GLuint getFrameBufferId() const { return frameBufferId; }

        private:
            void createFrameBuffer();
            GLuint frameBufferId = 0;

            std::set<OGLTexture*> colorTextures;
            OGLTexture* depthTexture = nullptr;
        };
    } // namespace graphics
} // namespace ouzel

#endif

#endif // OUZEL_GRAPHICS_OGLRENDERTARGET_HPP
