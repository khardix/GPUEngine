/** @file
 * @author Jan Staněk --- <xstane32@stud.fit.vutbr.cz>
 * @brief Scene graph manipulation necessary for OpenGL rendering.
 */

#pragma once

#include <memory>

#include <GL/gl.h>
#undef GL_GLEXT_VERSION  // Use the GPUEngine one

#include <geGL/VertexArray.h>
#include <geSG/AttributeDescriptor.h>
#include <geSG/Mesh.h>

/** @brief Loose grouping of functions and utilities for extraction OpenGL data
 * from generic scene graph.
 */
namespace glsg {
/// @brief Translate generic enumeration value to OpenGL equivalent.
GLenum translate(ge::sg::AttributeDescriptor::DataType type_id) noexcept;

/// @brief Create OpenGL "object" from equivalent ge::sg structure.
std::unique_ptr<ge::gl::VertexArray> convert(const ge::sg::Mesh &mesh);
}  // namespace glsg

// {{{ Inline and template members
/**
 * @param[in] type_id Generic attribute type identification.
 * @returns Equivalent OpenGL enumeration value.
 */
inline GLenum glsg::translate(
    ge::sg::AttributeDescriptor::DataType type_id) noexcept
{
    using DT = ge::sg::AttributeDescriptor::DataType;

    switch (type_id) {
        default:
        case DT::UNKNOWN:
            return GL_BYTE;

        case DT::BYTE:
            return GL_BYTE;
        case DT::UNSIGNED_BYTE:
            return GL_UNSIGNED_BYTE;
        case DT::SHORT:
            return GL_SHORT;
        case DT::UNSIGNED_SHORT:
            return GL_UNSIGNED_SHORT;
        case DT::INT:
            return GL_INT;
        case DT::UNSIGNED_INT:
            return GL_UNSIGNED_INT;
        case DT::FLOAT:
            return GL_FLOAT;
        case DT::DOUBLE:
            return GL_DOUBLE;
    }
}
// }}} Inline and template members
