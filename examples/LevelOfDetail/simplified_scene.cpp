/** @file
 * @author Bc. Jan Staněk --- <xstane32@stud.fit.vutbr.cz>
 * @brief Structure holding mesh variants for a simplified scene
 * (implementation).
 */

#include <algorithm>
#include <iterator>
#include <vector>

#include <LoDGenerator.h>

#include "scene_util.h"
#include "simplified_scene.h"


/** Prepare the internal structure for further simplification.
 * @param[in] scene The original scene.
 */
SimplifiedScene::SimplifiedScene(scene_pointer_type scene)
    : m_scene(std::move(scene))
{
    using MC = util::SceneWalker::iterator::MeshContainer;

    for (const auto &model : util::SceneWalker(m_scene)) {
        for (auto &&mesh : std::get<MC &>(model)) {
            m_variants.insert({std::addressof(mesh), {mesh}});
        }
    }
}

/** Generate simplified variants of all meshes in the scene.
 * @param[in] num_variants How many levels of simplification should be
 * generated.
 * @return True if any generation has taken place, false otherwise (the scene is
 * empty).
 */
bool SimplifiedScene::generate(std::size_t num_variants)
{
    using lod::algorithm::MaxError;

    if (m_variants.empty()) {
        return false;
    }

    auto       thresholds = std::vector<MaxError<float>>(num_variants);
    const auto step = 1.f / static_cast<float>(num_variants);

    std::generate(
        std::begin(thresholds), std::end(thresholds), [&, cur = 0.f]() mutable {
            return MaxError<float>{cur += step};
        });

    for (auto &&item : m_variants) {
        item.second.resize(1);  // only keep the original
        lod::simplify(
            item.second[0],
            std::cbegin(thresholds),
            std::cend(thresholds),
            std::back_inserter(item.second));
    }

    return true;
}

/** Selects the appropriate level to display in scene.
 * @param[in] level_index Index of the level to select; 0 is original.
 */
void SimplifiedScene::select_level(std::size_t level_index)
{
    for (const auto &item : m_variants) {
        *item.first = item.second.at(level_index);
    }
}
