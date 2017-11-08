// This file is auto-generated and should not be modified directly.
#pragma once

#include "Texture.hh"

#include "glow/common/gltypeinfo.hh"
#include "glow/common/warn_unused.hh"
#include "glow/common/log.hh"

#include "glow/data/ColorSpace.hh"

#include <vector>

namespace glow
{
GLOW_SHARED(class, TextureCubeMapArray);
GLOW_SHARED(class, TextureData);

/// Defines an array of CubeMap textures in OpenGL
class TextureCubeMapArray final : public Texture
{
public:
    struct BoundTextureCubeMapArray;

private:

    /// Minification filter
    GLenum mMinFilter = GL_NEAREST_MIPMAP_LINEAR;
    /// Magnification filter
    GLenum mMagFilter = GL_LINEAR;

    /// Wrapping in S
    GLenum mWrapS = GL_REPEAT;
    /// Wrapping in T
    GLenum mWrapT = GL_REPEAT;

    /// Level of anisotropic filtering (>= 1.f, which is isotropic)
    /// Max number of samples basically
    GLfloat mAnisotropicFiltering = 1.0f;

    /// Texture size: Width
    size_t mWidth = 0u;
    /// Texture size: Height
    size_t mHeight = 0u;
    /// Texture size: Layers
    size_t mLayers = 0u;

    /// True iff mipmaps are generated since last data upload
    bool mMipmapsGenerated = false;

    /// if true, this texture got immutable storage by glTexStorage2D
    bool mStorageImmutable = false;

public: // getter
    /// Gets the currently bound texture (nullptr if none)
    static BoundTextureCubeMapArray* getCurrentTexture();

    GLenum getMinFilter() const { return mMinFilter; }
    GLenum getMagFilter() const { return mMagFilter; }
    GLenum getWrapS() const { return mWrapS; }
    GLenum getWrapT() const { return mWrapT; }
    size_t getWidth() const { return mWidth; }
    size_t getHeight() const { return mHeight; }
    size_t getLayers() const { return mLayers; }
    glm::uvec3 getDimensions() const override { return { mWidth, mHeight, mLayers }; }

    bool isStorageImmutable() const override { return mStorageImmutable; }

    /// returns true iff mipmaps are used (based on min filter)
    bool hasMipmapsEnabled() const;
    /// returns true iff mipmaps were generated via bind().generateMipmaps() (and are still valid)
    bool areMipmapsGenerated() const override { return mMipmapsGenerated; }
    /// Manually sets the internal flag if mipmaps were generated
    /// CAUTION: this should only be used if you modified the mipmap status manually (e.g. via glGenerateMipmaps)
    void setMipmapsGenerated(bool areGenerated) override { mMipmapsGenerated = areGenerated; }

public:
    /// RAII-object that defines a "bind"-scope for an array of CubeMap textures
    /// All functions that operate on the currently bound tex are accessed here
    struct BoundTextureCubeMapArray
    {
        GLOW_RAII_CLASS(BoundTextureCubeMapArray);

        /// Backreference to the texture
        TextureCubeMapArray* const texture;

        /// Makes the storage of this texture immutable
        /// It is an error to call this more than once
        /// It is an error to upload data with a different internal format at a later point
        /// It is an error to resize after storage was made immutable (unless it's the same size)
        /// Invalidates previously uploaded data
        /// If mipmapLevels is <= 0, log2(max(width, height, layers)) + 1 is used
        void makeStorageImmutable(size_t width, size_t height, size_t layers, GLenum internalFormat, int mipmapLevels = 0);

        /// Sets minification filter (GL_NEAREST, GL_LINEAR, GL_NEAREST_MIPMAP_NEAREST, ..., GL_LINEAR_MIPMAP_LINEAR)
        void setMinFilter(GLenum filter);
        /// Sets magnification filter (GL_NEAREST, GL_LINEAR)
        void setMagFilter(GLenum filter);
        /// Sets mag and min filter
        void setFilter(GLenum magFilter, GLenum minFilter);

        /// Sets the number of anisotropic samples (>= 1)
        void setAnisotropicFiltering(GLfloat samples);

        /// Sets texture wrapping in S
        void setWrapS(GLenum wrap);
        /// Sets texture wrapping in T
        void setWrapT(GLenum wrap);
        /// Sets texture wrapping in all directions
        void setWrap(GLenum wrapS, GLenum wrapT);

        /// Generates mipmaps for this texture
        void generateMipmaps();

        /// Resizes the texture
        /// invalidates the data
        void resize(size_t width, size_t height, size_t layers);

        /// Generic data uploads
        /// Changes internal format, width, height, layers, and data
        void setData(GLenum internalFormat, GLenum target, size_t width, size_t height, size_t layers, GLenum format, GLenum type, const GLvoid* data, int mipmapLevel = 0);
        /// Data upload via glm or c++ type (see gltypeinfo)
        template <typename DataT>
        void setData(GLenum internalFormat, GLenum target, size_t width, size_t height, size_t layers, std::vector<DataT> const& data, int mipmapLevel = 0)
        {
            if (data.size() != width * height * layers)
            {
                error() << "Texture size is " << width << " x " << height << " x " << layers << " = " << width * height * layers << " but " << data.size()
                        << " pixels are provided.";
                return;
            }
            setData(internalFormat, target, width, height, layers, glTypeOf<DataT>::format, glTypeOf<DataT>::type, data.data(), mipmapLevel);
        }
        /// Same as above
        /// Usage:
        ///   glm::vec3 texData[] = { ... }
        ///   setData(iFormat, width, height, layers, texData);
        template <typename DataT, std::size_t N>
        void setData(GLenum internalFormat, GLenum target, size_t width, size_t height, size_t layers, const DataT(&data)[N], int mipmapLevel = 0)
        {
            if (N != width * height * layers)
            {
                error() << "Texture size is " << width << " x " << height << " x " << layers << " = " << width * height * layers << " but " << N
                        << " pixels are provided.";
                return;
            }
            setData(internalFormat, target, width, height, layers, glTypeOf<DataT>::format, glTypeOf<DataT>::type, data, mipmapLevel);
        }
        /// Same as above
        /// Usage:
        ///   glm::vec3 texData[][] = { ... }
        ///   // it's [layers][height][width]
        ///   setData(iFormat, texData);
        template <typename DataT, size_t width, size_t height, size_t layers>
        void setData(GLenum internalFormat, GLenum target, const DataT(&data)[layers][height][width] , int mipmapLevel = 0)
        {
            setData(internalFormat, target, width, height, layers, glTypeOf<DataT>::format, glTypeOf<DataT>::type, data, mipmapLevel);
        }

        /// Generic partial data uploads
        /// Only changes data. Offset and size must be inside original bounds.
        void setSubData(GLenum target, size_t x, size_t y, size_t l, size_t width, size_t height, size_t layers, GLenum format, GLenum type, const GLvoid* data, int mipmapLevel = 0);
        /// Partial data upload via glm or c++ type (see gltypeinfo)
        template <typename DataT>
        void setSubData(GLenum target, size_t x, size_t y, size_t l, size_t width, size_t height, size_t layers, std::vector<DataT> const& data, int mipmapLevel = 0)
        {
            if (data.size() != width * height * layers)
            {
                error() << "Texture size is " << width << " x " << height << " x " << layers << " = " << width * height * layers << " but " << data.size()
                        << " pixels are provided.";
                return;
            }
            setSubData(target, x, y, l, width, height, layers, glTypeOf<DataT>::format, glTypeOf<DataT>::type, data.data(), mipmapLevel);
        }
        /// Same as above
        /// Usage:
        ///   glm::vec3 texData[] = { ... }
        ///   setSubData(x, y, l, width, height, layers, texData);
        template <typename DataT, std::size_t N>
        void setSubData(GLenum target, size_t x, size_t y, size_t l, size_t width, size_t height, size_t layers, const DataT(&data)[N], int mipmapLevel = 0)
        {
            if (N != width * height * layers)
            {
                error() << "Texture size is " << width << " x " << height << " x " << layers << " = " << width * height * layers << " but " << N
                        << " pixels are provided.";
                return;
            }
            setSubData(target, x, y, l, width, height, layers, glTypeOf<DataT>::format, glTypeOf<DataT>::type, data, mipmapLevel);
        }
        /// Same as above
        /// Usage:
        ///   glm::vec3 texData[][] = { ... }
        ///   // it's [layers][height][width]
        ///   setSubData(x, y, l, texData);
        template <typename DataT, size_t width, size_t height, size_t layers>
        void setSubData(GLenum target, size_t x, size_t y, size_t l, const DataT(&data)[layers][height][width] , int mipmapLevel = 0)
        {
            setSubData(target, x, y, l, width, height, layers, glTypeOf<DataT>::format, glTypeOf<DataT>::type, data, mipmapLevel);
        }

        /// Sets texture data from surface data
        /// May set multiple levels at once
        /// May modify texture parameter
        void setData(GLenum internalFormat, SharedTextureData const& data);

        /// Generic data download
        std::vector<char> getData(GLenum format, GLenum type, int mipmapLevel = 0);
        /// Generic data download
        void getData(GLenum format, GLenum type, size_t bufferSize, void* buffer, int mipmapLevel = 0);
        /// Data download via glm or c++ type (see gltypeinfo)
        template <typename DataT>
        std::vector<DataT> getData(int mipmapLevel = 0)
        {
            std::vector<DataT> data;
            data.resize(texture->mWidth * texture->mHeight * texture->mLayers);
            getData(glTypeOf<DataT>::format, glTypeOf<DataT>::type, data.size() * sizeof(DataT), data.data(), mipmapLevel);
            return std::move(data);
        }

        /* TODO: OpenGL <4.5 does not support subimage retrieval (in 4.5, https://www.opengl.org/sdk/docs/man/html/glGetTextureSubImage.xhtml can be used)
        /// Generic partial data download
        std::vector<char> getSubData(GLenum format, GLenum type, size_t x, size_t y, size_t l, size_t width, size_t height, size_t layers, int mipmapLevel = 0);
        /// Generic partial data download
        void getSubData(GLenum format, GLenum type, size_t x, size_t y, size_t l, size_t width, size_t height, size_t layers, size_t bufferSize, void* buffer, int mipmapLevel = 0);
        /// Partial data download via glm or c++ type (see gltypeinfo)
        template <typename DataT>
        std::vector<DataT> getSubData(size_t x, size_t y, size_t l, size_t width, size_t height, size_t layers, int mipmapLevel = 0)
        {
            std::vector<DataT> data;
            data.resize(width * height * layers);
            getSubData(glTypeOf<DataT>::format, glTypeOf<DataT>::type, x, y, l, width, height, layers, data.size() * sizeof(DataT), data.data(), mipmapLevel);
            return std::move(data);
        }
        */

        /// Extracts all stored surface data up to a given max mipmap level (inclusive)
        /// This is useful for saving the texture to a file
        SharedTextureData getTextureData(int maxMipLevel = 1000);
        /// Same as getTextureData()->writeToFile(filename)
        void writeToFile(std::string const& filename);

    private:
        GLint previousTexture;              ///< previously bound tex
        BoundTextureCubeMapArray* previousTexturePtr; ///< previously bound tex
        BoundTextureCubeMapArray (TextureCubeMapArray* buffer);
        friend class TextureCubeMapArray;

        /// returns true iff it's safe to use this bound class
        /// otherwise, runtime error
        bool isCurrent() const;

    public:
        BoundTextureCubeMapArray (BoundTextureCubeMapArray &&); // allow move
        ~BoundTextureCubeMapArray ();
    };
public:

    /// Fills the specific mipmap level (default 0) with the given data
    /// Requires OpenGL 4.4 (for now) and will throw a run-time error otherwise
    void clear(GLenum format, GLenum type, const GLvoid* data, int mipmapLevel = 0);
    /// Clear via glm or c++ type (see gltypeinfo)
    /// CAREFUL: pointers do not work!
    template <typename DataT>
    void clear(DataT&& data, int mipmapLevel = 0)
    {
        clear(glTypeOf<DataT>::format, glTypeOf<DataT>::type, (const GLvoid*)&data, mipmapLevel);
    }

public:
    TextureCubeMapArray (GLenum internalFormat = GL_RGBA);

    /// Binds this texture.
    /// Unbinding is done when the returned object runs out of scope.
    GLOW_WARN_UNUSED BoundTextureCubeMapArray bind() { return {this}; }
public: // static construction
    /// Creates an array of CubeMap textures with given width and height
    static SharedTextureCubeMapArray create(size_t width = 1, size_t height = 1, size_t layers = 1, GLenum internalFormat = GL_RGBA);
    /// Creates an array of CubeMap textures with given width and height which is storage immutable
    /// If mipmapLevels is <= 0, log2(max(width, height)) + 1 is used
    static SharedTextureCubeMapArray createStorageImmutable(size_t width, size_t height, size_t layers, GLenum internalFormat, int mipmapLevels = 0);

    /// Creates an array of CubeMap textures from file
    /// See TextureData::createFromFile for format documentation
    /// Uses preferred internal format
    static SharedTextureCubeMapArray createFromFile(std::string const& filename, ColorSpace colorSpace = ColorSpace::AutoDetect);
    /// same as createFromFile but with custom internal format
    static SharedTextureCubeMapArray createFromFile(std::string const& filename, GLenum internalFormat, ColorSpace colorSpace = ColorSpace::AutoDetect);

    /// Creates an array of CubeMap textures from given data
    /// Uses preferred internal format
    static SharedTextureCubeMapArray createFromData(SharedTextureData const& data);
    /// same as createFromData but with custom internal format
    static SharedTextureCubeMapArray createFromData(SharedTextureData const& data, GLenum internalFormat);

    friend class Framebuffer;
};
}
