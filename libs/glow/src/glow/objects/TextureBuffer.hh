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
GLOW_SHARED(class, TextureBuffer);
GLOW_SHARED(class, TextureData);

/// Defines a texture buffer in OpenGL
class TextureBuffer final : public Texture
{
public:
    struct BoundTextureBuffer;

private:


    /// Texture size: Size
    size_t mSize = 0u;



public: // getter
    /// Gets the currently bound texture (nullptr if none)
    static BoundTextureBuffer* getCurrentTexture();

    size_t getSize() const { return mSize; }
    glm::uvec3 getDimensions() const override { return { mSize, 1, 1 }; }



public:
    /// RAII-object that defines a "bind"-scope for a texture buffer
    /// All functions that operate on the currently bound tex are accessed here
    struct BoundTextureBuffer
    {
        GLOW_RAII_CLASS(BoundTextureBuffer);

        /// Backreference to the texture
        TextureBuffer* const texture;




        /// Resizes the texture
        /// invalidates the data
        void resize(size_t size);

        /// Generic data uploads
        /// Changes internal format, size, and data
        void setData(GLenum internalFormat, size_t size, GLenum format, GLenum type, const GLvoid* data);
        /// Data upload via glm or c++ type (see gltypeinfo)
        template <typename DataT>
        void setData(GLenum internalFormat, size_t size, std::vector<DataT> const& data)
        {
            if (data.size() != size)
            {
                error() << "Texture size is " << size << " = " << size << " but " << data.size()
                        << " pixels are provided.";
                return;
            }
            setData(internalFormat, size, glTypeOf<DataT>::format, glTypeOf<DataT>::type, data.data());
        }
        /// Same as above
        /// Usage:
        ///   glm::vec3 texData[] = { ... }
        ///   setData(iFormat, size, texData);
        template <typename DataT, std::size_t N>
        void setData(GLenum internalFormat, size_t size, const DataT(&data)[N])
        {
            if (N != size)
            {
                error() << "Texture size is " << size << " = " << size << " but " << N
                        << " pixels are provided.";
                return;
            }
            setData(internalFormat, size, glTypeOf<DataT>::format, glTypeOf<DataT>::type, data);
        }
        /// Same as above
        /// Usage:
        ///   glm::vec3 texData[][] = { ... }
        ///   // it's [size]
        ///   setData(iFormat, texData);
        template <typename DataT, size_t size>
        void setData(GLenum internalFormat, const DataT(&data)[size] )
        {
            setData(internalFormat, size, glTypeOf<DataT>::format, glTypeOf<DataT>::type, data);
        }

        /// Generic partial data uploads
        /// Only changes data. Offset and size must be inside original bounds.
        void setSubData(size_t offset, size_t size, GLenum format, GLenum type, const GLvoid* data);
        /// Partial data upload via glm or c++ type (see gltypeinfo)
        template <typename DataT>
        void setSubData(size_t offset, size_t size, std::vector<DataT> const& data)
        {
            if (data.size() != size)
            {
                error() << "Texture size is " << size << " = " << size << " but " << data.size()
                        << " pixels are provided.";
                return;
            }
            setSubData(offset, size, glTypeOf<DataT>::format, glTypeOf<DataT>::type, data.data());
        }
        /// Same as above
        /// Usage:
        ///   glm::vec3 texData[] = { ... }
        ///   setSubData(offset, size, texData);
        template <typename DataT, std::size_t N>
        void setSubData(size_t offset, size_t size, const DataT(&data)[N])
        {
            if (N != size)
            {
                error() << "Texture size is " << size << " = " << size << " but " << N
                        << " pixels are provided.";
                return;
            }
            setSubData(offset, size, glTypeOf<DataT>::format, glTypeOf<DataT>::type, data);
        }
        /// Same as above
        /// Usage:
        ///   glm::vec3 texData[][] = { ... }
        ///   // it's [size]
        ///   setSubData(offset, texData);
        template <typename DataT, size_t size>
        void setSubData(size_t offset, const DataT(&data)[size] )
        {
            setSubData(offset, size, glTypeOf<DataT>::format, glTypeOf<DataT>::type, data);
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
            data.resize(texture->mSize);
            getData(glTypeOf<DataT>::format, glTypeOf<DataT>::type, data.size() * sizeof(DataT), data.data());
            return std::move(data);
        }

        /* TODO: OpenGL <4.5 does not support subimage retrieval (in 4.5, https://www.opengl.org/sdk/docs/man/html/glGetTextureSubImage.xhtml can be used)
        /// Generic partial data download
        std::vector<char> getSubData(GLenum format, GLenum type, size_t offset, size_t size);
        /// Generic partial data download
        void getSubData(GLenum format, GLenum type, size_t offset, size_t size, size_t bufferSize, void* buffer);
        /// Partial data download via glm or c++ type (see gltypeinfo)
        template <typename DataT>
        std::vector<DataT> getSubData(size_t offset, size_t size, int mipmapLevel = 0)
        {
            std::vector<DataT> data;
            data.resize(size);
            getSubData(glTypeOf<DataT>::format, glTypeOf<DataT>::type, offset, size, data.size() * sizeof(DataT), data.data());
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
        BoundTextureBuffer* previousTexturePtr; ///< previously bound tex
        BoundTextureBuffer (TextureBuffer* buffer);
        friend class TextureBuffer;

        /// returns true iff it's safe to use this bound class
        /// otherwise, runtime error
        bool isCurrent() const;

    public:
        BoundTextureBuffer (BoundTextureBuffer &&); // allow move
        ~BoundTextureBuffer ();
    };
public:


public:
    TextureBuffer (GLenum internalFormat = GL_RGBA);

    /// Binds this texture.
    /// Unbinding is done when the returned object runs out of scope.
    GLOW_WARN_UNUSED BoundTextureBuffer bind() { return {this}; }
public: // static construction
    /// Creates a texture buffer with given width and height
    static SharedTextureBuffer create(size_t size = 1, GLenum internalFormat = GL_RGBA);

    /// Creates a texture buffer from file
    /// See TextureData::createFromFile for format documentation
    /// Uses preferred internal format
    static SharedTextureBuffer createFromFile(std::string const& filename, ColorSpace colorSpace = ColorSpace::AutoDetect);
    /// same as createFromFile but with custom internal format
    static SharedTextureBuffer createFromFile(std::string const& filename, GLenum internalFormat, ColorSpace colorSpace = ColorSpace::AutoDetect);

    /// Creates a texture buffer from given data
    /// Uses preferred internal format
    static SharedTextureBuffer createFromData(SharedTextureData const& data);
    /// same as createFromData but with custom internal format
    static SharedTextureBuffer createFromData(SharedTextureData const& data, GLenum internalFormat);

    friend class Framebuffer;
};
}
