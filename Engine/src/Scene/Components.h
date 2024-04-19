#pragma once
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include "Renderer/Camera.h"
#include "Renderer/Material.h"
#include "Renderer/Mesh.h"
#include "Lights/PointLight.h"
#include "Sound/Sound.h"

namespace Engine {

	enum ComponentType {
		ID,
		ParentID,
		Tag,
		LocalTransform,
		WorldTransform,
		Transform,
		CameraComp,
		PointLightComp,
		MeshComp,
		MaterialComp,
		SoundComp
	};

	struct Component {
		virtual const ComponentType Type() const = 0;
	};

	struct IDComponent : public Component {
		UUID id;

		IDComponent() = default;
		IDComponent(const IDComponent&) = default;
		IDComponent(UUID new_id) : id(new_id){}

		const ComponentType Type() const override {
			return ComponentType::ID;
		}
	};

	struct ParentIDComponent : public Component {
		UUID id;

		ParentIDComponent() = default;
		ParentIDComponent(const ParentIDComponent&) = default;
		ParentIDComponent(UUID new_id) : id(new_id) {}

		const ComponentType Type() const override {
			return ComponentType::ParentID;
		}
	};

	struct TagComponent : public Component {
		std::string name = "Entity";

		TagComponent() = default;
		TagComponent(const TagComponent&) = default;
		TagComponent(const std::string& name) : name(name) {}

		const ComponentType Type() const override {
			return ComponentType::Tag;
		}
	};

	struct LocalTransformComponent : public Component {
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

		const ComponentType Type() const override {
			return ComponentType::LocalTransform;
		}
	};

	struct WorldTransformComponent : public Component {
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

		const ComponentType Type() const override {
			return ComponentType::LocalTransform;
		}
	};

	struct TransformComponent : public Component {
		WorldTransformComponent world_transform{};
		LocalTransformComponent local_transform{};

		TransformComponent() = default;
		TransformComponent(const TransformComponent&) = default;
		TransformComponent(const glm::vec3& new_translation) : local_transform(new_translation) {}

		const ComponentType Type() const override {
			return ComponentType::Transform;
		}
	};

	struct MeshComponent : public Component {
		Ref<Mesh> mesh;

		MeshComponent() = default;
		MeshComponent(const MeshComponent&) = default;
		MeshComponent(Ref<Mesh> new_mesh) : mesh(new_mesh) {}

		const ComponentType Type() const override {
			return ComponentType::MeshComp;
		}
	};

	struct MaterialComponent : public Component {
		Ref<Material> material;

		MaterialComponent() = default;
		MaterialComponent(const MaterialComponent&) = default;
		MaterialComponent(Ref<Material> new_material) : material(new_material) {}

		const ComponentType Type() const override {
			return ComponentType::MaterialComp;
		}
	};


	struct CameraComponent : public Component {
		Ref<Camera> camera;

		CameraComponent() { camera = CreateRef<Camera>(); };
		CameraComponent(const CameraComponent&) = default;
		CameraComponent(Ref<Camera> new_camera) : camera(new_camera) {}

		const ComponentType Type() const override {
			return ComponentType::CameraComp;
		}
	};

	struct PointLightComponent : public Component {
		Ref<PointLight> light;

		PointLightComponent() { light = CreateRef<PointLight>(); };
		PointLightComponent(const PointLightComponent&) = default;
		PointLightComponent(Ref<PointLight> new_light) : light(new_light) {}

		const ComponentType Type() const override {
			return ComponentType::PointLightComp;
		}
	};

	struct SoundComponent : public Component
	{
		Ref<Sound> sound;

		SoundComponent() { sound = CreateRef<Sound>(); };
		SoundComponent(const SoundComponent&) = default;
		SoundComponent(Ref<Sound> new_sound) : sound(new_sound) {}

		const ComponentType Type() const override
		{
			return ComponentType::SoundComp;
		}
	};

}
