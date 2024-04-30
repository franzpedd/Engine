#include "epch.h"
#include "NativeScript.h"

#include "Entity/Entity.h"

namespace Cosmos
{
	NativeScript::NativeScript(Entity* entity, const char* name)
		: mEntity(entity), mName(name)
	{
		//// create a new entity on the scene if entity is nullptr
		//if (!mEntity)
		//{
		//	mEntity = Scene::Get()->CreateEntity(mName);
		//}
		//
		//mEntity->AddComponent<NativeScriptComponent>();
		//mEntity->GetComponent<NativeScriptComponent>().script = this;
	}

	NativeScript::~NativeScript()
	{
		//mEntity->GetComponent<NativeScriptComponent>().script = nullptr;
		//Scene::Get()->DestroyEntity(mEntity);
	}
}