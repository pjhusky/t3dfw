// put static functions here, without includes

#if ( STRIP_INCLUDES_FROM_INL != 1 )
#include "contextOpenGL.h"
#include "statusType.h"
#include "apiAbstractions.h"
#endif

static Status_t createRenderTargetTextures(
    const int32_t renderTargetW,
    const int32_t renderTargetH,
    std::vector< uint32_t >& fbos,
    Texture& colorRenderTargetTex,
    Texture& normalRenderTargetTex,
    Texture& silhouetteRenderTargetTex,
    Texture& depthRenderTargetTex) {

    glGenFramebuffers(fbos.size(), fbos.data());

    linAlg::i32vec3_t texDim{ renderTargetW, renderTargetH, 0 };

    glCheckError();

    Texture::Desc_t colorTexDesc; // GL_RGBA8
    colorTexDesc.texDim = texDim;
    colorTexDesc.numChannels = 4;
    colorTexDesc.channelType = eChannelType::i8;
    colorTexDesc.semantics = eSemantics::color;
    colorTexDesc.isMipMapped = true;
    colorRenderTargetTex.create(colorTexDesc);

    glCheckError();

    Texture::Desc_t normalTexDesc; // GL_RGB16F
    normalTexDesc.texDim = texDim;
    normalTexDesc.numChannels = 3;
    normalTexDesc.channelType = eChannelType::f16;
    normalTexDesc.semantics = eSemantics::color;
    normalTexDesc.isMipMapped = false;
    normalRenderTargetTex.create(normalTexDesc);

    glCheckError();

    Texture::Desc_t silhouetteTexDesc; // GL_R8
    silhouetteTexDesc.texDim = texDim;
    silhouetteTexDesc.numChannels = 1;
    silhouetteTexDesc.channelType = eChannelType::i8;
    silhouetteTexDesc.semantics = eSemantics::color;
    silhouetteTexDesc.isMipMapped = false;
    silhouetteRenderTargetTex.create(silhouetteTexDesc);

    glCheckError();

    Texture::Desc_t depthTexDesc; // GL_DEPTH_COMPONENT32F
    depthTexDesc.texDim = texDim;
    depthTexDesc.numChannels = 1;
    depthTexDesc.channelType = eChannelType::f32depth;
    depthTexDesc.semantics = eSemantics::depth;
    depthTexDesc.isMipMapped = false;
    depthRenderTargetTex.create(depthTexDesc);

    glCheckError();

    for (int32_t fboIdx = 0; fboIdx < fbos.size(); fboIdx++) {

        glBindFramebuffer(GL_FRAMEBUFFER, fbos[fboIdx]);

        std::array< GLenum, 2 > drawBuffersConfig{ GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT0 + 2 };
        glDrawBuffers(drawBuffersConfig.size(), drawBuffersConfig.data());

        int32_t colorTexIdx = 0;
        { // color texture
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + colorTexIdx, GL_TEXTURE_2D, colorRenderTargetTex.handle(), 0);

            if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) { return Status_t::ERROR(); }

            constexpr float clearColor[]{ 0.0f, 0.0f, 0.0f, 0.0f };
            glClearBufferfv(GL_COLOR, 0, clearColor);
        }
        glCheckError();
        { // silhouette texture
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + 1, GL_TEXTURE_2D, silhouetteRenderTargetTex.handle(), 0);
            if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) { return Status_t::ERROR(); }
            constexpr float clearColor[]{ 0.0f, 0.0f, 0.0f, 0.0f };
            glClearBufferfv(GL_COLOR, 2, clearColor);
        }
        int32_t normalTexIdx = 0;
        { // normal texture
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + 2, GL_TEXTURE_2D, normalRenderTargetTex.handle(), 0);

            if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) { return Status_t::ERROR(); }

            constexpr float clearColor[]{ 0.0f, 0.0f, 0.0f, 0.0f };
            glClearBufferfv(GL_COLOR, 1, clearColor);
        }
        glCheckError();
        //for ( int32_t depthTexIdx = 0; depthTexIdx < depthRenderTargetTex.size(); depthTexIdx++ ) { // depth texture
        int32_t depthTexIdx = 0;
        {
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthRenderTargetTex.handle(), 0);

            if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) { return Status_t::ERROR(); }

            constexpr float clearDepthValue = 1.0f;
            glClearBufferfv(GL_DEPTH, 0, &clearDepthValue);
        }
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    return Status_t::OK();
}

static void destroyRenderTargetTextures(
    std::vector< uint32_t >& fbos,
    Texture& colorRenderTargetTex,
    Texture& normalRenderTargetTex,
    Texture& silhouetteRenderTargetTex,
    Texture& depthRenderTargetTex) {

    Texture::unbindAllTextures();

    glBindFramebuffer(GL_FRAMEBUFFER, fbos[0]);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + 1, GL_TEXTURE_2D, 0, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + 2, GL_TEXTURE_2D, 0, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, 0, 0);


    glActiveTexture(GL_TEXTURE0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    colorRenderTargetTex.destroy();
    normalRenderTargetTex.destroy();
    silhouetteRenderTargetTex.destroy();
    depthRenderTargetTex.destroy();

    glDeleteFramebuffers(fbos.size(), fbos.data());
}

static void createScreenQuadMatrix(
    linAlg::mat3_t& screenQuadMat3,
    const linAlg::vec2_t& translationVec,
    const linAlg::vec2_t& screenQuadScaleVec) {
    linAlg::mat3_t screenTranslationMat3;
    linAlg::loadTranslationMatrix(screenTranslationMat3, translationVec);
    linAlg::mat3_t screenScaleMat3;
    linAlg::loadScaleMatrix(screenScaleMat3, linAlg::vec3_t{ screenQuadScaleVec[0], screenQuadScaleVec[1], 1.0f });
    linAlg::multMatrix(screenQuadMat3, screenTranslationMat3, screenScaleMat3);
}

static void createScreenQuadMatrix(linAlg::mat3_t& screenQuadMat3, const float screenQuadScale) {
    linAlg::mat3_t screenTranslationMat3;
    linAlg::loadTranslationMatrix(screenTranslationMat3, linAlg::vec2_t{ -1.0f + screenQuadScale, +1.0f - screenQuadScale });
    linAlg::mat3_t screenScaleMat3;
    linAlg::loadScaleMatrix(screenScaleMat3, linAlg::vec3_t{ screenQuadScale, screenQuadScale, 1.0f });
    linAlg::multMatrix(screenQuadMat3, screenTranslationMat3, screenScaleMat3);
}
