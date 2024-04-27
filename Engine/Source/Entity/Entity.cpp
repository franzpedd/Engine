#include "epch.h"
#include "Entity.h"

namespace Cosmos
{
    Entity::Entity(Scene* scene, entt::entity enttHandle, UUID id)
        : mScene(scene), mEntityHandle(enttHandle), mUUID(id)
    {

    }
}