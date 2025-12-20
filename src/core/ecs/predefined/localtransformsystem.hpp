#pragma once

#include "../../container.hpp"
#include "../core.hpp"
#include <span>
#include <vector>

#include "transform.hpp"
#include <components/predefined.hpp>
#include "../../geomhelper/geomhelper.hpp"

namespace Pelican {

DECLARE_MODULE(LocalTransformSystem) {
    struct TempTransform {
        LocalTransformComponent *local;
        TransformComponent *dst_ptr;
    };
    std::vector<TempTransform> tmpbuf;
    std::vector<uint8_t> tmpappearbuf;

  public:
    ECSCore* core = nullptr;
    using Query = std::span<ChunkView<EntityId, TransformComponent, LocalTransformComponent>>;
    void updateTransformRecursively(EntityId entity_id, TransformComponent& out_transform, LocalTransformComponent& local_transform, ECSCore& core, std::vector<uint8_t>& visited);

    void process(Query chunks);
};

} // namespace Pelican
