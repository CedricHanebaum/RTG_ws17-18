// This file is auto-generated and should not be modified directly.
#include "Texture2DMultisampleArray.hh"

#include <glm/glm.hpp>

#include "glow/data/SurfaceData.hh"
#include "glow/data/TextureData.hh"

#include "glow/glow.hh"
#include "glow/limits.hh"
#include "glow/common/runtime_assert.hh"
#include "glow/common/ogl_typeinfo.hh"
#include "glow/common/scoped_gl.hh"
#include "glow/common/thread_local.hh"

using namespace glow;

/// Currently bound buffer
static GLOW_THREADLOCAL Texture2DMultisampleArray::BoundTexture2DMultisampleArray *sCurrentTexture = nullptr;

Texture2DMultisampleArray::BoundTexture2DMultisampleArray *Texture2DMultisampleArray::getCurrentTexture()
{
    return sCurrentTexture;
}


Texture2DMultisampleArray::Texture2DMultisampleArray (GLenum internalFormat)
  : Texture(GL_TEXTURE_2D_MULTISAMPLE_ARRAY, GL_TEXTURE_BINDING_2D_MULTISAMPLE_ARRAY, internalFormat)
{
}

SharedTexture2DMultisampleArray Texture2DMultisampleArray::create(size_t width, size_t height, size_t layers, GLenum internalFormat)
{
    auto tex = std::make_shared<Texture2DMultisampleArray>(internalFormat);
    tex->bind().resize(width, height, layers);
    return tex;
}



bool Texture2DMultisampleArray::BoundTexture2DMultisampleArray::isCurrent() const
{
    GLOW_RUNTIME_ASSERT(sCurrentTexture == this, "Currently bound FBO does NOT match represented Texture", return false);
    return true;
}


void Texture2DMultisampleArray::BoundTexture2DMultisampleArray::setMinFilter(GLenum filter)
{
    if (!isCurrent())
        return;
    checkValidGLOW();

    auto corrected = false;
    switch (filter) {
        case GL_NEAREST_MIPMAP_NEAREST:
        case GL_NEAREST_MIPMAP_LINEAR:
            filter = GL_NEAREST;
            corrected = true;
            break;
        case GL_LINEAR_MIPMAP_NEAREST:
        case GL_LINEAR_MIPMAP_LINEAR:
            filter = GL_LINEAR;
            corrected = true;
            break;
    }
    if (corrected)
        warning() << "Texture2DMultisampleArray does not support MipMapping.";

    glTexParameteri(texture->mTarget, GL_TEXTURE_MIN_FILTER, filter);
    texture->mMinFilter = filter;
}

void Texture2DMultisampleArray::BoundTexture2DMultisampleArray::setMagFilter(GLenum filter)
{
    if (!isCurrent())
        return;
    checkValidGLOW();

    glTexParameteri(texture->mTarget, GL_TEXTURE_MAG_FILTER, filter);
    texture->mMagFilter = filter;
}

void Texture2DMultisampleArray::BoundTexture2DMultisampleArray::setFilter(GLenum magFilter, GLenum minFilter)
{
    if (!isCurrent())
        return;
    checkValidGLOW();

    glTexParameteri(texture->mTarget, GL_TEXTURE_MIN_FILTER, minFilter);
    glTexParameteri(texture->mTarget, GL_TEXTURE_MAG_FILTER, magFilter);
    texture->mMinFilter = minFilter;
    texture->mMagFilter = magFilter;
}

void Texture2DMultisampleArray::BoundTexture2DMultisampleArray::setAnisotropicFiltering(GLfloat samples)
{
    if (!isCurrent())
        return;
    checkValidGLOW();

    samples = glm::clamp(samples, 1.f, limits::maxAnisotropy);
    glTexParameterf(texture->mTarget, GL_TEXTURE_MAX_ANISOTROPY_EXT, samples);
    texture->mAnisotropicFiltering = samples;
}

void Texture2DMultisampleArray::BoundTexture2DMultisampleArray::setWrapS(GLenum wrap)
{
    if (!isCurrent())
        return;
    checkValidGLOW();

    glTexParameteri(texture->mTarget, GL_TEXTURE_WRAP_S, wrap);
    texture->mWrapS = wrap;
}
void Texture2DMultisampleArray::BoundTexture2DMultisampleArray::setWrapT(GLenum wrap)
{
    if (!isCurrent())
        return;
    checkValidGLOW();

    glTexParameteri(texture->mTarget, GL_TEXTURE_WRAP_T, wrap);
    texture->mWrapT = wrap;
}

void Texture2DMultisampleArray::BoundTexture2DMultisampleArray::setWrap(GLenum wrapS, GLenum wrapT)
{
    if (!isCurrent())
        return;
    checkValidGLOW();

    glTexParameteri(texture->mTarget, GL_TEXTURE_WRAP_S, wrapS);
    texture->mWrapS = wrapS;
    glTexParameteri(texture->mTarget, GL_TEXTURE_WRAP_T, wrapT);
    texture->mWrapT = wrapT;
}


void Texture2DMultisampleArray::BoundTexture2DMultisampleArray::resize(size_t width, size_t height, size_t layers)
{
    if (!isCurrent())
        return;
    checkValidGLOW();


    texture->mWidth = width;
    texture->mHeight = height;
    texture->mLayers = layers;


    auto const samples = 4; // TODO: configurable
    auto const fixedSampleLocs = GL_FALSE; // TODO: configurable
    glTexImage3DMultisample(texture->mTarget, samples, texture->mInternalFormat, width, height, layers, fixedSampleLocs);

}


Texture2DMultisampleArray::BoundTexture2DMultisampleArray::BoundTexture2DMultisampleArray (Texture2DMultisampleArray *texture) : texture(texture)
{
    checkValidGLOW();
    glGetIntegerv(texture->mBindingTarget, &previousTexture);
    glActiveTexture(GL_TEXTURE0 + limits::maxCombinedTextureImageUnits - 1);
    glBindTexture(texture->mTarget, texture->mObjectName);

    previousTexturePtr = sCurrentTexture;
    sCurrentTexture = this;
}

Texture2DMultisampleArray::BoundTexture2DMultisampleArray::BoundTexture2DMultisampleArray (Texture2DMultisampleArray::BoundTexture2DMultisampleArray &&rhs)
  : texture(rhs.texture), previousTexture(rhs.previousTexture), previousTexturePtr(rhs.previousTexturePtr)
{
    // invalidate rhs
    rhs.previousTexture = -1;
}

Texture2DMultisampleArray::BoundTexture2DMultisampleArray::~BoundTexture2DMultisampleArray ()
{
    if (previousTexture != -1) // if valid
    {
        checkValidGLOW();
        glActiveTexture(GL_TEXTURE0 + limits::maxCombinedTextureImageUnits - 1);
        glBindTexture(texture->mTarget, previousTexture);
        sCurrentTexture = previousTexturePtr;
    }
}
