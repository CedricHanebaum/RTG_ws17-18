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
GLOW_SHARED(class, Texture2DMultisample);
GLOW_SHARED(class, TextureData);

/// Defines a 2D multisampled texture in OpenGL
class Texture2DMultisample final : public Texture
{
public:
    struct BoundTexture2DMultisample;

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



public: // getter
    /// Gets the currently bound texture (nullptr if none)
    static BoundTexture2DMultisample* getCurrentTexture();

    GLenum getMinFilter() const { return mMinFilter; }
    GLenum getMagFilter() const { return mMagFilter; }
    GLenum getWrapS() const { return mWrapS; }
    GLenum getWrapT() const { return mWrapT; }
    size_t getWidth() const { return mWidth; }
    size_t getHeight() const { return mHeight; }
    glm::uvec3 getDimensions() const override { return { mWidth, mHeight, 1 }; }



public:
    /// RAII-object that defines a "bind"-scope for a 2D multisampled texture
    /// All functions that operate on the currently bound tex are accessed here
    struct BoundTexture2DMultisample
    {
        GLOW_RAII_CLASS(BoundTexture2DMultisample);

        /// Backreference to the texture
        Texture2DMultisample* const texture;


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


    private:
        GLint previousTexture;              ///< previously bound tex
        BoundTexture2DMultisample* previousTexturePtr; ///< previously bound tex
        BoundTexture2DMultisample (Texture2DMultisample* buffer);
        friend class Texture2DMultisample;

        /// returns true iff it's safe to use this bound class
        /// otherwise, runtime error
        bool isCurrent() const;

    public:
        BoundTexture2DMultisample (BoundTexture2DMultisample &&); // allow move
        ~BoundTexture2DMultisample ();
    };
public:

    /// Fills the specific mipmap level (default 0) with the given data
    /// Requires OpenGL 4.4 (for now) and will throw a run-time error otherwise
    void clear(GLenum format, GLenum type, const GLvoid* data);
    /// Clear via glm or c++ type (see gltypeinfo)
    /// CAREFUL: pointers do not work!
    template <typename DataT>
    void clear(DataT const& data)
    {
        clear(glTypeOf<DataT>::format, glTypeOf<DataT>::type, (const GLvoid*)&data);
    }

public:
    Texture2DMultisample (GLenum internalFormat = GL_RGBA);

    /// Binds this texture.
    /// Unbinding is done when the returned object runs out of scope.
    GLOW_WARN_UNUSED BoundTexture2DMultisample bind() { return {this}; }
public: // static construction
    /// Creates a 2D multisampled texture with given width and height
    static SharedTexture2DMultisample create(size_t width = 1, size_t height = 1, GLenum internalFormat = GL_RGBA);


    friend class Framebuffer;
};
}
