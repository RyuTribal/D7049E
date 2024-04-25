#include "pch.h"
#include "Scene.h"
#include "Components.h"
#include "Entity.h"
#include <glm/gtx/matrix_decompose.hpp>
#include "Renderer/Renderer.h"
#include "Assets/AssetManager.h"

namespace Engine {

	Ref<Scene> Scene::CreateScene(std::string name)
	{
		return CreateRef<Scene>(name);
	}

	Ref<Scene> Scene::LoadScene(AssetHandle handle, const AssetMetadata& metadata)
	{
		return SceneSerializer::Deserializer(metadata.FilePath);
	}

	void Scene::ReloadScene()
	{
		SaveScene();
		AssetMetadata metadata = AssetManager::GetMetadata(Handle);
		Ref<Scene> new_scene = SceneSerializer::Deserializer(metadata.FilePath);

		this->m_Name = std::move(new_scene->m_Name);
		this->m_Registry = std::move(new_scene->m_Registry);
		this->m_RootSceneNode = std::move(new_scene->m_RootSceneNode);
		this->entities = std::move(new_scene->entities);
		this->m_IsReloading = true;

		for (auto& entity : entities)
		{
			entity.second->ChangeScene(this);
		}
	}

	bool Scene::SaveScene(const std::filesystem::path& folder_path)
	{
		SceneSerializer::Serializer(folder_path, this);

		return true; // Handle not saving correctly
	}

	bool Scene::SaveScene()
	{
		SaveScene(AssetManager::GetMetadata(Handle).FilePath);
		return true;
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
		UUID* id = &parent->GetID();
		return CreateEntity(name, id);
	}

	EntityHandle* Scene::CreateEntity(std::string name, UUID* parent)
	{
		UUID new_id = UUID();
		return CreateEntityByUUID(new_id, name, parent);
	}

	EntityHandle* Scene::CreateEntity(std::string name, nullptr_t parent)
	{
		UUID root_id = 0;
		return CreateEntity(name, &root_id);
	}

	EntityHandle* Scene::CreateEntityByUUID(UUID id, std::string name, Entity* parent)
	{
		UUID* parent_id = &parent->GetID();
		return CreateEntityByUUID(id, name, parent_id);
	}

	EntityHandle* Scene::CreateEntityByUUID(UUID id, std::string name, EntityHandle* parent)
	{
		UUID* parent_id = &parent->GetID();
		return CreateEntityByUUID(id, name, parent_id);
	}

	EntityHandle* Scene::CreateEntityByUUID(UUID id, std::string name, nullptr_t parent)
	{
		UUID root_id = 0;
		return CreateEntityByUUID(id, name, &root_id);
	}

	EntityHandle* Scene::CreateEntityByUUID(UUID id, std::string name, UUID* parent)
	{
		UUID root_id = 0;
		m_RootSceneNode.AddChild(parent == nullptr ? root_id : *parent, id);

		m_Registry.Add<IDComponent>(id, IDComponent(id));
		m_Registry.Add<ParentIDComponent>(id, ParentIDComponent(parent == nullptr ? root_id : *parent));
		m_Registry.Add<TagComponent>(id, TagComponent(name));
		m_Registry.Add<TransformComponent>(id, TransformComponent());

		Ref<Entity> new_entity = CreateRef<Entity>(id, this);

		entities[id] = new_entity;

		return new_entity->GetHandle();
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

	Entity* Scene::GetEntity(UUID& id)
	{
		auto it = entities.find(id);
		if (it != entities.end()) {
			return it->second.get();
		}
		return nullptr;
	}

	Entity* Scene::GetEntity(EntityHandle* id)
	{
		return GetEntity(id->GetID());
	}

	void Scene::ForEachEntity(std::function<void(const UUID, const Ref<Entity>)> func) const
	{
		for (const auto& [key, value] : entities)
		{
			func(key, value);
		}
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
		UUID* ent_id = &id->GetID();
		UUID* parent_id = &new_parent_id->GetID();
		ReparentSceneNode(ent_id, parent_id);
	}

	void Scene::ReparentSceneNode(UUID* id, UUID* new_parent_id)
	{
		m_RootSceneNode.RemoveChild(*id, &m_RootSceneNode);
		m_RootSceneNode.AddChild(*new_parent_id, *id);
	}

	Camera* Scene::GetCurrentCamera(){
		return m_CurrentCamera.get();
	}
	void Scene::SetCurrentCamera(Ref<Camera> camera)
	{
		m_CurrentCamera = camera;
	}
	void Scene::UpdateScene()
	{
		m_IsReloading = false;
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
		HVE_PROFILE_FUNC();
		glm::mat4 identityMatrix = glm::mat4(1.0f);

		for (auto& child : *m_RootSceneNode.GetChildren()) {
			UpdateWorldTransform(child.get(), identityMatrix);
		}
	}

	void Scene::DrawSystem()
	{
		HVE_PROFILE_FUNC();
		if (m_Registry.GetComponentRegistry<MeshComponent>() != nullptr) {
			for (const auto& [id, value] : *m_Registry.GetComponentRegistry<MeshComponent>()) {
				if (value.mesh != nullptr)
				{
					value.mesh->SetTransform(m_Registry.Get<TransformComponent>(id)->world_transform.mat4());
					Renderer::Get()->SubmitObject(value.mesh);
				}
			}
		}
		if (m_Registry.GetComponentRegistry<PointLightComponent>() != nullptr) {
			for (const auto& [id, value] : *m_Registry.GetComponentRegistry<PointLightComponent>()) {
				if (m_Registry.Get<TransformComponent>(id) != nullptr) {
					value.light->SetPosition(m_Registry.Get<TransformComponent>(id)->world_transform.translation);
				}
				Renderer::Get()->SubmitPointLight(value.light);
			}
		}

		if (m_Registry.GetComponentRegistry<DirectionalLightComponent>() != nullptr)
		{
			for (const auto& [id, value] : *m_Registry.GetComponentRegistry<DirectionalLightComponent>())
			{
				Renderer::Get()->SubmitDirectionalLight(value.light);
			}
		}
		Renderer::Get()->BeginDrawing();
	}


}
