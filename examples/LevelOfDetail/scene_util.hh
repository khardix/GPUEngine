/** @file
 * @author Jan Staněk --- <xstane32@stud.fit.vutbr.cz>
 * @brief Utilities for necessary scene graph manipulation.
 */

#pragma once

#include <iterator>
#include <memory>
#include <stack>

#include <GL/gl.h>
#undef GL_GLEXT_VERSION  // Use the GPUEngine one

#include <glm/mat4x4.hpp>

#include <geGL/VertexArray.h>
#include <geSG/AttributeDescriptor.h>
#include <geSG/MatrixTransform.h>
#include <geSG/Mesh.h>
#include <geSG/Node.h>
#include <geSG/Scene.h>
#include <ste/DAG.h>

namespace util {
/** @brief Loose grouping of functions and utilities for extraction OpenGL data
 * from generic scene graph.
 */
namespace glsg {
/// @brief Translate generic enumeration value to OpenGL equivalent.
GLenum translate(ge::sg::AttributeDescriptor::DataType type_id) noexcept;

/// @brief Create OpenGL "object" from equivalent ge::sg structure.
std::unique_ptr<ge::gl::VertexArray> convert(const ge::sg::Mesh &mesh);
}  // namespace glsg

/// @brief Thin wrapper for in-order iteration over a scene.
class SceneWalker {
public:
    class iterator;

    /// @brief Maintain only weak binding to the iterated scene.
    explicit SceneWalker(std::weak_ptr<ge::sg::Scene> scene) noexcept;

    /// @brief Start iteration at the root node of the scene.
    iterator begin();
    /// @brief Return empty, past-end scene iterator.
    iterator end();

private:
    std::weak_ptr<ge::sg::Scene> m_scene = {};
};

/// @brief Encapsulated logic for scene iteration, compatible with STL.
class SceneWalker::iterator {
public:
    // Internal type names
    using Node = ge::sg::MatrixTransformNode;
    using ChildContainer = Node::ChildContainer;
    using ChildIterator = Node::ChildIterator;
    using MeshContainer = decltype(ge::sg::MatrixTransform::meshes);
    // Iterator traits interface
    using value_type = std::tuple<MeshContainer &, glm::mat4>;
    using difference_type = void;  // undefined
    using pointer = value_type *;
    using reference = value_type;  // value_type is proxy, always pass by value
    using iterator_category = std::input_iterator_tag;

    /// @brief Construct empty, past-end iterator.
    explicit iterator() = default;
    /// @brief Construct iterator for a subgraph.
    explicit iterator(
        std::shared_ptr<Node> root,
        glm::mat4             initial_transform = glm::mat4{1.f});
    // Default operations
    iterator(const iterator &) = default;
    iterator(iterator &&) = default;
    iterator &operator=(const iterator &) = default;
    iterator &operator=(iterator &&) = default;

    /// @brief Dereference the iterator.
    reference operator*() const noexcept;
    /// @brief Advance the iterator to next node.
    iterator &operator++();
    iterator  operator++(int);
    /// @brief Compare two iterators.
    bool operator==(const iterator &other) noexcept;
    bool operator!=(const iterator &other) noexcept;

protected:
    /// @brief Indicate if the internal stacks can be manipulated.
    bool empty() const noexcept;
    /// @brief Indicate if the current level is finished.
    bool level_finished() noexcept;
    /// @brief Indicate if the current node have any children.
    bool have_children() noexcept;

    /// @brief Move to first child.
    void push_level();
    /// @brief Move to next sibling.
    void next_sibling();
    /// @brief Return to parent level.
    void pop_level();

private:
    struct Level {
        ChildIterator       current;
        const ChildIterator end;
    };

    ChildContainer        m_root_container = {};
    std::stack<Level>     m_history = {};
    std::stack<glm::mat4> m_transform = {};
};
}  // namespace util

// {{{ Inline and template members
/**
 * @param[in] scene The scene to iterate over.
 */
inline util::SceneWalker::SceneWalker(
    std::weak_ptr<ge::sg::Scene> scene) noexcept
    : m_scene(std::move(scene))
{
}

/// @returns Iterator over the whole sub-graph.
inline util::SceneWalker::iterator util::SceneWalker::begin()
{
    if (auto scene = m_scene.lock()) {
        return iterator(scene->rootNode);
    }
    else {
        return iterator();
    }
}
/// @returns Past-the-end scene iterator.
inline util::SceneWalker::iterator util::SceneWalker::end()
{
    return iterator();
}

/// @returns True if either of internal stacks are empty.
inline bool util::SceneWalker::iterator::empty() const noexcept
{
    return m_history.empty() || m_transform.empty();
}

/// @returns True if the current iterator is past-to-end.
/// @warning Undefined if the history stack is empty.
inline bool util::SceneWalker::iterator::level_finished() noexcept
{
    auto &level = m_history.top();
    return level.current == level.end;
}

/// @returns True if the current node have non-empty children container.
/// @warning Undefined if the iterator is empty.
inline bool util::SceneWalker::iterator::have_children() noexcept
{
    auto &current = *m_history.top().current;
    return !current->children.empty();
}

inline util::SceneWalker::iterator::reference util::SceneWalker::iterator::
                                              operator*() const noexcept
{
    auto &container = (*m_history.top().current)->data->meshes;
    auto &trans = m_transform.top();
    return std::forward_as_tuple(container, trans);
}

/** Move to next in-order node in the scene.
 * @return An iterator before the move.
 */
inline util::SceneWalker::iterator util::SceneWalker::iterator::operator++(int)
{
    auto old_state = *this;
    ++(*this);
    return std::move(old_state);
}

/** Compare two iterators.
 * @see operator==()
 * @param[in] other The other iterator to compare.
 * @returns Comparison result.
 */
inline bool util::SceneWalker::iterator::operator!=(
    const iterator &other) noexcept
{
    return !(*this == other);
}
// }}} Inline and template members
