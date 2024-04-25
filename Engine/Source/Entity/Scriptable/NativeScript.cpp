#include "epch.h"
#include "NativeScript.h"

#include "Entity/Entity.h"

namespace Cosmos::entity
{
	NativeScript::NativeScript(const char* name, Entity* entity)
		: mName(name), mEntity(entity)
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