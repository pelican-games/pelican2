#include "localtransformsystem.hpp"
#include <iostream>

namespace Pelican {

void LocalTransformSystem::updateTransformRecursively(EntityId entity_id, TransformComponent& out_transform, LocalTransformComponent& local_transform, ECSCore& core, std::vector<uint8_t>& visited) {
    if (entity_id >= visited.size()) {
        visited.resize(entity_id * 2 + 1024, 0);
    }

    if (visited[entity_id]) return;
    visited[entity_id] = 1;
    glm::vec3 parent_pos(0.0f);
    glm::quat parent_rot(1.0f, 0.0f, 0.0f, 0.0f);
    glm::vec3 parent_scale(1.0f);

    if (local_transform.parent != UINT32_MAX) {
        auto* parent_t = core.getComponent<TransformComponent>(local_transform.parent);
        auto* parent_l = core.getComponent<LocalTransformComponent>(local_transform.parent);
        if (parent_t && parent_l) {
            updateTransformRecursively(local_transform.parent, *parent_t, *parent_l, core, visited);
            parent_pos = parent_t->pos;
            parent_rot = parent_t->rotation;
            parent_scale = parent_t->scale;
        } else if (parent_t) {
            parent_pos = parent_t->pos;
            parent_rot = parent_t->rotation;
            parent_scale = parent_t->scale;
        } else {
             std::cout << "  [Rec] Parent NOT found components!" << std::endl;
        }
    }
    out_transform.scale = parent_scale * to_glm(local_transform.scale);
    out_transform.rotation = parent_rot * to_glm(local_transform.rotation);
    out_transform.pos = parent_pos + (parent_rot * (parent_scale * to_glm(local_transform.pos)));
}

void LocalTransformSystem::process(Query chunks) {
    if (!core) return;
    auto& core_ref = *core;
    if(tmpappearbuf.empty()) tmpappearbuf.resize(256);
    std::fill(tmpappearbuf.begin(), tmpappearbuf.end(), 0);

    for (auto &chunk : chunks) {
        auto [entity_ids, transforms, localtransforms] = chunk.components;
        
        for (size_t i = 0; i < chunk.count; i++) {
            EntityId id = entity_ids[i];
            
            if (id >= tmpappearbuf.size()) {
                tmpappearbuf.resize(id * 2 + 1024, 0);
            }

            if (!tmpappearbuf[id]) {
                updateTransformRecursively(id, transforms[i], localtransforms[i], core_ref, tmpappearbuf);
            }
        }
    }
}

} // namespace Pelican
