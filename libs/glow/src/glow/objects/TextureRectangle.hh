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
GLOW_SHARED(class, TextureRectangle);
GLOW_SHARED(class, TextureData);

/// Defines a rectangular texture in OpenGL
class TextureRectangle final : public Texture
{
public:
    struct BoundTextureRectangle;

private:

    /// Minification filter
    GLenum mMinFilter = GL_LINEAR;
    /// Magnification filter
    GLenum mMagFilter = GL_LINEAR;

    /// Wrapping in S
    GLenum mWrapS = GL_CLAMP_TO_EDGE;
    /// Wrapping in T
    GLenum mWrapT = GL_CLAMP_TO_EDGE;

    /// Level of anisotropic filtering (>= 1.f, which is isotropic)
    /// Max number of samples basically
    GLfloat mAnisotropicFiltering = 1.0f;

    /// Texture size: Width
    size_t mWidth = 0u;
    /// Texture size: Height
    size_t mHeight = 0u;


    /// if true, this texture got immutable storage by glTexStorage2D
    bool mStorageImmutable = false;

public: // getter
    /// Gets the currently bound texture (nullptr if none)
    static BoundTextureRectangle* getCurrentTexture();

    GLenum getMinFilter() const { return mMinFilter; }
    GLenum getMagFilter() const { return mMagFilter; }
    GLenum getWrapS() const { return mWrapS; }
    GLenum getWrapT() const { return mWrapT; }
    size_t getWidth() const { return mWidth; }
    size_t getHeight() const { return mHeight; }
    glm::uvec3 getDimensions() const override { return { mWidth, mHeight, 1 }; }

    bool isStorageImmutable() const override { return mStorageImmutable; }


public:
    /// RAII-object that defines a "bind"-scope for a rectangular texture
    /// All functions that operate on the currently bound tex are accessed here
    struct BoundTextureRectangle
    {
        GLOW_RAII_CLASS(BoundTextureRectangle);

        /// Backreference to the texture
        TextureRectangle* const texture;

        /// Makes the storage of this texture immutable
        /// It is an error to call this more than once
        /// It is an error to upload data with a different internal format at a later point
        /// It is an error to resize after storage was made immutable (unless it's the same size)
        /// Invalidates previously uploaded data
        void makeStorageImmutable(size_t width, size_t height, GLenum internalFormat);

        /// Sets minification filter (GL_NEAREST, GL_LINEAR)
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


        /// Resizes the texture
        /// invalidates the data
        void resize(size_t width, size_t height);

        /// Generic data uploads
        /// Changes internal format, width, height, and data
        void setData(GLenum internalFormat, size_t width, size_t height, GLenum format, GLenum type, const GLvoid* data);
        /// Data upload via glm or c++ type (see gltypeinfo)
        template <typename DataT>
        void setData(GLenum internalFormat, size_t width, size_t height, std::vector<DataT> const& data)
        {
            if (data.size() != width * height)
            {
                error() << "Texture size is " << width << " x " << height << " = " << width * height << " but " << data.size()
                        << " pixels are provided.";
                return;
            }
            setData(internalFormat, width, height, glTypeOf<DataT>::format, glTypeOf<DataT>::type, data.data());
        }
        /// Same as above
        /// Usage:
        ///   glm::vec3 texData[] = { ... }
        ///   setData(iFormat, width, height, texData);
        template <typename DataT, std::size_t N>
        void setData(GLenum internalFormat, size_t width, size_t height, const DataT(&data)[N])
        {
            if (N != width * height)
            {
                error() << "Texture size is " << width << " x " << height << " = " << width * height << " but " << N
                        << " pixels are provided.";
                return;
            }
            setData(internalFormat, width, height, glTypeOf<DataT>::format, glTypeOf<DataT>::type, data);
        }
        /// Same as above
        /// Usage:
        ///   glm::vec3 texData[][] = { ... }
        ///   // it's [height][width]
        ///   setData(iFormat, texData);
        template <typename DataT, size_t width, size_t height>
        void setData(GLenum internalFormat, const DataT(&data)[height][width] )
        {
            setData(internalFormat, width, height, glTypeOf<DataT>::format, glTypeOf<DataT>::type, data);
        }

        /// Generic partial data uploads
        /// Only changes data. Offset and size must be inside original bounds.
        void setSubData(size_t x, size_t y, size_t width, size_t height, GLenum format, GLenum type, const GLvoid* data);
        /// Partial data upload via glm or c++ type (see gltypeinfo)
        template <typename DataT>
        void setSubData(size_t x, size_t y, size_t width, size_t height, std::vector<DataT> const& data)
        {
            if (data.size() != width * height)
            {
                error() << "Texture size is " << width << " x " << height << " = " << width * height << " but " << data.size()
                        << " pixels are provided.";
                return;
            }
            setSubData(x, y, width, height, glTypeOf<DataT>::format, glTypeOf<DataT>::type, data.data());
        }
        /// Same as above
        /// Usage:
        ///   glm::vec3 texData[] = { ... }
        ///   setSubData(x, y, width, height, texData);
        template <typename DataT, std::size_t N>
        void setSubData(size_t x, size_t y, size_t width, size_t height, const DataT(&data)[N])
        {
            if (N != width * height)
            {
                error() << "Texture size is " << width << " x " << height << " = " << width * height << " but " << N
                        << " pixels are provided.";
                return;
            }
            setSubData(x, y, width, height, glTypeOf<DataT>::format, glTypeOf<DataT>::type, data);
        }
        /// Same as above
        /// Usage:
        ///   glm::vec3 texData[][] = { ... }
        ///   // it's [height][width]
        ///   setSubData(x, y, texData);
        template <typename DataT, size_t width, size_t height>
        void setSubData(size_t x, size_t y, const DataT(&data)[height][width] )
        {
            setSubData(x, y, width, height, glTypeOf<DataT>::format, glTypeOf<DataT>::type, data);
        }

        /// Sets texture data from surface data
        /// May set multiple levels at once
        /// May modify texture parameter
        void setData(GLenum internalFormat, SharedTextureData const& data);

        /// Generic data download
        std::vector<char> getData(GLenum format, GLenum type);
        /// Generic data download
        void getData(GLenum format, GLenum type, size_t bufferSize, void* buffer);
        /// Data download via glm or c++ type (see gltypeinfo)
        template <typename DataT>
        std::vector<DataT> getData(int mipmapLevel = 0)
        {
            std::vector<DataT> data;
            data.resize(texture->mWidth * texture->mHeight);
            getData(glTypeOf<DataT>::format, glTypeOf<DataT>::type, data.size() * sizeof(DataT), data.data());
            return std::move(data);
        }

        /* TODO: OpenGL <4.5 does not support subimage retrieval (in 4.5, https://www.opengl.org/sdk/docs/man/html/glGetTextureSubImage.xhtml can be used)
        /// Generic partial data download
        std::vector<char> getSubData(GLenum format, GLenum type, size_t x, size_t y, size_t width, size_t height);
        /// Generic partial data download
        void getSubData(GLenum format, GLenum type, size_t x, size_t y, size_t width, size_t height, size_t bufferSize, void* buffer);
        /// Partial data download via glm or c++ type (see gltypeinfo)
        template <typename DataT>
        std::vector<DataT> getSubData(size_t x, size_t y, size_t width, size_t height, int mipmapLevel = 0)
        {
            std::vector<DataT> data;
            data.resize(width * height);
            getSubData(glTypeOf<DataT>::format, glTypeOf<DataT>::type, x, y, width, height, data.size() * sizeof(DataT), data.data());
            return std::move(data);
        }
        */

        /// Extracts all stored surface data
        /// This is useful for saving the texture to a file
        SharedTextureData getTextureData();
        /// Same as getTextureData()->writeToFile(filename)
        void writeToFile(std::string const& filename);

    private:
        GLint previousTexture;              ///< previously bound tex
        BoundTextureRectangle* previousTexturePtr; ///< previously bound tex
        BoundTextureRectangle (TextureRectangle* buffer);
        friend class TextureRectangle;

        /// returns true iff it's safe to use this bound class
        /// otherwise, runtime error
        bool isCurrent() const;

    public:
        BoundTextureRectangle (BoundTextureRectangle &&); // allow move
        ~BoundTextureRectangle ();
    };
public:

    /// Fills the specific mipmap level (default 0) with the given data
    /// Requires OpenGL 4.4 (for now) and will throw a run-time error otherwise
    void clear(GLenum format, GLenum type, const GLvoid* data);
    /// Clear via glm or c++ type (see gltypeinfo)
    /// CAREFUL: pointers do not work!
    template <typename DataT>
    void clear(DataT&& data)
    {
        clear(glTypeOf<DataT>::format, glTypeOf<DataT>::type, (const GLvoid*)&data);
    }

public:
    TextureRectangle (GLenum internalFormat = GL_RGBA);

    /// Binds this texture.
    /// Unbinding is done when the returned object runs out of scope.
    GLOW_WARN_UNUSED BoundTextureRectangle bind() { return {this}; }
public: // static construction
    /// Creates a rectangular texture with given width and height
    static SharedTextureRectangle create(size_t width = 1, size_t height = 1, GLenum internalFormat = GL_RGBA);
    /// Creates a rectangular texture with given width and height which is storage immutable
    static SharedTextureRectangle createStorageImmutable(size_t width, size_t height, GLenum internalFormat);

    /// Creates a rectangular texture from file
    /// See TextureData::createFromFile for format documentation
    /// Uses preferred internal format
    static SharedTextureRectangle createFromFile(std::string const& filename, ColorSpace colorSpace = ColorSpace::AutoDetect);
    /// same as createFromFile but with custom internal format
    static SharedTextureRectangle createFromFile(std::string const& filename, GLenum internalFormat, ColorSpace colorSpace = ColorSpace::AutoDetect);

    /// Creates a rectangular texture from given data
    /// Uses preferred internal format
    static SharedTextureRectangle createFromData(SharedTextureData const& data);
    /// same as createFromData but with custom internal format
    static SharedTextureRectangle createFromData(SharedTextureData const& data, GLenum internalFormat);

    friend class Framebuffer;
};
}
