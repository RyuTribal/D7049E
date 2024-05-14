#pragma once
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include "Renderer/Camera.h"
#include "Renderer/Material.h"
#include "Renderer/Mesh.h"
#include "Lights/PointLight.h"
#include "Lights/DirectionalLight.h"
#include <Script/ScriptEngine.h>
#include <Physics/Auxiliary/HEMotionType.h>
#include <Sound/GlobalSource.h>
#include <Sound/LocalSource.h>

namespace Engine {

	struct IDComponent  {
		UUID id;

		IDComponent() = default;
		IDComponent(const IDComponent&) = default;
		IDComponent(UUID new_id) : id(new_id){}
	};

	struct ParentIDComponent  {
		UUID id;

		ParentIDComponent() = default;
		ParentIDComponent(const ParentIDComponent&) = default;
		ParentIDComponent(UUID new_id) : id(new_id) {}

	};

	struct TagComponent  {
		std::string name = "Entity";

		TagComponent() = default;
		TagComponent(const TagComponent&) = default;
		TagComponent(const std::string& name) : name(name) {}

	};

	struct LocalTransformComponent  {
		glm::vec3 translation = { 0.0f, 0.0f, 0.0f };
		glm::vec3 rotation = { 0.0f, 0.0f, 0.0f };
		glm::vec3 scale = { 1.0f, 1.0f, 1.0f };

		LocalTransformComponent() = default;
		LocalTransformComponent(const LocalTransformComponent&) = default;
		LocalTransformComponent(const glm::vec3& new_translation) : translation(new_translation) {}

		glm::mat4 mat4() {
			glm::mat4 Rotation = glm::toMat4(glm::quat(rotation));

			return glm::translate(glm::mat4(1.0f), translation)
				* Rotation
				* glm::scale(glm::mat4(1.0f), scale);
		}

		glm::quat RotationVecToQuat()
		{
			return glm::quat(rotation);
		}
	};

	struct WorldTransformComponent  {
		glm::vec3 translation = { 0.0f, 0.0f, 0.0f };
		glm::vec3 rotation = { 0.0f, 0.0f, 0.0f };
		glm::vec3 scale = { 1.0f, 1.0f, 1.0f };

		WorldTransformComponent() = default;
		WorldTransformComponent(const WorldTransformComponent&) = default;
		WorldTransformComponent(const glm::vec3& new_translation) : translation(new_translation) {}

		glm::mat4 mat4() {
			glm::mat4 Rotation = glm::toMat4(glm::quat(rotation));

			return glm::translate(glm::mat4(1.0f), translation)
				* Rotation
				* glm::scale(glm::mat4(1.0f), scale);
		}

		glm::quat RotationVecToQuat()
		{
			return glm::quat(rotation);
		}
	};

	struct TransformComponent  {
		WorldTransformComponent world_transform{};
		LocalTransformComponent local_transform{};

		TransformComponent() = default;
		TransformComponent(const TransformComponent&) = default;
		TransformComponent(const glm::vec3& new_translation) : local_transform(new_translation) {}
	};

	struct MeshComponent  {
		Ref<Mesh> mesh = CreateRef<Mesh>(nullptr);

		MeshComponent() = default;
		MeshComponent(const MeshComponent&) = default;
		MeshComponent(Ref<Mesh> new_mesh) : mesh(new_mesh) {}
	};


	struct CameraComponent  {
		Camera camera{};
		bool IsPrimary = false;

		CameraComponent() = default;
		CameraComponent(const CameraComponent&) = default;
		CameraComponent(Camera new_camera) : camera(new_camera) {}
	};

	struct PointLightComponent  {
		PointLight light{};

		PointLightComponent()  = default;
		PointLightComponent(const PointLightComponent&) = default;
		PointLightComponent(PointLight new_light) : light(new_light) {}
	};

	struct DirectionalLightComponent 
	{
		DirectionalLight light;

		DirectionalLightComponent() = default;
		DirectionalLightComponent(const DirectionalLightComponent&) = default;
		DirectionalLightComponent(DirectionalLight new_light) : light(new_light) {}
	};

	struct GlobalSoundsComponent 
	{
		std::vector<Ref<GlobalSource>> Sounds;

		GlobalSoundsComponent() = default;
		GlobalSoundsComponent(const GlobalSoundsComponent&) = default;
	};

	struct LocalSoundsComponent
	{
		std::vector<Ref<LocalSource>> Sounds;

		LocalSoundsComponent() = default;
		LocalSoundsComponent(const LocalSoundsComponent&) = default;
	};

	struct ScriptComponent 
	{
		Ref<ScriptClass> script = nullptr;
		std::string Name = "";

		ScriptComponent() = default;
		ScriptComponent(const ScriptComponent&) = default;
		ScriptComponent(Ref<ScriptClass> new_script) : script(new_script) {}
	};

	/// Physics stuff
	struct ColliderMaterial
	{
		float Friction = 1.f;
		float Restituion = 1.f;

	};
	struct CharacterControllerComponent
	{
		float Mass = 70.f;
		float HalfHeight = 0.5f;
		float Radius = 0.5f;
		float Friction = 0.5f;
		float Restitution = 0.5f;
		glm::vec3 Offset = glm::vec3(0, 0, 0);

		CharacterControllerComponent() = default;
		CharacterControllerComponent(const CharacterControllerComponent&) = default;
		CharacterControllerComponent(float halfHeight, float radius) : HalfHeight(halfHeight), Radius(radius) {}
		CharacterControllerComponent(float mass, float halfHeight, float radius, glm::vec3 offset)
			: Mass(mass), HalfHeight(halfHeight), Radius(radius), Offset(offset){}
	};

	struct BoxColliderComponent
	{
		float Mass = 70.f;
		glm::vec3 HalfSize = { 1.f, 1.f, 1.f };
		glm::vec3 Offset = { 0.f, 0.f, 0.f };
		float Friction = 0.5f;
		float Restitution = 0.5f;
		HEMotionType MotionType = HEMotionType::Dynamic;
		ColliderMaterial Material{};



		BoxColliderComponent() = default;
		BoxColliderComponent(const BoxColliderComponent&) = default;
		BoxColliderComponent(glm::vec3 half_size) : HalfSize(half_size) {}
		BoxColliderComponent(glm::vec3 half_size, glm::vec3 offset) : HalfSize(half_size), Offset(offset) {}
		BoxColliderComponent(float mass, glm::vec3 half_size, glm::vec3 offset, float friction, float restitution) 
			: Mass(mass), HalfSize(half_size), Offset(offset), Friction(friction), Restitution(restitution) {}
	};

	struct SphereColliderComponent
	{
		float Mass = 70.f;
		float Radius = 1.f;
		float Friction = 0.5f;
		float Restitution = 0.5f;
		glm::vec3 Offset = { 0.f, 0.f, 0.f };
		HEMotionType MotionType = HEMotionType::Dynamic;
		ColliderMaterial Material{};

		SphereColliderComponent() = default;
		SphereColliderComponent(const SphereColliderComponent&) = default;
		SphereColliderComponent(float radius) : Radius(Radius) {}
		SphereColliderComponent(float radius, glm::vec3 offset) : Radius(radius), Offset(offset) {}
		SphereColliderComponent(float mass, float radius, float friction, float restitution, glm::vec3 offset)
			: Mass(mass), Radius(radius), Friction(friction), Restitution(restitution), Offset(offset) {}
	};

	template<typename... Component>
	struct ComponentGroup
	{
	};

	using AllComponents =
		ComponentGroup< IDComponent, ParentIDComponent, TagComponent,
		TransformComponent, MeshComponent, CameraComponent,
		PointLightComponent, DirectionalLightComponent,
		GlobalSoundsComponent, LocalSoundsComponent, ScriptComponent,
		SphereColliderComponent, BoxColliderComponent, CharacterControllerComponent >;

}
