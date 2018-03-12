/** @file
 * @author Jan Staněk --- <xstane32@stud.fit.vutbr.cz>
 * @brief Scene graph manipulation necessary for OpenGL rendering
 * (implementation).
 */

#include "scene.hh"

#include <geGL/Buffer.h>

/** Create VertexArray object from a single Mesh. All attributes are converted
 * as they are.
 * @param[in] mesh The source mesh.
 * @returns Unique pointer to the VertexArray, since the type has incomplete
 * default operation set definition, and copying is thus problematic.
 */
std::unique_ptr<ge::gl::VertexArray> glsg::convert(const ge::sg::Mesh &mesh)
{
    using Semantic = ge::sg::AttributeDescriptor::Semantic;

    auto result = std::make_unique<ge::gl::VertexArray>();

    for (const auto &descriptor : mesh.attributes) {
        auto buffer = std::make_unique<ge::gl::Buffer>(
            static_cast<const GLsizeiptr>(descriptor->size),
            static_cast<const GLvoid *>(descriptor->data.get()));

        if (descriptor->semantic == Semantic::indices) {
            result->addElementBuffer(std::move(buffer));
            continue;
        }

        result->addAttrib(
            std::move(buffer),
            static_cast<const GLuint>(0),
            static_cast<const GLint>(descriptor->numComponents),
            translate(descriptor->type),
            static_cast<const GLsizei>(descriptor->stride),
            static_cast<const GLintptr>(descriptor->offset));
    }

    return std::move(result);
}
