#include "pch.h"
#include "ScriptGlue.h"
#include "Core/UUID.h"
#include <mono/metadata/assembly.h>
#include <mono/metadata/object.h>
#include <mono/metadata/reflection.h>
#include "Scene/Scene.h"
#include "Scene/Entity.h"
#include "Scene/Components.h"
#include "ScriptEngine.h"

namespace Engine {

	static std::unordered_map<MonoType*, std::function<bool(Entity*)>> s_HasComponentFuncs;

#define HVE_ADD_INTERNAL_CALL(Name) mono_add_internal_call("Helios.InternalCalls::" #Name, Name)

	std::pair<Scene*, Entity*> GetSceneAndEntity(UUID entity_id)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		HVE_CORE_ASSERT(scene);
		Entity* entity = scene->GetEntity(entity_id);
		HVE_CORE_ASSERT(entity);
		return { scene, entity };
	}

	static bool Entity_HasComponent(uint64_t entity_id, MonoReflectionType* component_type)
	{
		auto [scene, entity] = GetSceneAndEntity(entity_id);

		MonoType* monoManagedType = mono_reflection_type_get_type(component_type);
		HVE_CORE_ASSERT(s_HasComponentFuncs.find(monoManagedType) != s_HasComponentFuncs.end());
		
		return s_HasComponentFuncs.at(monoManagedType)(entity);
	}

	static void TransformComponent_GetTranslation(uint64_t entity_id, glm::vec3* out_translation)
	{
		auto [scene, entity] = GetSceneAndEntity(entity_id);
		*out_translation = entity->GetComponent<TransformComponent>()->local_transform.translation;
	}

	static void TransformComponent_SetTranslation(uint64_t entity_id, glm::vec3* in_translation)
	{
		auto [scene, entity] = GetSceneAndEntity(entity_id);
		entity->GetComponent<TransformComponent>()->local_transform.translation = *in_translation;
	}

	static void TransformComponent_GetRotation(uint64_t entity_id, glm::vec3* out_rotation)
	{
		auto [scene, entity] = GetSceneAndEntity(entity_id);
		*out_rotation = entity->GetComponent<TransformComponent>()->local_transform.rotation;
	}

	static void TransformComponent_SetRotation(uint64_t entity_id, glm::vec3* in_rotation)
	{
		auto [scene, entity] = GetSceneAndEntity(entity_id);
		entity->GetComponent<TransformComponent>()->local_transform.rotation = *in_rotation;
	}

	static void TransformComponent_GetScale(uint64_t entity_id, glm::vec3* out_scale)
	{
		auto [scene, entity] = GetSceneAndEntity(entity_id);
		*out_scale = entity->GetComponent<TransformComponent>()->local_transform.scale;
	}

	static void TransformComponent_SetScale(uint64_t entity_id, glm::vec3* in_scale)
	{
		auto [scene, entity] = GetSceneAndEntity(entity_id);
		entity->GetComponent<TransformComponent>()->local_transform.scale = *in_scale;
	}


	template <typename... Component>
	static void RegisterComponent()
	{
		([]() {
			std::string_view class_name = typeid(Component).name();
			size_t pos = class_name.find_last_of(':'); // If someone places a component outside of the Engine namespace, you messed up
			std::string_view structName = class_name.substr(pos + 1);
			std::string managedTypename = fmt::format("Helios.{}", structName);
			MonoType* managed_type = mono_reflection_type_from_name(managedTypename.data(), ScriptEngine::GetCoreAssemblyImage());
			s_HasComponentFuncs[managed_type] = [](Entity* entity) {return entity->HasComponent<Component>(); };
			if (!managed_type)
			{
				HVE_CORE_ERROR("Could not find component type - {}", managedTypename);
				return;
			}
			HVE_CORE_TRACE("Bound classes C#: <{0}> to C++: <{1}>", managedTypename, class_name);
		}(), ...);
	}

	template<typename... Component>
	static void RegisterComponent(ComponentGroup<Component...>)
	{
		RegisterComponent<Component...>();
	}

	void ScriptGlue::RegisterComponents()
	{
		RegisterComponent(AllComponents{});
	}

	void ScriptGlue::RegisterFunctions()
	{
		HVE_ADD_INTERNAL_CALL(Entity_HasComponent);

		HVE_ADD_INTERNAL_CALL(TransformComponent_GetTranslation);
		HVE_ADD_INTERNAL_CALL(TransformComponent_SetTranslation);
		HVE_ADD_INTERNAL_CALL(TransformComponent_GetRotation);
		HVE_ADD_INTERNAL_CALL(TransformComponent_SetRotation);
		HVE_ADD_INTERNAL_CALL(TransformComponent_GetScale);
		HVE_ADD_INTERNAL_CALL(TransformComponent_SetScale);


	}
}
