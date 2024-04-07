#include "pch.h"
#include "Scene.h"
#include "Components.h"
#include "Entity.h"
#include <glm/gtx/matrix_decompose.hpp>
#include "Renderer/Renderer.h"

namespace Engine {

	std::pair<Ref<Scene>, EntityHandle*> Scene::CreateScene(std::string name)
	{
		Ref<Camera> camera = CreateRef<Camera>(CameraType::PERSPECTIVE);
		camera->SetIsRotationLocked(true);
		

		return CreateScene(camera, name);
	}

	std::pair<Ref<Scene>, EntityHandle*> Scene::CreateScene(Ref<Camera> starting_camera, std::string name)
	 {
		 Ref<Scene> scene = CreateRef<Scene>(name);
		 EntityHandle* default_camera_entity = scene->CreateEntity("Default scene camera", nullptr);
		 scene->SetCurrentCamera(default_camera_entity);
		 scene->GetEntity(default_camera_entity)->AddComponent<CameraComponent>(CameraComponent(starting_camera));
		 Renderer::Get()->SetCamera(starting_camera.get());
		 return { scene, default_camera_entity };
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
	EntityHandle* Scene::CreateEntity(std::string name, Entity* parent) {
		return CreateEntity(name, parent->GetHandle());
	}

	EntityHandle* Scene::CreateEntity(std::string name, EntityHandle* parent)
	{
		return CreateEntity(name, &parent->GetID());
	}

	EntityHandle* Scene::CreateEntity(std::string name, UUID* parent)
	{
		UUID new_id = UUID();
		m_RootSceneNode.AddChild(parent == nullptr ? m_ID : *parent, new_id);

		m_Registry.Add<IDComponent>(new_id, IDComponent(new_id));
		m_Registry.Add<ParentIDComponent>(new_id, ParentIDComponent(parent == nullptr ? m_ID : *parent));
		m_Registry.Add<TagComponent>(new_id, TagComponent(name));
		m_Registry.Add<TransformComponent>(new_id, TransformComponent());

		Ref<Entity> new_entity = CreateRef<Entity>(new_id, this);

		entities[new_id] = new_entity;

		return new_entity->GetHandle();
	}

	EntityHandle* Scene::CreateEntity(std::string name, nullptr_t parent)
	{
		return CreateEntity(name, &m_ID);
	}

	void Scene::DestroyEntity(EntityHandle* id)
	{
		DestroyEntity(id->GetID());
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

	void Scene::ReparentSceneNode(EntityHandle* id, EntityHandle* new_parent_id) {
		ReparentSceneNode(&id->GetID(), &new_parent_id->GetID());
	}

	void Scene::ReparentSceneNode(UUID* id, UUID* new_parent_id)
	{
		m_RootSceneNode.RemoveChild(*id, &m_RootSceneNode);
		m_RootSceneNode.AddChild(*new_parent_id, *id);
	}


	Camera* Scene::GetCurrentCamera(){
		return m_CurrentCamera->GetComponent<CameraComponent>()->camera.get();
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

				if (m_Registry.Get<CameraComponent>(node->GetID())->camera->IsRotationLocked()) {
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
				value.mesh->SetTransform(m_Registry.Get<TransformComponent>(id)->world_transform.mat4());
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