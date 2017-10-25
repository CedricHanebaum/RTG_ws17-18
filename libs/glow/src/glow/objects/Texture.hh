#pragma once

#include "glow/common/shared.hh"
#include "glow/common/non_copyable.hh"

#include "glow/gl.hh"

#include <glm/vec3.hpp>

namespace glow
{
GLOW_SHARED(class, Texture);

/**
 * @brief Base class for textures
 *
 * Specializations exist for:
 * GL_TEXTURE_1D,
 * GL_TEXTURE_2D,
 * GL_TEXTURE_3D,
 * GL_TEXTURE_1D_ARRAY,
 * GL_TEXTURE_2D_ARRAY,
 * GL_TEXTURE_RECTANGLE,
 * GL_TEXTURE_CUBE_MAP,
 * GL_TEXTURE_CUBE_MAP_ARRAY,
 * GL_TEXTURE_BUFFER,
 * GL_TEXTURE_2D_MULTISAMPLE and
 * GL_TEXTURE_2D_MULTISAMPLE_ARRAY
 */
class Texture
{
    GLOW_NON_COPYABLE(Texture);

protected:
    /// OGL id
    GLuint mObjectName;

    /// Texture target
    GLenum mTarget;
    /// Binding enum
    GLenum mBindingTarget;

    /// Texture internal format
    GLenum mInternalFormat = GL_RGBA;

public: // getter
    GLuint getObjectName() const { return mObjectName; }
    GLenum getTarget() const { return mTarget; }
    GLenum getBindingTarget() const { return mBindingTarget; }
    GLenum getInternalFormat() const { return mInternalFormat; }
    virtual bool isStorageImmutable() const { return false; }

    /// returns the size of this texture. Contains 1 for non-used dimensions.
    virtual glm::uvec3 getDimensions() const = 0;
protected:
    Texture(GLenum target, GLenum bindingTarget, GLenum internalFormat);
    virtual ~Texture();
};
}
