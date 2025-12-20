#include "predefined.hpp"
#include "componentinfo.hpp"
#include "core.hpp"

#include "predefined/camera.hpp"
#include "predefined/collision.hpp"
#include "predefined/modelview.hpp"
#include "predefined/transform.hpp"

#include "predefined/camerasystem.hpp"
#include "predefined/localtransformsystem.hpp"
#include "predefined/modelviewtransoformsystem.hpp"

#include <details/component/registerer.hpp>

namespace Pelican {

void ECSPredefinedRegistration::reg() {
    // predefined component
    GET_MODULE(ComponentInfoManager)
        .registerComponent(ComponentInfo{
            .id = ComponentIdByType<EntityId>::value,
            .size = sizeof(EntityId),
            .name = "eid",
        });
    internal::getComponentRegisterer().registerComponent<TransformComponent>("transform");
    internal::getComponentRegisterer().registerComponent<SimpleModelViewComponent>("simplemodelview");
    internal::getComponentRegisterer().registerComponent<CameraComponent>("camera");
    internal::getComponentRegisterer().registerComponent<LocalTransformComponent>("localtransform");

    GET_MODULE(ECSCore)
        .registerSystemForce<SimpleModelViewTransformSystem, TransformComponent, SimpleModelViewComponent>(
            GET_MODULE(SimpleModelViewTransformSystem), {});
    GET_MODULE(ECSCore).registerSystemForce<CameraSystem, TransformComponent, CameraComponent>(GET_MODULE(CameraSystem),
                                                                                               {});
    auto& loc_sys = GET_MODULE(LocalTransformSystem);
    loc_sys.core = &GET_MODULE(ECSCore);
    GET_MODULE(ECSCore)
        .registerSystemForce<LocalTransformSystem, EntityId, TransformComponent, LocalTransformComponent>(
            loc_sys, {});

    GET_MODULE(ECSCore).registerSystemForce<SimpleCollisionSystem, TransformComponent, SphereColliderComponent>(
        GET_MODULE(SimpleCollisionSystem), {});
}

} // namespace Pelican
