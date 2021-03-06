/** @file
 * @author Jan Staněk --- <xstane32@stud.fit.vutbr.cz>
 * @brief Utilities for necessary scene graph manipulation (implementation).
 */

#include <cassert>

#include <geGL/Buffer.h>

#include "scene_util.h"

/**
 * @param[in] type_id Generic attribute type identification.
 * @returns Equivalent OpenGL enumeration value.
 */
GLenum util::glsg::translate(
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

/** @overload
 * @param[in] primitive_id Generic primitive type identification.
 * @returns Equivalent OpenGL primitive identification.
 */
GLenum util::glsg::translate(ge::sg::Mesh::PrimitiveType primitive_id) noexcept
{
    using PT = ge::sg::Mesh::PrimitiveType;

    switch (primitive_id) {
        default:
        case PT::UNKNOWN:
            return GL_TRIANGLES;

        case PT::POINTS:
            return GL_POINTS;
        case PT::LINES:
            return GL_LINES;
        case PT::LINE_LOOP:
            return GL_LINE_LOOP;
        case PT::LINE_STRIP:
            return GL_LINE_STRIP;
        case PT::TRIANGLES:
            return GL_TRIANGLES;
        case PT::TRIANGLE_STRIP:
            return GL_TRIANGLE_STRIP;
        case PT::TRIANGLE_FAN:
            return GL_TRIANGLE_FAN;
        case PT::QUADS:
            return GL_QUADS;
        case PT::QUAD_STRIP:
            return GL_QUAD_STRIP;
        case PT::POLYGON:
            return GL_POLYGON;
        case PT::PATCH:
            return GL_PATCHES;
    }
}

/** Extract all vertices from a scene and store minimum and maximum along each
 * axis.
 * @param[in] scene The scene to process.
 * @return A (minimum, maximum) pair of points representing the corners of the
 * bounding box.
 */
std::pair<glm::vec3, glm::vec3> util::bounding_box(
    const std::shared_ptr<const ge::sg::Scene> &scene)
{
    constexpr auto position = ge::sg::AttributeDescriptor::Semantic::position;
    constexpr auto min_v = std::numeric_limits<float>::min();
    constexpr auto max_v = std::numeric_limits<float>::max();

    auto minimum = glm::vec3{max_v, max_v, max_v};
    auto maximum = glm::vec3{min_v, min_v, min_v};

    for (const auto &model : scene->models) {
        for (const auto &mesh : model->meshes) {
            const auto attr = mesh->getAttribute(position);
            assert((attr && attr->numComponents == 3));  // NOLINT

            const auto data = std::static_pointer_cast<const float>(attr->data);
            const auto begin = data.get();
            const auto end = begin + (attr->size / sizeof(float));

            for (auto p = begin; p != end; p += attr->numComponents) {
                minimum.x = std::min(minimum.x, p[0]);  // NOLINT
                minimum.y = std::min(minimum.y, p[1]);  // NOLINT
                minimum.z = std::min(minimum.z, p[2]);  // NOLINT

                maximum.x = std::max(maximum.x, p[0]);  // NOLINT
                maximum.y = std::max(maximum.y, p[1]);  // NOLINT
                maximum.z = std::max(maximum.z, p[2]);  // NOLINT
            }
        }
    }

    return {minimum, maximum};
}

/** Initialize internal stacks to iterate over a sub-graph.
 * @param[in] root The root node of the DAG.
 * @param[in] initial_transform Optional initial transformation.
 */
util::SceneWalker::iterator::iterator(
    std::shared_ptr<Node> root, glm::mat4 initial_transform)
    : m_root_container({root})
{
    auto root_iter
        = Level{std::begin(m_root_container), std::end(m_root_container)};

    m_history.push(std::move(root_iter));
    m_transform.push(initial_transform);
}

/// @warning Undefined if the iterator is empty.
void util::SceneWalker::iterator::push_level()
{
    // history
    auto &current = *(m_history.top().current);
    m_history.push(
        Level{std::begin(current->children), std::end(current->children)});

    // transformation
    auto &child = *(m_history.top().current);
    m_transform.push(m_transform.top() * child->data->getMatrix());
}

/// @warning Undefined if the iterator is empty.
void util::SceneWalker::iterator::next_sibling()
{
    auto &child = ++(m_history.top().current);
    if (!level_finished()) {
        m_transform.pop();
        m_transform.push(m_transform.top() * (*child)->data->getMatrix());
    }
}

/// @warning Undefined if the iterator is empty.
void util::SceneWalker::iterator::pop_level()
{
    m_transform.pop();
    m_history.pop();
}

/** Move to next in-order node in the scene.
 * @returns Reference to modified self.
 */
util::SceneWalker::iterator &util::SceneWalker::iterator::operator++()
{
    if (empty()) {
        return *this;
    }

    if (!level_finished()) {
        if (have_children()) {  // continue to first child
            push_level();
        }
        else {  // continue to next sibling
            next_sibling();
        }
    }

    // go up until empty or not at the end of level
    while (!empty() && level_finished()) {
        pop_level();
        if (empty()) {
            break;
        }
        next_sibling();
    }

    return *this;
}

/** Compare two iterators.
 * Empty (past-end) iterators always compare equal.
 * Otherwise, equal iterators should point to the same node and have the same
 * transformation.
 * @param[in] other The other iterator to compare.
 * @returns Comparison result.
 */
bool util::SceneWalker::iterator::operator==(const iterator &other) noexcept
{
    if (empty() && other.empty()) {
        return true;
    }
    if (empty() != other.empty()) {
        return false;
    }

    auto same_ptr
        = (m_history.top().current->get()
           == other.m_history.top().current->get());
    auto same_transformation = (m_transform.top() == other.m_transform.top());

    return same_ptr && same_transformation;
}
