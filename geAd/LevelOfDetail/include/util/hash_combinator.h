#ifndef HASH_COMBINATOR_HPP_HM2SRLWQ
#define HASH_COMBINATOR_HPP_HM2SRLWQ

/** @file
 * @author Bc. Jan Staněk --- <xstane32@stud.fit.vutbr.cz>
 * @brief Private utility: Hash combinator.
 * Adapted from https://stackoverflow.com/questions/2590677 and Boost.
 */

#include <functional>

namespace lod {
namespace util {
// Variadic expansion end guard
constexpr std::size_t hash_combinator(std::size_t hash)
{
    return hash;
}

template <typename T, typename... Rest>
constexpr std::size_t hash_combinator(
    std::size_t hash, const T &next, Rest... rest)
{
    hash ^= std::hash<T>{}(next) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
    return hash_combinator(hash, std::forward<Rest>(rest)...);
}
}  // namespace util
}  // namespace lod

#endif /* end of include guard: HASH_COMBINATOR_HPP_HM2SRLWQ */
