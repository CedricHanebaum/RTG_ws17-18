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
GLOW_SHARED(class, Texture1D);
GLOW_SHARED(class, TextureData);

/// Defines a 1D texture in OpenGL
class Texture1D final : public Texture
{
public:
    struct BoundTexture1D;

private:

    /// Minification filter
    GLenum mMinFilter = GL_NEAREST_MIPMAP_LINEAR;
    /// Magnification filter
    GLenum mMagFilter = GL_LINEAR;

    /// Wrapping in S
    GLenum mWrapS = GL_REPEAT;

    /// Level of anisotropic filtering (>= 1.f, which is isotropic)
    /// Max number of samples basically
    GLfloat mAnisotropicFiltering = 1.0f;

    /// Texture size: Width
    size_t mWidth = 0u;

    /// True iff mipmaps are generated since last data upload
    bool mMipmapsGenerated = false;

    /// if true, this texture got immutable storage by glTexStorage2D
    bool mStorageImmutable = false;

public: // getter
    /// Gets the currently bound texture (nullptr if none)
    static BoundTexture1D* getCurrentTexture();

    GLenum getMinFilter() const { return mMinFilter; }
    GLenum getMagFilter() const { return mMagFilter; }
    GLenum getWrapS() const { return mWrapS; }
    size_t getWidth() const { return mWidth; }
    glm::uvec3 getDimensions() const override { return { mWidth, 1, 1 }; }

    bool isStorageImmutable() const override { return mStorageImmutable; }

    /// returns true iff mipmaps are used (based on min filter)
    bool hasMipmapsEnabled() const;
    /// returns true iff mipmaps were generated via bind().generateMipmaps() (and are still valid)
    bool areMipmapsGenerated() const override { return mMipmapsGenerated; }
    /// Manually sets the internal flag if mipmaps were generated
    /// CAUTION: this should only be used if you modified the mipmap status manually (e.g. via glGenerateMipmaps)
    void setMipmapsGenerated(bool areGenerated) override { mMipmapsGenerated = areGenerated; }

public:
    /// RAII-object that defines a "bind"-scope for a 1D texture
    /// All functions that operate on the currently bound tex are accessed here
    struct BoundTexture1D
    {
        GLOW_RAII_CLASS(BoundTexture1D);

        /// Backreference to the texture
        Texture1D* const texture;

        /// Makes the storage of this texture immutable
        /// It is an error to call this more than once
        /// It is an error to upload data with a different internal format at a later point
        /// It is an error to resize after storage was made immutable (unless it's the same size)
        /// Invalidates previously uploaded data
        /// If mipmapLevels is <= 0, log2(max(width)) + 1 is used
        void makeStorageImmutable(size_t width, GLenum internalFormat, int mipmapLevels = 0);

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
        /// Sets texture wrapping in all directions
        void setWrap(GLenum wrapS);

        /// Generates mipmaps for this texture
        void generateMipmaps();

        /// Resizes the texture
        /// invalidates the data
        void resize(size_t width);

        /// Generic data uploads
        /// Changes internal format, width, and data
        void setData(GLenum internalFormat, size_t width, GLenum format, GLenum type, const GLvoid* data, int mipmapLevel = 0);
        /// Data upload via glm or c++ type (see gltypeinfo)
        template <typename DataT>
        void setData(GLenum internalFormat, size_t width, std::vector<DataT> const& data, int mipmapLevel = 0)
        {
            if (data.size() != width)
            {
                error() << "Texture size is " << width << " = " << width << " but " << data.size()
                        << " pixels are provided.";
                return;
            }
            setData(internalFormat, width, glTypeOf<DataT>::format, glTypeOf<DataT>::type, data.data(), mipmapLevel);
        }
        /// Same as above
        /// Usage:
        ///   glm::vec3 texData[] = { ... }
        ///   setData(iFormat, width, texData);
        template <typename DataT, std::size_t N>
        void setData(GLenum internalFormat, size_t width, const DataT(&data)[N], int mipmapLevel = 0)
        {
            if (N != width)
            {
                error() << "Texture size is " << width << " = " << width << " but " << N
                        << " pixels are provided.";
                return;
            }
            setData(internalFormat, width, glTypeOf<DataT>::format, glTypeOf<DataT>::type, data, mipmapLevel);
        }
        /// Same as above
        /// Usage:
        ///   glm::vec3 texData[][] = { ... }
        ///   // it's [width]
        ///   setData(iFormat, texData);
        template <typename DataT, size_t width>
        void setData(GLenum internalFormat, const DataT(&data)[width] , int mipmapLevel = 0)
        {
            setData(internalFormat, width, glTypeOf<DataT>::format, glTypeOf<DataT>::type, data, mipmapLevel);
        }

        /// Generic partial data uploads
        /// Only changes data. Offset and size must be inside original bounds.
        void setSubData(size_t x, size_t width, GLenum format, GLenum type, const GLvoid* data, int mipmapLevel = 0);
        /// Partial data upload via glm or c++ type (see gltypeinfo)
        template <typename DataT>
        void setSubData(size_t x, size_t width, std::vector<DataT> const& data, int mipmapLevel = 0)
        {
            if (data.size() != width)
            {
                error() << "Texture size is " << width << " = " << width << " but " << data.size()
                        << " pixels are provided.";
                return;
            }
            setSubData(x, width, glTypeOf<DataT>::format, glTypeOf<DataT>::type, data.data(), mipmapLevel);
        }
        /// Same as above
        /// Usage:
        ///   glm::vec3 texData[] = { ... }
        ///   setSubData(x, width, texData);
        template <typename DataT, std::size_t N>
        void setSubData(size_t x, size_t width, const DataT(&data)[N], int mipmapLevel = 0)
        {
            if (N != width)
            {
                error() << "Texture size is " << width << " = " << width << " but " << N
                        << " pixels are provided.";
                return;
            }
            setSubData(x, width, glTypeOf<DataT>::format, glTypeOf<DataT>::type, data, mipmapLevel);
        }
        /// Same as above
        /// Usage:
        ///   glm::vec3 texData[][] = { ... }
        ///   // it's [width]
        ///   setSubData(x, texData);
        template <typename DataT, size_t width>
        void setSubData(size_t x, const DataT(&data)[width] , int mipmapLevel = 0)
        {
            setSubData(x, width, glTypeOf<DataT>::format, glTypeOf<DataT>::type, data, mipmapLevel);
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
            data.resize(texture->mWidth);
            getData(glTypeOf<DataT>::format, glTypeOf<DataT>::type, data.size() * sizeof(DataT), data.data(), mipmapLevel);
            return std::move(data);
        }

        /* TODO: OpenGL <4.5 does not support subimage retrieval (in 4.5, https://www.opengl.org/sdk/docs/man/html/glGetTextureSubImage.xhtml can be used)
        /// Generic partial data download
        std::vector<char> getSubData(GLenum format, GLenum type, size_t x, size_t width, int mipmapLevel = 0);
        /// Generic partial data download
        void getSubData(GLenum format, GLenum type, size_t x, size_t width, size_t bufferSize, void* buffer, int mipmapLevel = 0);
        /// Partial data download via glm or c++ type (see gltypeinfo)
        template <typename DataT>
        std::vector<DataT> getSubData(size_t x, size_t width, int mipmapLevel = 0)
        {
            std::vector<DataT> data;
            data.resize(width);
            getSubData(glTypeOf<DataT>::format, glTypeOf<DataT>::type, x, width, data.size() * sizeof(DataT), data.data(), mipmapLevel);
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
        BoundTexture1D* previousTexturePtr; ///< previously bound tex
        BoundTexture1D (Texture1D* buffer);
        friend class Texture1D;

        /// returns true iff it's safe to use this bound class
        /// otherwise, runtime error
        bool isCurrent() const;

    public:
        BoundTexture1D (BoundTexture1D &&); // allow move
        ~BoundTexture1D ();
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
    Texture1D (GLenum internalFormat = GL_RGBA);

    /// Binds this texture.
    /// Unbinding is done when the returned object runs out of scope.
    GLOW_WARN_UNUSED BoundTexture1D bind() { return {this}; }
public: // static construction
    /// Creates a 1D texture with given width and height
    static SharedTexture1D create(size_t width = 1, GLenum internalFormat = GL_RGBA);
    /// Creates a 1D texture with given width and height which is storage immutable
    /// If mipmapLevels is <= 0, log2(max(width, height)) + 1 is used
    static SharedTexture1D createStorageImmutable(size_t width, GLenum internalFormat, int mipmapLevels = 0);

    /// Creates a 1D texture from file
    /// See TextureData::createFromFile for format documentation
    /// Uses preferred internal format
    static SharedTexture1D createFromFile(std::string const& filename, ColorSpace colorSpace = ColorSpace::AutoDetect);
    /// same as createFromFile but with custom internal format
    static SharedTexture1D createFromFile(std::string const& filename, GLenum internalFormat, ColorSpace colorSpace = ColorSpace::AutoDetect);

    /// Creates a 1D texture from given data
    /// Uses preferred internal format
    static SharedTexture1D createFromData(SharedTextureData const& data);
    /// same as createFromData but with custom internal format
    static SharedTexture1D createFromData(SharedTextureData const& data, GLenum internalFormat);

    friend class Framebuffer;
};
}
