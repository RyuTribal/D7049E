#include "pch.h"
#include "Scene.h"
#include "Components.h"
#include "Entity.h"
#include <glm/gtx/matrix_decompose.hpp>
#include "Renderer/Renderer.h"

namespace Engine {

	std::pair<Ref<Entity>, Ref<Scene>> Scene::CreateScene(std::string name)
	{
		Ref<Scene> scene = CreateRef<Scene>(name);
		Ref<Entity> default_camera_entity = scene->CreateEntity("Default scene camera", nullptr);
		scene->SetCurrentCamera(default_camera_entity->GetID());
		Ref<Camera> camera = CreateRef<Camera>(CameraType::PERSPECTIVE);
		default_camera_entity->AddComponent<CameraComponent>(CameraComponent(camera));
		Renderer::Get()->SetCamera(camera.get());

		return {default_camera_entity, scene};
	}


	Scene::Scene(std::string name)
		: m_Name(name)
	{
	}

	Scene::~Scene() {
		for (auto [key, entity] : entities) {
			m_Registry.RemoveAllFromEntity(key);
		}
	}
	Ref<Entity> Scene::CreateEntity(std::string name, Entity* parent) {
		
		UUID new_id = UUID();
		m_RootSceneNode.AddChild(parent == nullptr ? m_ID : parent->GetID(), new_id);

		m_Registry.Add<ParentIDComponent>(new_id, ParentIDComponent(parent == nullptr ? m_ID : parent->GetID()));

		Ref<Entity> new_entity = CreateRef<Entity>(new_id, name, this);

		entities[new_id] = new_entity->GetHandle();

		return new_entity;
	}

	void Scene::DestroyEntity(UUID id)
	{
		m_RootSceneNode.RemoveChild(id, &m_RootSceneNode);
		m_Registry.RemoveAllFromEntity(id);
		entities.erase(id);
	}

	void Scene::FindNodeAndParent(SceneNode* current, UUID id, SceneNode** node, SceneNode** parent) {
		for (auto& child : *current->GetChildren()) {
			if (child->GetID() == id) {
				*node = child.get();
				*parent = current;
				return;
			}
			FindNodeAndParent(child.get(), id, node, parent);
		}
	}

	void Scene::ReparentSceneNode(UUID id, UUID new_parent_id) {
		m_RootSceneNode.RemoveChild(id, &m_RootSceneNode);
		m_RootSceneNode.AddChild(new_parent_id, id);
		HVE_CORE_WARN(m_RootSceneNode.GetChildren()->size());
	}


	Camera* Scene::GetCurrentCamera(){
		return m_Registry.Get<CameraComponent>(m_CurrentCamera)->camera.get();
	}
	void Scene::UpdateScene()
	{
		UpdateTransforms();
		DrawSystem();
	}

	void Scene::UpdateWorldTransform(SceneNode* node, glm::mat4& parentWorldTransform)
	{
		if (!node) return;
		auto transform = m_Registry.Get<TransformComponent>(node->GetID());
		if (transform) {
			glm::mat4 worldTransform = parentWorldTransform * transform->local_transform.mat4();
			glm::vec3 scale;
			glm::quat rotation;
			glm::vec3 translation;
			glm::vec3 skew;
			glm::vec4 perspective;

			glm::decompose(worldTransform, scale, rotation, translation, skew, perspective);

			glm::vec3 eulerAngles = glm::eulerAngles(rotation);

			transform->world_transform.translation = translation;
			transform->world_transform.rotation = eulerAngles;
			transform->world_transform.scale = scale;

			if (m_Registry.Get<CameraComponent>(node->GetID()) != nullptr) {
				m_Registry.Get<CameraComponent>(node->GetID())->camera->SetPosition(translation);

				// Might remove this, kinda dumb to lock rotation to an object
				if (m_Registry.Get<CameraComponent>(node->GetID())->lock_camera) {
					m_Registry.Get<CameraComponent>(node->GetID())->camera->SetPitch(-transform->local_transform.rotation.x);
					m_Registry.Get<CameraComponent>(node->GetID())->camera->SetYaw(-transform->local_transform.rotation.y);
				}
			}

			for (auto& child : *node->GetChildren()) {
				UpdateWorldTransform(child.get(), worldTransform);
			}
			
		}
		else {
			for (auto& child : *node->GetChildren()) {
				UpdateWorldTransform(child.get(), parentWorldTransform);
			}
		}
	}

	void Scene::UpdateTransforms()
	{
		glm::mat4 identityMatrix = glm::mat4(1.0f);

		for (auto& child : *m_RootSceneNode.GetChildren()) {
			UpdateWorldTransform(child.get(), identityMatrix);
		}
	}

	void Scene::DrawSystem()
	{
		if (m_Registry.GetComponentRegistry<MeshComponent>() != nullptr) {
			for (const auto& [id, value] : *m_Registry.GetComponentRegistry<MeshComponent>()) {
				if (m_Registry.Get<TransformComponent>(id) != nullptr) {
					value.mesh->SetTransform(m_Registry.Get<TransformComponent>(id)->world_transform.mat4());
				}
				else {
					value.mesh->SetTransform(glm::mat4(0.f)); // just a default one, adds no transform
				}
				Renderer::Get()->SubmitObject(value.mesh.get(), m_Registry.Get<MaterialComponent>(id)->material.get());
			}
		}
		if (m_Registry.GetComponentRegistry<PointLightComponent>() != nullptr) {
			for (const auto& [id, value] : *m_Registry.GetComponentRegistry<PointLightComponent>()) {
				if (m_Registry.Get<TransformComponent>(id) != nullptr) {
					value.light->SetPosition(m_Registry.Get<TransformComponent>(id)->world_transform.translation);
				}
				Renderer::Get()->SubmitPointLight(value.light.get());
			}
		}

		Renderer::Get()->BeginDrawing();
	}


}