#ifndef LAZY_SELECTION_H_DX3JHVCY
#define LAZY_SELECTION_H_DX3JHVCY
/** @file
 * @author Bc. Jan Staněk --- <xstane32@stud.fit.vutbr.cz>
 * @brief Lazy selection algorithm for mesh decimation.
 */

#include <algorithm>
#include <functional>
#include <iterator>
#include <queue>
#include <set>
#include <unordered_set>
#include <vector>

#include <nonstd/variant.hpp>

#include <geSG/Mesh.h>

#include "../graph/Mesh.h"

namespace lod {
namespace algorithm {
/** @brief Lazy selection decimation algorithm.
 * @tparam Tag The tag type that determine the kind of operation to perform.
 * @tparam Metric The metric to use for ordering the operations.
 * @tparam Operator The operator to use by the algorithm.
 */
template <
    typename Tag,
    template <typename> class Metric,
    template <typename> class Operator>
class LazySelection {
public:
    using operation_type = typename Metric<Tag>::result_type;
    using element_type = typename operation_type::element_pointer::type;
    using element_pointer = typename operation_type::element_pointer;

    using pointer_set_type = std::set<
        typename element_pointer::pointer_type,
        typename element_pointer::pointer_cmp>;
    using queue_type = std::priority_queue<
        operation_type,
        std::vector<operation_type>,
        std::greater<operation_type>>;

    using threshold_type = typename operation_type::cost_type;
    using size_type = typename queue_type::size_type;

    /// @brief Decimate all elements with cost below a certain threshold.
    graph::Mesh &operator()(graph::Mesh &mesh, threshold_type threshold) const;
    ge::sg::Mesh operator()(
        const ge::sg::Mesh &mesh, threshold_type threshold) const;
    template <typename InputIt, typename OutputIt>
    OutputIt operator()(
        const ge::sg::Mesh &mesh,
        InputIt             threshold_begin,
        InputIt             threshold_end,
        OutputIt            destination_begin) const;

    /// @brief Create regular series of simplified variants of the input mesh.
    template <typename OutputIt>
    OutputIt operator()(
        const ge::sg::Mesh &mesh,
        size_type           num_variants,
        OutputIt            out_begin) const;

protected:
    /// @brief Initialize the internal state for new mesh processing.
    void initialize(graph::Mesh &mesh) const;

    /// @brief Indicate that decimation with current parameters should continue.
    bool continue_decimation() const;

    /// @brief Perform one round of the decimation.
    void decimate() const;

private:
    Metric<Tag>   m_metric;    ///< Metric to use for evaluation.
    Operator<Tag> m_operator;  ///< Operator used for decimation.

    /// @brief Currently decimated mesh.
    mutable graph::Mesh *m_mesh = nullptr;
    /// @brief Elements currently scheduled for decimation.
    mutable queue_type m_queue = {};
    /// @brief Elements that need to be re-evaluated before decimation.
    mutable pointer_set_type m_dirty = {};
    /// @brief Current stop condition.
    mutable nonstd::variant<threshold_type, size_type> m_continue_cond
        = static_cast<threshold_type>(0);
};

/** @brief Convenience wrapper around the full LazySelection functor. */
template <
    typename T,
    template <class> class M,
    template <class> class O,
    typename... Args>
inline decltype(auto) lazy_selection(Args &&... args)
{
    return LazySelection<T, M, O>{}(std::forward<Args>(args)...);
}


// Inline and template members
/** Initialize the internal state with a new mesh.
 * @param[in] mesh The new mesh to decimate.
 */
template <typename T, template <class> class M, template <class> class O>
inline void LazySelection<T, M, O>::initialize(graph::Mesh &mesh) const
{
    m_mesh = std::addressof(mesh);
    m_queue = queue_type{};
    m_dirty = pointer_set_type{};

    // Fill queue with the initial costs.
    for (const auto &element : m_mesh->container<element_type>()) {
        m_queue.push(m_metric(element));
    }
}

/** @param[in] queue The current queue of operations scheduled for decimation.
 * @return True if the decimation should continue, false otherwise.
 */
template <typename T, template <class> class M, template <class> class O>
bool LazySelection<T, M, O>::continue_decimation() const
{
    using nonstd::get;
    using nonstd::holds_alternative;

    // Top value is cheaper than the threshold
    if (holds_alternative<threshold_type>(m_continue_cond)) {
        return m_queue.top().cost() < get<threshold_type>(m_continue_cond);
    }
    // Queue is larger than requested size.
    if (holds_alternative<size_type>(m_continue_cond)) {
        return m_queue.size() >= get<size_type>(m_continue_cond);
    }

    return false;  // XXX Exception?
}

/** Runs the decimation until the stop condition is fulfilled. */
template <typename T, template <class> class M, template <class> class O>
void LazySelection<T, M, O>::decimate() const
{
    while (!m_queue.empty() && continue_decimation()) {
        auto operation = m_queue.top();
        m_queue.pop();

        // skip operations with invalid elements
        if (!operation) {
            continue;
        }

        // lazy re-evaluation
        auto dirty_it = m_dirty.find(operation.element());
        if (dirty_it != m_dirty.end()) {
            m_queue.push(m_metric(operation.element().get()));
            m_dirty.erase(dirty_it);
        }
        // operator application
        else {
            auto modified = m_operator(*m_mesh, operation);
            for (auto &&element : modified) {
                m_dirty.insert(std::move(element));
            }
        }
    }
}

/** Decimates all elements that have metric-dependent cost
 * lesser than the threshold.
 * @param[in,out] mesh The mesh to decimate. It is decimated in-place.
 * @param[in] threshold The threshold to stop the decimation at.
 * @return Reference to the decimated mesh.
 */
template <typename T, template <class> class M, template <class> class O>
inline graph::Mesh &LazySelection<T, M, O>::operator()(
    graph::Mesh &mesh, threshold_type threshold) const
{
    initialize(mesh);
    m_continue_cond = std::move_if_noexcept(threshold);
    decimate();
    return mesh;
}

/** @overload
 * @param[in] mesh The original mesh to decimate.
 * @param[in] threshold The threshold to stop decimation at.
 * @return New decimated mesh.
 */
template <typename T, template <class> class M, template <class> class O>
inline ge::sg::Mesh LazySelection<T, M, O>::operator()(
    const ge::sg::Mesh &mesh, threshold_type threshold) const
{
    auto graph = graph::Mesh(mesh);

    operator()(graph, std::move_if_noexcept(threshold));

    return static_cast<ge::sg::Mesh>(graph);
}

/** @overload
 * This overload efficiently creates several variant of the original mesh
 * and stores them in container starting at destination_begin.
 * The destination_begin should conform to the same requirements
 * as i.e. d_first parameter of std::transform.
 * @see std::transform algorithm.
 * @note The threshold container SHOULD be sorted in ascending order,
 * otherwise some of them will be skipped.
 * @param[in] mesh The original mesh to decimate.
 * @param[in] threshold_begin The start of threshold container.
 * @param[in] threshold_end The end of threshold container.
 * @param[in] destination_begin The beginning of the output container.
 * @return Iterator after the last generated variant,
 * created from destination_begin.
 */
template <typename T, template <class> class M, template <class> class O>
template <typename InputIt, typename OutputIt>
OutputIt LazySelection<T, M, O>::operator()(
    const ge::sg::Mesh &mesh,
    InputIt             threshold_begin,
    InputIt             threshold_end,
    OutputIt            destination_begin) const
{
    auto graph = graph::Mesh(mesh);
    initialize(graph);
    std::for_each(threshold_begin, threshold_end, [&](const auto &threshold) {
        m_continue_cond = threshold;
        decimate();
        *destination_begin++ = static_cast<ge::sg::Mesh>(*m_mesh);
    });
    return destination_begin;
}

/** Efficiently creates several variants from one mesh.
 * Each variant is created after decimating a fraction of original elements.
 * @param[in] mesh The mesh to decimate.
 * @param[in] num_variants The number of variants to create.
 * @param[in] out_begin The start of output container.
 * @return Iterator after the last generated variant,
 * created from out_begin.
 */
template <typename T, template <class> class M, template <class> class O>
template <typename OutputIt>
OutputIt LazySelection<T, M, O>::operator()(
    const ge::sg::Mesh &mesh, size_type num_variants, OutputIt out_begin) const
{
    auto graph = graph::Mesh(mesh);
    initialize(graph);
    const auto queue_size = m_queue.size();

    auto sizes = std::vector<size_type>(num_variants - 1);
    std::generate(
        std::begin(sizes),
        std::end(sizes),
        // generate regular fractions
        [n = num_variants, size_unit = queue_size / num_variants]() mutable {
            return (--n) * size_unit;
        });

    std::for_each(std::cbegin(sizes), std::cend(sizes), [&](const auto &size) {
        m_continue_cond = static_cast<size_type>(size);
        decimate();
        *out_begin++ = static_cast<ge::sg::Mesh>(*m_mesh);
    });
    return out_begin;
}
}  // namespace algorithm
}  // namespace lod

#endif /* end of include guard: LAZY_SELECTION_H_DX3JHVCY */
