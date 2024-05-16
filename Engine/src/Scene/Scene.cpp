#include "pch.h"
#include "Scene.h"
#include "Components.h"
#include "Entity.h"
#include <glm/gtx/matrix_decompose.hpp>
#include "Renderer/Renderer.h"
#include "Assets/AssetManager.h"
#include "Core/Application.h"
#include "Physics/PhysicsEngine.h"
#include "Sound/SoundEngine.h"

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
		// Temp fix
		auto entity = GetEntity(id);
		if (entity->HasComponent<GlobalSoundsComponent>())
		{
			auto sounds = entity->GetComponent<GlobalSoundsComponent>();
			for (auto sound : sounds->Sounds)
			{
				sound->StopSound();
			}
		}

		if (entity->HasComponent<LocalSoundsComponent>())
		{
			auto sounds = entity->GetComponent<LocalSoundsComponent>();
			for (auto sound : sounds->Sounds)
			{
				sound->StopSound();
			}
		}

		m_RootSceneNode.RemoveChild(id, &m_RootSceneNode);
		m_Registry.RemoveAllFromEntity(id);
		entities.erase(id);
	}

	Entity* Scene::GetEntity(const UUID& id)
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

	static void CopySceneEntities(SceneNode* source, Ref<Scene> target)
	{
		for (size_t i = 0; i < source->GetChildren()->size(); i++)
		{
			target->CreateEntityByUUID(source->GetChildren()->at(i)->GetID(), "", &source->GetID());
			CopySceneEntities(source->GetChildren()->at(i).get(), target);
		}
	}

	template<typename ComponentType>
	static void CopyComponent(Registry& source, Registry& target)
	{
		auto component_reg = source.GetComponentRegistry<ComponentType>();
		if (!component_reg)
		{
			return;
		}
		for (auto& [id, component] : *component_reg)
		{
			target.Add<ComponentType>(id, component);
		}
	}

	Ref<Scene> Scene::Copy(Ref<Scene> original_scene)
	{
		Ref<Scene> new_scene = CreateRef<Scene>(original_scene->m_Name);
		
		CopySceneEntities(&original_scene->m_RootSceneNode, new_scene);

		CopyComponent<TagComponent>(original_scene->m_Registry, new_scene->m_Registry);
		CopyComponent<TransformComponent>(original_scene->m_Registry, new_scene->m_Registry);
		CopyComponent<MeshComponent>(original_scene->m_Registry, new_scene->m_Registry);
		CopyComponent<CameraComponent>(original_scene->m_Registry, new_scene->m_Registry);
		CopyComponent<PointLightComponent>(original_scene->m_Registry, new_scene->m_Registry);
		CopyComponent<DirectionalLightComponent>(original_scene->m_Registry, new_scene->m_Registry);
		CopyComponent<GlobalSoundsComponent>(original_scene->m_Registry, new_scene->m_Registry);
		CopyComponent<LocalSoundsComponent>(original_scene->m_Registry, new_scene->m_Registry);
		CopyComponent<ScriptComponent>(original_scene->m_Registry, new_scene->m_Registry);
		CopyComponent<CharacterControllerComponent>(original_scene->m_Registry, new_scene->m_Registry);
		CopyComponent<BoxColliderComponent>(original_scene->m_Registry, new_scene->m_Registry);
		CopyComponent<SphereColliderComponent>(original_scene->m_Registry, new_scene->m_Registry);

		return new_scene;
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

	void Scene::OnRuntimeStart()
	{
		HVE_CORE_ASSERT(!Application::Get().GetProps().NoScripting, "The scene requires you to use scripting, which is currently set to false!");
		ScriptEngine::OnRuntimeStart(this);
		SoundEngine::StopAll();

		m_SceneState = SceneRunType::Runtime;

		// Init all script entities
		auto script_registry = m_Registry.GetComponentRegistry<ScriptComponent>();
		if (script_registry)
		{
			for (auto& [entity_id, script_component] : *script_registry)
			{
				ScriptEngine::OnCreateEntityClass(GetEntity(entity_id));
			}
		}

		PhysicsEngine::Get()->CreateScene(this, 10);
		PhysicsEngine::Get()->OnRuntimeStart(1, 1);

		std::set<UUID> finished_assets = std::set<UUID>();

		auto character_controllers = m_Registry.GetComponentRegistry<CharacterControllerComponent>();
		if (character_controllers)
		{
			for (auto& [entity_id, character_controller] : *character_controllers)
			{
				if (finished_assets.contains(entity_id))
					continue;
				PhysicsEngine::Get()->GetCurrentScene()->CreateBody(GetEntity(entity_id));
				finished_assets.insert(entity_id);
			}
		}

		auto box_colliders = m_Registry.GetComponentRegistry<BoxColliderComponent>();
		if (box_colliders)
		{
			for (auto& [entity_id, box_collider] : *box_colliders)
			{
				if (finished_assets.contains(entity_id))
					continue;
				PhysicsEngine::Get()->GetCurrentScene()->CreateBody(GetEntity(entity_id));
				finished_assets.insert(entity_id);
			}
		}

		auto sphere_colliders = m_Registry.GetComponentRegistry<SphereColliderComponent>();
		if (sphere_colliders)
		{
			for (auto& [entity_id, sphere_collider] : *sphere_colliders)
			{
				if (finished_assets.contains(entity_id))
					continue;
				PhysicsEngine::Get()->GetCurrentScene()->CreateBody(GetEntity(entity_id));
				finished_assets.insert(entity_id);
			}
		}
	}

	void Scene::OnRuntimeStop()
	{
		HVE_CORE_ASSERT(!Application::Get().GetProps().NoScripting, "The scene requires you to use scripting, which is currently set to false!");
		ScriptEngine::OnRuntimeStop();
		PhysicsEngine::Get()->OnRuntimeStop();
		SoundEngine::StopAll();
		m_SceneState = SceneRunType::Edit;
	}

	void Scene::OnSimulateStart()
	{
	}

	void Scene::OnSimulateStop()
	{
	}

	Camera* Scene::GetCurrentCamera(){
		return m_CurrentCamera;
	}
	void Scene::SetCurrentCamera(Camera* camera)
	{
		m_CurrentCamera = camera;
	}

	Camera* Scene::GetPrimaryEntityCamera()
	{
		if (m_Registry.GetComponentRegistry<CameraComponent>() != nullptr)
		{
			for (auto& [id, value] : *m_Registry.GetComponentRegistry<CameraComponent>())
			{
				if (value.IsPrimary)
				{
					return &value.camera;
				}
			}
		}

		return nullptr;
	}

	void Scene::SetSkybox(SkyboxSettings& skybox)
	{
		m_Skybox = skybox;
		Renderer::Get()->SetSkybox(m_Skybox);
	}

	void Scene::UpdateScene()
	{
		SetCurrentCamera(Renderer::Get()->GetCamera());
		m_IsReloading = false;

		if (m_SceneState == SceneRunType::Runtime)
		{
			ScriptEngine::OnUpdate(Application::Get().GetFrameData().DeltaTime);

			auto box_colliders = m_Registry.GetComponentRegistry<BoxColliderComponent>();
			if (box_colliders)
			{
				for (auto& [entity_id, box_collider] : *box_colliders)
				{
					glm::vec3 entity_world_translation = GetEntity(entity_id)->GetComponent<TransformComponent>()->world_transform.translation;
					PhysicsEngine::Get()->GetCurrentScene()->SetPosition(entity_id, entity_world_translation, true);
				}

				PhysicsEngine::Get()->Step(Application::Get().GetFrameData().DeltaTime);
			}
		}

		UpdateTransforms();

		if (m_SceneState == SceneRunType::Runtime)
		{
			SyncPhysicsTransforms();
		}

		SoundEngine::SetListenerPosition(GetCurrentCamera()->CalculatePosition());

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
				m_Registry.Get<CameraComponent>(node->GetID())->camera.SetPosition(translation);

				if (m_Registry.Get<CameraComponent>(node->GetID())->camera.IsRotationLocked()) {
					m_Registry.Get<CameraComponent>(node->GetID())->camera.SetRotation(glm::vec2(-eulerAngles.x, -eulerAngles.y));
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
			for (auto& [id, value] : *m_Registry.GetComponentRegistry<PointLightComponent>()) {
				if (m_Registry.Get<TransformComponent>(id) != nullptr) {
					value.light.SetPosition(m_Registry.Get<TransformComponent>(id)->world_transform.translation);
				}
				Renderer::Get()->SubmitPointLight(&value.light);
			}
		}

		if (m_Registry.GetComponentRegistry<DirectionalLightComponent>() != nullptr)
		{
			for (auto& [id, value] : *m_Registry.GetComponentRegistry<DirectionalLightComponent>())
			{
				Renderer::Get()->SubmitDirectionalLight(&value.light);
			}
		}
		Renderer::Get()->BeginDrawing();
	}

	void Scene::SyncPhysicsTransforms()
	{
		auto box_colliders = m_Registry.GetComponentRegistry<BoxColliderComponent>();
		if (box_colliders)
		{
			for (auto& [entity_id, box_collider] : *box_colliders)
			{
				auto transform = GetEntity(entity_id)->GetComponent<TransformComponent>();
				glm::mat4 collider_transform = PhysicsEngine::Get()->GetCurrentScene()->GetTransform(entity_id);
				glm::mat4 worldTransform = collider_transform;

				glm::vec3 scale;
				glm::quat rotation;
				glm::vec3 translation;
				glm::vec3 skew;
				glm::vec4 perspective;

				glm::decompose(worldTransform, scale, rotation, translation, skew, perspective);

				glm::vec3 eulerAngles = glm::eulerAngles(rotation);

				transform->world_transform.translation = translation;
				transform->world_transform.rotation = eulerAngles;

				// Update the camera if present
				auto camera_component = m_Registry.Get<CameraComponent>(entity_id);
				if (camera_component)
				{
					camera_component->camera.SetPosition(translation);

					if (camera_component->camera.IsRotationLocked())
					{
						camera_component->camera.SetRotationAroundFocalPoint(glm::vec2(-eulerAngles.x, -eulerAngles.y));
					}
				}
			}
		}


		auto sphere_colliders = m_Registry.GetComponentRegistry<SphereColliderComponent>();
		if (sphere_colliders)
		{
			for (auto& [entity_id, sphere_collider] : *sphere_colliders)
			{
				auto transform = GetEntity(entity_id)->GetComponent<TransformComponent>();
				glm::mat4 collider_transform = PhysicsEngine::Get()->GetCurrentScene()->GetTransform(entity_id);
				glm::mat4 worldTransform = collider_transform;

				glm::vec3 scale;
				glm::quat rotation;
				glm::vec3 translation;
				glm::vec3 skew;
				glm::vec4 perspective;

				glm::decompose(worldTransform, scale, rotation, translation, skew, perspective);

				glm::vec3 eulerAngles = glm::eulerAngles(rotation);

				transform->world_transform.translation = translation;
				transform->world_transform.rotation = eulerAngles;

				// Update the camera if present
				auto camera_component = m_Registry.Get<CameraComponent>(entity_id);
				if (camera_component)
				{
					camera_component->camera.SetPosition(translation);

					if (camera_component->camera.IsRotationLocked())
					{
						camera_component->camera.SetRotationAroundFocalPoint(glm::vec2(-eulerAngles.x, -eulerAngles.y));
					}
				}
			}
		}

		auto character_controllers= m_Registry.GetComponentRegistry<CharacterControllerComponent>();
		if (character_controllers)
		{
			for (auto& [entity_id, character_controller] : *character_controllers)
			{
				auto transform = GetEntity(entity_id)->GetComponent<TransformComponent>();


				glm::vec3 eulerAngles = PhysicsEngine::Get()->GetCurrentScene()->GetRotation(entity_id);

				transform->world_transform.translation = PhysicsEngine::Get()->GetCurrentScene()->GetPosition(entity_id);
				transform->world_transform.rotation = PhysicsEngine::Get()->GetCurrentScene()->GetRotation(entity_id);

				// Update the camera if present
				auto camera_component = m_Registry.Get<CameraComponent>(entity_id);
				if (camera_component)
				{
					camera_component->camera.SetPosition(transform->world_transform.translation);

					if (camera_component->camera.IsRotationLocked())
					{
						camera_component->camera.SetRotationAroundFocalPoint(glm::vec2(-eulerAngles.x, -eulerAngles.y));
					}
				}
			}
		}
	}

}
