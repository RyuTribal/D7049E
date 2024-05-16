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
#include "Physics/PhysicsEngine.h"

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


	static bool IsKeyPressed(int key)
	{
		return Input::IsKeyPressed(key);
	}

	static bool IsMouseButtonPressed(int button)
	{
		return Input::IsMouseButtonPressed(button);
	}

	static void GetMousePosition(glm::vec2* mouse_position_destination)
	{
		*mouse_position_destination = glm::vec2(Input::GetMouseX(), Input::GetMouseY());
	}

	static bool Entity_HasComponent(uint64_t entity_id, MonoReflectionType* component_type)
	{
		auto [scene, entity] = GetSceneAndEntity(entity_id);

		MonoType* monoManagedType = mono_reflection_type_get_type(component_type);
		HVE_CORE_ASSERT(s_HasComponentFuncs.find(monoManagedType) != s_HasComponentFuncs.end());
		
		return s_HasComponentFuncs.at(monoManagedType)(entity);
	}

	static void Entity_Destroy(uint64_t entity_id)
	{
		auto [scene, entity] = GetSceneAndEntity(entity_id);
		scene->DestroyEntity(entity_id);
	}


	static void Camera_RotateAroundEntity(uint64_t entity_id, glm::vec2* rotation, float speed, bool inverse_controls)
	{
		auto [scene, entity] = GetSceneAndEntity(entity_id);
		auto camera_comp = entity->GetComponent<CameraComponent>();
		if (camera_comp)
		{
			camera_comp->camera.RotateAroundFocalPoint(*rotation, speed, inverse_controls);
		}
	}

	static void Camera_Rotate(uint64_t entity_id, glm::vec2* rotation, float speed, bool inverse_controls)
	{
		auto [scene, entity] = GetSceneAndEntity(entity_id);
		auto camera_comp = entity->GetComponent<CameraComponent>();
		if (camera_comp)
		{
			camera_comp->camera.Rotate(*rotation, speed, inverse_controls);
		}
	}

	static void Camera_GetForwardDirection(uint64_t entity_id, glm::vec3* forward_direction)
	{
		auto [scene, entity] = GetSceneAndEntity(entity_id);
		auto camera_comp = entity->GetComponent<CameraComponent>();
		if (camera_comp)
		{
			*forward_direction = camera_comp->camera.GetForwardDirection();
		}
	}

	static void Camera_GetRightDirection(uint64_t entity_id, glm::vec3* right_direction)
	{
		auto [scene, entity] = GetSceneAndEntity(entity_id);
		auto camera_comp = entity->GetComponent<CameraComponent>();
		if (camera_comp)
		{
			*right_direction = camera_comp->camera.GetRightDirection();
		}
	}

	static void Camera_GetPosition(uint64_t entity_id, glm::vec3* position)
	{
		auto [scene, entity] = GetSceneAndEntity(entity_id);
		auto camera_comp = entity->GetComponent<CameraComponent>();
		if (camera_comp)
		{
			*position = camera_comp->camera.GetPosition();
		}
	}

	static void Camera_GetRotation(uint64_t entity_id, glm::vec3* rotation)
	{
		auto [scene, entity] = GetSceneAndEntity(entity_id);
		auto camera_comp = entity->GetComponent<CameraComponent>();
		if (camera_comp)
		{
			*rotation = glm::eulerAngles(camera_comp->camera.GetOrientation());
		}
	}

	static void Camera_SetPosition(uint64_t entity_id, glm::vec3* position)
	{
		auto [scene, entity] = GetSceneAndEntity(entity_id);
		auto camera_comp = entity->GetComponent<CameraComponent>();
		if (camera_comp)
		{
			camera_comp->camera.SetPosition(*position);
		}
	}

	static void Camera_SetRotation(uint64_t entity_id, glm::vec3* rotation)
	{
		auto [scene, entity] = GetSceneAndEntity(entity_id);
		auto camera_comp = entity->GetComponent<CameraComponent>();
		if (camera_comp)
		{
			camera_comp->camera.SetRotation(glm::vec2(*rotation));
		}
	}


	static void TransformComponent_GetTranslation(uint64_t entity_id, glm::vec3* out_translation)
	{
		auto [scene, entity] = GetSceneAndEntity(entity_id);
		*out_translation = entity->GetComponent<TransformComponent>()->world_transform.translation;
	}

	static void TransformComponent_SetTranslation(uint64_t entity_id, glm::vec3* in_translation)
	{
		auto [scene, entity] = GetSceneAndEntity(entity_id);
		entity->GetComponent<TransformComponent>()->world_transform.translation = *in_translation;
	}

	static void TransformComponent_GetRotation(uint64_t entity_id, glm::vec3* out_rotation)
	{
		auto [scene, entity] = GetSceneAndEntity(entity_id);
		*out_rotation = entity->GetComponent<TransformComponent>()->world_transform.rotation;
	}

	static void TransformComponent_SetRotation(uint64_t entity_id, glm::vec3* in_rotation)
	{
		auto [scene, entity] = GetSceneAndEntity(entity_id);
		entity->GetComponent<TransformComponent>()->world_transform.rotation = *in_rotation;
	}

	static void TransformComponent_GetScale(uint64_t entity_id, glm::vec3* out_scale)
	{
		auto [scene, entity] = GetSceneAndEntity(entity_id);
		*out_scale = entity->GetComponent<TransformComponent>()->world_transform.scale;
	}

	static void TransformComponent_SetScale(uint64_t entity_id, glm::vec3* in_scale)
	{
		auto [scene, entity] = GetSceneAndEntity(entity_id);
		entity->GetComponent<TransformComponent>()->world_transform.scale = *in_scale;
	}

	static void Sounds_PlaySoundAtIndexGlobal(uint64_t entity_id, int index)
	{
		auto [scene, entity] = GetSceneAndEntity(entity_id);
		auto sounds_library = entity->GetComponent<GlobalSoundsComponent>();
		if (sounds_library && index <= sounds_library->Sounds.size() - 1)
		{
			sounds_library->Sounds.at(index)->PlaySound(false);
		}
	}

	static void Sounds_PlaySoundAtIndexLocal(uint64_t entity_id, int index)
	{
		auto [scene, entity] = GetSceneAndEntity(entity_id);
		auto sounds_library = entity->GetComponent<LocalSoundsComponent>();
		if (sounds_library && index <= sounds_library->Sounds.size() - 1)
		{
			sounds_library->Sounds.at(index)->PlaySound(scene->GetCurrentCamera()->CalculatePosition(), false);
		}
	}

	static void BoxCollider_GetLinearVelocity(uint64_t entity_id, glm::vec3* out_velocity)
	{
		auto [scene, entity] = GetSceneAndEntity(entity_id);
		auto box_collider = entity->GetComponent<BoxColliderComponent>();
		if (box_collider)
		{
			*out_velocity = PhysicsEngine::Get()->GetCurrentScene()->GetLinearVelocity(entity_id);
		}
	}

	static void BoxCollider_SetLinearVelocity(uint64_t entity_id, glm::vec3* in_velocity)
	{
		auto [scene, entity] = GetSceneAndEntity(entity_id);
		auto box_collider = entity->GetComponent<BoxColliderComponent>();
		if (box_collider)
		{
			PhysicsEngine::Get()->GetCurrentScene()->SetLinearVelocity(entity_id, *in_velocity);
		}
	}

	static void BoxCollider_AddLinearVelocity(uint64_t entity_id, glm::vec3* velocity)
	{
		auto [scene, entity] = GetSceneAndEntity(entity_id);
		auto box_collider = entity->GetComponent<BoxColliderComponent>();
		if (box_collider)
		{
			glm::vec3 curr_velocity = PhysicsEngine::Get()->GetCurrentScene()->GetLinearVelocity(entity_id);
			curr_velocity += *velocity;
			PhysicsEngine::Get()->GetCurrentScene()->SetLinearVelocity(entity_id, curr_velocity);
		}
	}

	static void BoxCollider_AddAngularVelocity(uint64_t entity_id, glm::vec3* velocity)
	{
		auto [scene, entity] = GetSceneAndEntity(entity_id);
		auto box_collider = entity->GetComponent<BoxColliderComponent>();
		if (box_collider)
		{
			glm::vec3 curr_velocity = PhysicsEngine::Get()->GetCurrentScene()->GetAngularVelocity(entity_id);
			curr_velocity += *velocity;
			PhysicsEngine::Get()->GetCurrentScene()->SetLinearVelocity(entity_id, curr_velocity);
		}
	}

	static void BoxCollider_AddImpulse(uint64_t entity_id, glm::vec3* impulse)
	{
		auto [scene, entity] = GetSceneAndEntity(entity_id);
		auto box_collider = entity->GetComponent<BoxColliderComponent>();
		if (box_collider)
		{
			PhysicsEngine::Get()->GetCurrentScene()->AddLinearImpulse(entity_id, *impulse);
		}
	}

	static void BoxCollider_AddAngularImpulse(uint64_t entity_id, glm::vec3* impulse)
	{
		auto [scene, entity] = GetSceneAndEntity(entity_id);
		auto box_collider = entity->GetComponent<BoxColliderComponent>();
		if (box_collider)
		{
			PhysicsEngine::Get()->GetCurrentScene()->AddAngularImpulse(entity_id, *impulse);
		}
	}

	static void BoxCollider_AddLinearAngularImpulse(uint64_t entity_id, glm::vec3* linear_impulse, glm::vec3* angular_impulse)
	{
		auto [scene, entity] = GetSceneAndEntity(entity_id);
		auto box_collider = entity->GetComponent<BoxColliderComponent>();
		if (box_collider)
		{
			PhysicsEngine::Get()->GetCurrentScene()->AddLinearAndAngularImpulse(entity_id, *linear_impulse, *angular_impulse);
		}
	}


	static void SphereCollider_GetLinearVelocity(uint64_t entity_id, glm::vec3* out_velocity)
	{
		auto [scene, entity] = GetSceneAndEntity(entity_id);
		auto sphere_collider = entity->GetComponent<SphereColliderComponent>();
		if (sphere_collider)
		{
			*out_velocity = PhysicsEngine::Get()->GetCurrentScene()->GetLinearVelocity(entity_id);
		}
	}

	static void SphereCollider_SetLinearVelocity(uint64_t entity_id, glm::vec3* in_velocity)
	{
		auto [scene, entity] = GetSceneAndEntity(entity_id);
		auto sphere_collider = entity->GetComponent<SphereColliderComponent>();
		if (sphere_collider)
		{
			PhysicsEngine::Get()->GetCurrentScene()->SetLinearVelocity(entity_id, *in_velocity);
		}
	}

	static void SphereCollider_AddLinearVelocity(uint64_t entity_id, glm::vec3* velocity)
	{
		auto [scene, entity] = GetSceneAndEntity(entity_id);
		auto sphere_collider = entity->GetComponent<SphereColliderComponent>();
		if (sphere_collider)
		{
			glm::vec3 curr_velocity = PhysicsEngine::Get()->GetCurrentScene()->GetLinearVelocity(entity_id);
			curr_velocity += *velocity;
			PhysicsEngine::Get()->GetCurrentScene()->SetLinearVelocity(entity_id, curr_velocity);
		}
	}

	static void SphereCollider_AddAngularVelocity(uint64_t entity_id, glm::vec3* velocity)
	{
		auto [scene, entity] = GetSceneAndEntity(entity_id);
		auto sphere_collider = entity->GetComponent<SphereColliderComponent>();
		if (sphere_collider)
		{
			glm::vec3 curr_velocity = PhysicsEngine::Get()->GetCurrentScene()->GetAngularVelocity(entity_id);
			curr_velocity += *velocity;
			PhysicsEngine::Get()->GetCurrentScene()->SetLinearVelocity(entity_id, curr_velocity);
		}
	}

	static void SphereCollider_AddImpulse(uint64_t entity_id, glm::vec3* impulse)
	{
		auto [scene, entity] = GetSceneAndEntity(entity_id);
		auto sphere_collider = entity->GetComponent<SphereColliderComponent>();
		if (sphere_collider)
		{
			PhysicsEngine::Get()->GetCurrentScene()->AddLinearImpulse(entity_id, *impulse);
		}
	}

	static void SphereCollider_AddAngularImpulse(uint64_t entity_id, glm::vec3* impulse)
	{
		auto [scene, entity] = GetSceneAndEntity(entity_id);
		auto sphere_collider = entity->GetComponent<SphereColliderComponent>();
		if (sphere_collider)
		{
			PhysicsEngine::Get()->GetCurrentScene()->AddAngularImpulse(entity_id, *impulse);
		}
	}

	static void SphereCollider_AddLinearAngularImpulse(uint64_t entity_id, glm::vec3* linear_impulse, glm::vec3* angular_impulse)
	{
		auto [scene, entity] = GetSceneAndEntity(entity_id);
		auto sphere_collider = entity->GetComponent<SphereColliderComponent>();
		if (sphere_collider)
		{
			PhysicsEngine::Get()->GetCurrentScene()->AddLinearAndAngularImpulse(entity_id, *linear_impulse, *angular_impulse);
		}
	}

	static void CharacterController_GetLinearVelocity(uint64_t entity_id, glm::vec3* out_velocity)
	{
		auto [scene, entity] = GetSceneAndEntity(entity_id);
		auto character_controller = entity->GetComponent<CharacterControllerComponent>();
		if (character_controller)
		{
			*out_velocity = PhysicsEngine::Get()->GetCurrentScene()->GetLinearVelocity(entity_id);
		}
	}

	static void CharacterController_SetLinearVelocity(uint64_t entity_id, glm::vec3* in_velocity)
	{
		auto [scene, entity] = GetSceneAndEntity(entity_id);
		auto character_controller = entity->GetComponent<CharacterControllerComponent>();
		if (character_controller)
		{
			PhysicsEngine::Get()->GetCurrentScene()->SetLinearVelocity(entity_id, *in_velocity);
		}
	}

	static void CharacterController_AddLinearVelocity(uint64_t entity_id, glm::vec3* velocity)
	{
		auto [scene, entity] = GetSceneAndEntity(entity_id);
		auto character_controller = entity->GetComponent<CharacterControllerComponent>();
		if (character_controller)
		{
			glm::vec3 curr_velocity = PhysicsEngine::Get()->GetCurrentScene()->GetLinearVelocity(entity_id);
			curr_velocity += *velocity;
			PhysicsEngine::Get()->GetCurrentScene()->SetLinearVelocity(entity_id, curr_velocity);
		}
	}

	static void CharacterController_AddAngularVelocity(uint64_t entity_id, glm::vec3* velocity)
	{
		auto [scene, entity] = GetSceneAndEntity(entity_id);
		auto character_controller = entity->GetComponent<CharacterControllerComponent>();
		if (character_controller)
		{
			glm::vec3 curr_velocity = PhysicsEngine::Get()->GetCurrentScene()->GetAngularVelocity(entity_id);
			curr_velocity += *velocity;
			PhysicsEngine::Get()->GetCurrentScene()->SetLinearVelocity(entity_id, curr_velocity);
		}
	}

	static void CharacterController_AddImpulse(uint64_t entity_id, glm::vec3* impulse)
	{
		auto [scene, entity] = GetSceneAndEntity(entity_id);
		auto character_controller = entity->GetComponent<CharacterControllerComponent>();
		if (character_controller)
		{
			PhysicsEngine::Get()->GetCurrentScene()->AddLinearImpulse(entity_id, *impulse);
		}
	}


	static void CharacterController_AddAngularImpulse(uint64_t entity_id, glm::vec3* impulse)
	{
		auto [scene, entity] = GetSceneAndEntity(entity_id);
		auto character_controller = entity->GetComponent<CharacterControllerComponent>();
		if (character_controller)
		{
			PhysicsEngine::Get()->GetCurrentScene()->AddAngularImpulse(entity_id, *impulse);
		}
	}

	static void CharacterController_AddLinearAngularImpulse(uint64_t entity_id, glm::vec3* linear_impulse, glm::vec3* angular_impulse)
	{
		auto [scene, entity] = GetSceneAndEntity(entity_id);
		auto character_controller = entity->GetComponent<CharacterControllerComponent>();
		if (character_controller)
		{
			PhysicsEngine::Get()->GetCurrentScene()->AddLinearAndAngularImpulse(entity_id, *linear_impulse, *angular_impulse);
		}
	}

	static bool CharacterController_IsCharacterGrounded(uint64_t entity_id)
	{
		auto [scene, entity] = GetSceneAndEntity(entity_id);
		auto character_controller = entity->GetComponent<CharacterControllerComponent>();
		if (character_controller)
		{
			return PhysicsEngine::Get()->GetCurrentScene()->IsCharacterGrounded(entity_id);
		}
	}

	static void CharacterController_GetRotation(uint64_t entity_id, glm::vec3* rotation)
	{
		auto [scene, entity] = GetSceneAndEntity(entity_id);
		auto character_controller = entity->GetComponent<CharacterControllerComponent>();
		if (character_controller)
		{
			*rotation = PhysicsEngine::Get()->GetCurrentScene()->GetRotation(entity_id);
		}
	}

	static void CharacterController_SetRotation(uint64_t entity_id, glm::vec3* rotation)
	{
		auto [scene, entity] = GetSceneAndEntity(entity_id);
		auto character_controller = entity->GetComponent<CharacterControllerComponent>();
		if (character_controller)
		{
			PhysicsEngine::Get()->GetCurrentScene()->SetRotation(entity_id, *rotation);
		}
	}

	static void CharacterController_Rotate(uint64_t entity_id, glm::vec3* rotation)
	{
		auto [scene, entity] = GetSceneAndEntity(entity_id);
		auto character_controller = entity->GetComponent<CharacterControllerComponent>();
		if (character_controller)
		{
			PhysicsEngine::Get()->GetCurrentScene()->Rotate(entity_id, *rotation);
		}
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
				HVE_CORE_WARN("Could not find component type - {}", managedTypename);
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

		HVE_ADD_INTERNAL_CALL(IsKeyPressed);
		HVE_ADD_INTERNAL_CALL(IsMouseButtonPressed);
		HVE_ADD_INTERNAL_CALL(GetMousePosition);
		HVE_ADD_INTERNAL_CALL(Entity_HasComponent);
		HVE_ADD_INTERNAL_CALL(Entity_Destroy);

		HVE_ADD_INTERNAL_CALL(Camera_RotateAroundEntity);
		HVE_ADD_INTERNAL_CALL(Camera_Rotate);
		HVE_ADD_INTERNAL_CALL(Camera_GetForwardDirection);
		HVE_ADD_INTERNAL_CALL(Camera_GetRightDirection);
		HVE_ADD_INTERNAL_CALL(Camera_GetPosition);
		HVE_ADD_INTERNAL_CALL(Camera_GetRotation);
		HVE_ADD_INTERNAL_CALL(Camera_SetPosition);
		HVE_ADD_INTERNAL_CALL(Camera_SetRotation);

		HVE_ADD_INTERNAL_CALL(TransformComponent_GetTranslation);
		HVE_ADD_INTERNAL_CALL(TransformComponent_SetTranslation);
		HVE_ADD_INTERNAL_CALL(TransformComponent_GetRotation);
		HVE_ADD_INTERNAL_CALL(TransformComponent_SetRotation);
		HVE_ADD_INTERNAL_CALL(TransformComponent_GetScale);
		HVE_ADD_INTERNAL_CALL(TransformComponent_SetScale);

		HVE_ADD_INTERNAL_CALL(Sounds_PlaySoundAtIndexGlobal);
		HVE_ADD_INTERNAL_CALL(Sounds_PlaySoundAtIndexLocal);

		HVE_ADD_INTERNAL_CALL(BoxCollider_GetLinearVelocity);
		HVE_ADD_INTERNAL_CALL(BoxCollider_SetLinearVelocity);
		HVE_ADD_INTERNAL_CALL(BoxCollider_AddLinearVelocity);
		HVE_ADD_INTERNAL_CALL(BoxCollider_AddAngularVelocity);
		HVE_ADD_INTERNAL_CALL(BoxCollider_AddImpulse);
		HVE_ADD_INTERNAL_CALL(BoxCollider_AddAngularImpulse);
		HVE_ADD_INTERNAL_CALL(BoxCollider_AddLinearAngularImpulse);

		HVE_ADD_INTERNAL_CALL(SphereCollider_GetLinearVelocity);
		HVE_ADD_INTERNAL_CALL(SphereCollider_SetLinearVelocity);
		HVE_ADD_INTERNAL_CALL(SphereCollider_AddLinearVelocity);
		HVE_ADD_INTERNAL_CALL(SphereCollider_AddAngularVelocity);
		HVE_ADD_INTERNAL_CALL(SphereCollider_AddImpulse);
		HVE_ADD_INTERNAL_CALL(SphereCollider_AddAngularImpulse);
		HVE_ADD_INTERNAL_CALL(SphereCollider_AddLinearAngularImpulse);

		HVE_ADD_INTERNAL_CALL(CharacterController_GetLinearVelocity);
		HVE_ADD_INTERNAL_CALL(CharacterController_SetLinearVelocity);
		HVE_ADD_INTERNAL_CALL(CharacterController_AddLinearVelocity);
		HVE_ADD_INTERNAL_CALL(CharacterController_AddAngularVelocity);
		HVE_ADD_INTERNAL_CALL(CharacterController_AddImpulse);
		HVE_ADD_INTERNAL_CALL(CharacterController_AddAngularImpulse);
		HVE_ADD_INTERNAL_CALL(CharacterController_AddLinearAngularImpulse);
		HVE_ADD_INTERNAL_CALL(CharacterController_IsCharacterGrounded);
		HVE_ADD_INTERNAL_CALL(CharacterController_GetRotation);
		HVE_ADD_INTERNAL_CALL(CharacterController_SetRotation);
		HVE_ADD_INTERNAL_CALL(CharacterController_Rotate);

	}
}
