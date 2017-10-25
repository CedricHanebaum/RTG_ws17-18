// This file is auto-generated and should not be modified directly.
#include "Texture2DArray.hh"

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
static GLOW_THREADLOCAL Texture2DArray::BoundTexture2DArray *sCurrentTexture = nullptr;

Texture2DArray::BoundTexture2DArray *Texture2DArray::getCurrentTexture()
{
    return sCurrentTexture;
}

bool Texture2DArray::hasMipmapsEnabled() const
{
    switch (mMinFilter)
    {
    case GL_LINEAR_MIPMAP_LINEAR:
    case GL_LINEAR_MIPMAP_NEAREST:
    case GL_NEAREST_MIPMAP_LINEAR:
    case GL_NEAREST_MIPMAP_NEAREST:
        return true;

    default:
        return false;
    }
}

Texture2DArray::Texture2DArray (GLenum internalFormat)
  : Texture(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BINDING_2D_ARRAY, internalFormat)
{
}

SharedTexture2DArray Texture2DArray::create(size_t width, size_t height, size_t layers, GLenum internalFormat)
{
    auto tex = std::make_shared<Texture2DArray>(internalFormat);
    tex->bind().resize(width, height, layers);
    return tex;
}

SharedTexture2DArray Texture2DArray::createStorageImmutable(size_t width, size_t height, size_t layers, GLenum internalFormat, int mipmapLevels)
{
    auto tex = std::make_shared<Texture2DArray>(internalFormat);
    tex->bind().makeStorageImmutable(width, height, layers, internalFormat, mipmapLevels);
    return tex;
}

SharedTexture2DArray Texture2DArray::createFromFile(const std::string &filename, ColorSpace colorSpace)
{
    return createFromData(TextureData::createFromFile(filename, colorSpace));
}

SharedTexture2DArray Texture2DArray::createFromFile(const std::string &filename, GLenum internalFormat, ColorSpace colorSpace)
{
    return createFromData(TextureData::createFromFile(filename, colorSpace), internalFormat);
}

SharedTexture2DArray Texture2DArray::createFromData(const SharedTextureData &data)
{
    if (!data)
    {
        error() << "Texture2DArray::createFromData failed, no data provided";
        return nullptr;
    }

    if (data->getPreferredInternalFormat() == GL_INVALID_ENUM)
    {
        error() << "Texture2DArray::createFromData failed, no preferred internal format specified";
        return nullptr;
    }

    auto tex = std::make_shared<Texture2DArray>(data->getPreferredInternalFormat());
    tex->bind().setData(data->getPreferredInternalFormat(), data);
    return tex;
}

SharedTexture2DArray Texture2DArray::createFromData(const SharedTextureData &data, GLenum internalFormat)
{
    if (!data)
    {
        error() << "Texture2DArray::createFromData failed, no data provided";
        return nullptr;
    }

    auto tex = std::make_shared<Texture2DArray>(internalFormat);
    tex->bind().setData(internalFormat, data);
    return tex;
}

bool Texture2DArray::BoundTexture2DArray::isCurrent() const
{
    GLOW_RUNTIME_ASSERT(sCurrentTexture == this, "Currently bound FBO does NOT match represented Texture", return false);
    return true;
}

void Texture2DArray::BoundTexture2DArray::makeStorageImmutable(size_t width, size_t height, size_t layers, GLenum internalFormat, int mipmapLevels)
{
    if (!isCurrent())
        return;

    GLOW_RUNTIME_ASSERT(!texture->isStorageImmutable(), "Texture is already immutable", return );
    checkValidGLOW();

    if (mipmapLevels <= 0)
        mipmapLevels = glm::floor(glm::log2(glm::max((float)width, (float)height))) + 1;

    texture->mStorageImmutable = true;
    texture->mInternalFormat = internalFormat;
    texture->mWidth = width;
    texture->mHeight = height;
    texture->mLayers = layers;
    glTexStorage3D(texture->mTarget, mipmapLevels, internalFormat, width, height, layers);
}

void Texture2DArray::BoundTexture2DArray::setMinFilter(GLenum filter)
{
    if (!isCurrent())
        return;
    checkValidGLOW();


    glTexParameteri(texture->mTarget, GL_TEXTURE_MIN_FILTER, filter);
    texture->mMinFilter = filter;
}

void Texture2DArray::BoundTexture2DArray::setMagFilter(GLenum filter)
{
    if (!isCurrent())
        return;
    checkValidGLOW();

    glTexParameteri(texture->mTarget, GL_TEXTURE_MAG_FILTER, filter);
    texture->mMagFilter = filter;
}

void Texture2DArray::BoundTexture2DArray::setFilter(GLenum magFilter, GLenum minFilter)
{
    if (!isCurrent())
        return;
    checkValidGLOW();

    glTexParameteri(texture->mTarget, GL_TEXTURE_MIN_FILTER, minFilter);
    glTexParameteri(texture->mTarget, GL_TEXTURE_MAG_FILTER, magFilter);
    texture->mMinFilter = minFilter;
    texture->mMagFilter = magFilter;
}

void Texture2DArray::BoundTexture2DArray::setAnisotropicFiltering(GLfloat samples)
{
    if (!isCurrent())
        return;
    checkValidGLOW();

    samples = glm::clamp(samples, 1.f, limits::maxAnisotropy);
    glTexParameterf(texture->mTarget, GL_TEXTURE_MAX_ANISOTROPY_EXT, samples);
    texture->mAnisotropicFiltering = samples;
}

void Texture2DArray::BoundTexture2DArray::setWrapS(GLenum wrap)
{
    if (!isCurrent())
        return;
    checkValidGLOW();

    glTexParameteri(texture->mTarget, GL_TEXTURE_WRAP_S, wrap);
    texture->mWrapS = wrap;
}
void Texture2DArray::BoundTexture2DArray::setWrapT(GLenum wrap)
{
    if (!isCurrent())
        return;
    checkValidGLOW();

    glTexParameteri(texture->mTarget, GL_TEXTURE_WRAP_T, wrap);
    texture->mWrapT = wrap;
}

void Texture2DArray::BoundTexture2DArray::setWrap(GLenum wrapS, GLenum wrapT)
{
    if (!isCurrent())
        return;
    checkValidGLOW();

    glTexParameteri(texture->mTarget, GL_TEXTURE_WRAP_S, wrapS);
    texture->mWrapS = wrapS;
    glTexParameteri(texture->mTarget, GL_TEXTURE_WRAP_T, wrapT);
    texture->mWrapT = wrapT;
}

void Texture2DArray::BoundTexture2DArray::generateMipmaps()
{
    if (!isCurrent())
        return;
    checkValidGLOW();

    glGenerateMipmap(texture->mTarget);
    texture->mMipmapsGenerated = true;
}

void Texture2DArray::BoundTexture2DArray::resize(size_t width, size_t height, size_t layers)
{
    if (!isCurrent())
        return;
    checkValidGLOW();

    GLOW_RUNTIME_ASSERT(!texture->isStorageImmutable(), "Texture is storage immutable", return );

    texture->mWidth = width;
    texture->mHeight = height;
    texture->mLayers = layers;

    GLenum format = GL_RGBA;
    auto isIntegerFormat = isIntegerInternalFormat(texture->mInternalFormat);
    switch (channelsOfInternalFormat(texture->mInternalFormat))
    {
    case 1:
        format = isIntegerFormat ? GL_RED_INTEGER : GL_RED;
        break;
    case 2:
        format = isIntegerFormat ? GL_RG_INTEGER : GL_RG;
        break;
    case 3:
        format = isIntegerFormat ? GL_RGB_INTEGER : GL_RGB;
        break;
    case 4:
        format = isIntegerFormat ? GL_RGBA_INTEGER : GL_RGBA;
        break;
    }
    switch (texture->mInternalFormat)
    {
    case GL_DEPTH_COMPONENT:
    case GL_DEPTH_COMPONENT16:
    case GL_DEPTH_COMPONENT24:
    case GL_DEPTH_COMPONENT32:
    case GL_DEPTH_COMPONENT32F:
        format = GL_DEPTH_COMPONENT;
        break;
    }

    glTexImage3D(texture->mTarget, 0, texture->mInternalFormat, width, height, layers, 0, format, GL_UNSIGNED_BYTE, nullptr);
}

void Texture2DArray::BoundTexture2DArray::setData(GLenum internalFormat, size_t width, size_t height, size_t layers, GLenum format, GLenum type, const GLvoid *data, int mipmapLevel)
{
    if (!isCurrent())
        return;
    checkValidGLOW();

    if (texture->isStorageImmutable())
    {
        assert(mipmapLevel == 0 && "not implemented for higher levels");
        GLOW_RUNTIME_ASSERT(texture->mWidth == width, "Texture is storage immutable and a wrong width was specified", return );
        GLOW_RUNTIME_ASSERT(texture->mHeight == height, "Texture is storage immutable and a wrong height was specified", return );
        GLOW_RUNTIME_ASSERT(texture->mLayers == layers, "Texture is storage immutable and a wrong layers was specified", return );
        GLOW_RUNTIME_ASSERT(texture->mInternalFormat == internalFormat,
                            "Texture is storage immutable and a wrong internal format was specified", return );
    }

    if (mipmapLevel == 0)
    {
    texture->mWidth = width;
    texture->mHeight = height;
    texture->mLayers = layers;
        texture->mMipmapsGenerated = false;
        texture->mInternalFormat = internalFormat;
    }

    // assure proper pixel store parameter
    scoped::unpackSwapBytes   _p0(false);
    scoped::unpackLsbFirst    _p1(false);
    scoped::unpackRowLength   _p2(0);
    scoped::unpackImageHeight _p3(0);
    scoped::unpackSkipRows    _p4(0);
    scoped::unpackSkipPixels  _p5(0);
    scoped::unpackSkipImages  _p6(0);
    scoped::unpackAlignment   _p7(1); // tight

    if (texture->isStorageImmutable())
        glTexSubImage3D(texture->mTarget, mipmapLevel, 0, 0, 0, width, height, layers, format, type, data);
    else
        glTexImage3D(texture->mTarget, mipmapLevel, texture->mInternalFormat, width, height, layers, 0, format, type, data);
}

void Texture2DArray::BoundTexture2DArray::setSubData(size_t x, size_t y, size_t l, size_t width, size_t height, size_t layers, GLenum format, GLenum type, const GLvoid *data, int mipmapLevel)
{
    if (!isCurrent())
        return;
    checkValidGLOW();

    // assure proper pixel store parameter
    scoped::unpackSwapBytes   _p0(false);
    scoped::unpackLsbFirst    _p1(false);
    scoped::unpackRowLength   _p2(0);
    scoped::unpackImageHeight _p3(0);
    scoped::unpackSkipRows    _p4(0);
    scoped::unpackSkipPixels  _p5(0);
    scoped::unpackSkipImages  _p6(0);
    scoped::unpackAlignment   _p7(1); // tight

    glTexSubImage3D(texture->mTarget, mipmapLevel, x, y, l, width, height, layers, format, type, data);
 }

void Texture2DArray::BoundTexture2DArray::setData(GLenum internalFormat, const SharedTextureData &data)
{
    if (!isCurrent())
        return;

    texture->mInternalFormat = internalFormat; // format first, then resize
    resize(data->getWidth(), data->getHeight(), data->getDepth());

    // set all level 0 surfaces
    for (auto const &surf : data->getSurfaces())
        if (surf->getMipmapLevel() == 0)
            setSubData(surf->getOffsetX(), surf->getOffsetY(), surf->getOffsetZ(),
                       surf->getWidth(), surf->getHeight(), surf->getDepth(),
                       surf->getFormat(), surf->getType(),
                       surf->getData().data(), surf->getMipmapLevel());

    // set parameters
    if (data->getAnisotropicFiltering() >= 1.f)
        setAnisotropicFiltering(data->getAnisotropicFiltering());
    if (data->getMinFilter() != GL_INVALID_ENUM)
        setMinFilter(data->getMinFilter());
    if (data->getMagFilter() != GL_INVALID_ENUM)
        setMagFilter(data->getMagFilter());
    if (data->getWrapS() != GL_INVALID_ENUM)
        setWrapS(data->getWrapS());
    if (data->getWrapT() != GL_INVALID_ENUM)
        setWrapT(data->getWrapT());

    // generate mipmaps
    if (texture->hasMipmapsEnabled())
        generateMipmaps();

    // set all level 1+ surfaces
    for (auto const &surf : data->getSurfaces())
        if (surf->getMipmapLevel() > 0)
            setSubData(surf->getOffsetX(), surf->getOffsetY(), surf->getOffsetZ(),
                       surf->getWidth(), surf->getHeight(), surf->getDepth(),
                       surf->getFormat(), surf->getType(),
                       surf->getData().data(), surf->getMipmapLevel());
}

void Texture2DArray::clear(GLenum format, GLenum type, const GLvoid* data, int mipmapLevel)
{
    checkValidGLOW();
#if GLOW_OPENGL_VERSION >= 44
    glClearTexImage(mObjectName, mipmapLevel, format, type, data);
#else
    error() << "Texture2DArray::clear is only supported for OpenGL 4.4+";9
#endif
}

std::vector<char> Texture2DArray::BoundTexture2DArray::getData(GLenum format, GLenum type, int mipmapLevel)
{
    if (!isCurrent())
        return {};
    checkValidGLOW();

    auto target = texture->mTarget;

    size_t dataSize = channelsOfFormat(format) * sizeOfTypeInBytes(type);

    GLint sWidth;
    glGetTexLevelParameteriv(target, mipmapLevel, GL_TEXTURE_WIDTH, &sWidth);
    dataSize *= sWidth;

    GLint sHeight;
    glGetTexLevelParameteriv(target, mipmapLevel, GL_TEXTURE_HEIGHT, &sHeight);
    dataSize *= sHeight;

    GLint sDepth;
    glGetTexLevelParameteriv(target, mipmapLevel, GL_TEXTURE_DEPTH, &sDepth);
    dataSize *= sDepth;

    std::vector<char> data(dataSize);
    getData(format, type, data.size(), data.data(), mipmapLevel);
    return data;
}

void Texture2DArray::BoundTexture2DArray::getData(GLenum format, GLenum type, size_t bufferSize, void *buffer, int mipmapLevel)
{
    if (!isCurrent())
        return;
    checkValidGLOW();

    // assure proper pixel store parameter
    scoped::packSwapBytes   _p0(false);
    scoped::packLsbFirst    _p1(false);
    scoped::packRowLength   _p2(0);
    scoped::packImageHeight _p3(0);
    scoped::packSkipRows    _p4(0);
    scoped::packSkipPixels  _p5(0);
    scoped::packSkipImages  _p6(0);
    scoped::packAlignment   _p7(1); // tight

    (void)bufferSize; // TODO: check me!
    glGetTexImage(texture->mTarget, mipmapLevel, format, type, buffer);
}

SharedTextureData Texture2DArray::BoundTexture2DArray::getTextureData(int maxMipLevel)
{
    if (!isCurrent())
        return nullptr;
    checkValidGLOW();

    // assure proper pixel store parameter
    scoped::packSwapBytes   _p0(false);
    scoped::packLsbFirst    _p1(false);
    scoped::packRowLength   _p2(0);
    scoped::packImageHeight _p3(0);
    scoped::packSkipRows    _p4(0);
    scoped::packSkipPixels  _p5(0);
    scoped::packSkipImages  _p6(0);
    scoped::packAlignment   _p7(1); // tight

    auto tex = std::make_shared<TextureData>();

    // format
    GLenum format = GL_RGBA;
    GLenum type = GL_UNSIGNED_BYTE; // TODO: 16bit, 32bit types
    size_t bytesPerTexel = 4;
    auto isIntegerFormat = isIntegerInternalFormat(texture->mInternalFormat);
    switch (channelsOfInternalFormat(texture->mInternalFormat))
    {
    case 1:
        format = isIntegerFormat ? GL_RED_INTEGER : GL_RED;
        bytesPerTexel = 1;
        break;
    case 2:
        format = isIntegerFormat ? GL_RG_INTEGER : GL_RG;
        bytesPerTexel = 2;
        break;
    case 3:
        format = isIntegerFormat ? GL_RGB_INTEGER : GL_RGB;
        bytesPerTexel = 3;
        break;
    case 4:
        format = isIntegerFormat ? GL_RGBA_INTEGER : GL_RGBA;
        bytesPerTexel = 4;
        break;
    }

    // tex parameters
    // TODO

    // surfaces
    auto target = texture->mTarget;

    for (auto lvl = 0; lvl <= maxMipLevel; ++lvl)
    {
        GLint w;
        GLint h;
        GLint d;
        GLenum internalFormat;
        glGetTexLevelParameteriv(target, lvl, GL_TEXTURE_WIDTH, &w);
        glGetTexLevelParameteriv(target, lvl, GL_TEXTURE_HEIGHT, &h);
        glGetTexLevelParameteriv(target, lvl, GL_TEXTURE_DEPTH, &d);
        glGetTexLevelParameteriv(target, lvl, GL_TEXTURE_INTERNAL_FORMAT, (GLint *)&internalFormat);


        if (w * h * d == 0)
            break; // no mipmaps any more

        if (lvl == 0)
        {
            tex->setWidth(w);
            tex->setHeight(h);
            tex->setDepth(d);
            tex->setPreferredInternalFormat(internalFormat);
        }

        auto surface = std::make_shared<SurfaceData>();
        surface->setWidth(w);
        surface->setHeight(h);
        surface->setDepth(d);

        surface->setMipmapLevel(lvl);

        surface->setFormat(format);
        surface->setType(type);

        std::vector<char> data;
        data.resize(bytesPerTexel * w * h * d);
        glGetTexImage(target, lvl, surface->getFormat(), surface->getType(), data.data());
        surface->setData(data);

        tex->addSurface(surface);
    }

    return tex;
}

void Texture2DArray::BoundTexture2DArray::writeToFile(const std::string &filename)
{
    getTextureData()->saveToFile(filename);
}


Texture2DArray::BoundTexture2DArray::BoundTexture2DArray (Texture2DArray *texture) : texture(texture)
{
    checkValidGLOW();
    glGetIntegerv(texture->mBindingTarget, &previousTexture);
    glActiveTexture(GL_TEXTURE0 + limits::maxCombinedTextureImageUnits - 1);
    glBindTexture(texture->mTarget, texture->mObjectName);

    previousTexturePtr = sCurrentTexture;
    sCurrentTexture = this;
}

Texture2DArray::BoundTexture2DArray::BoundTexture2DArray (Texture2DArray::BoundTexture2DArray &&rhs)
  : texture(rhs.texture), previousTexture(rhs.previousTexture), previousTexturePtr(rhs.previousTexturePtr)
{
    // invalidate rhs
    rhs.previousTexture = -1;
}

Texture2DArray::BoundTexture2DArray::~BoundTexture2DArray ()
{
    if (previousTexture != -1) // if valid
    {
        checkValidGLOW();
        glActiveTexture(GL_TEXTURE0 + limits::maxCombinedTextureImageUnits - 1);
        glBindTexture(texture->mTarget, previousTexture);
        sCurrentTexture = previousTexturePtr;
    }
}
