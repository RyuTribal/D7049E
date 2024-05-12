#pragma once
#include "Core/Input.h"
#include "Registry.h"
#include "Renderer/Camera.h"
#include "EntityHandle.h"
#include "SceneSerializer.h"
#include "Assets/AssetMetadata.h"
#include "Renderer/Renderer.h"

namespace Engine {

	class Entity;

	class SceneNode {
	public:
		SceneNode(UUID entity_id) : m_ID(entity_id) {}
		~SceneNode() = default;

		SceneNode(const SceneNode& other)
		{
			m_ID = other.m_ID;
			children.clear();
			for (const auto& child : other.children)
			{
				children.emplace_back(std::make_unique<SceneNode>(*child));  // Deep copy each child
			}
		}

		// Copy assignment operator
		SceneNode& operator=(const SceneNode& other)
		{
			if (this == &other) return *this; // handle self assignment

			m_ID = other.m_ID;
			children.clear();
			for (const auto& child : other.children)
			{
				children.emplace_back(std::make_unique<SceneNode>(*child));  // Deep copy each child
			}
			return *this;
		}

		bool AddChild(UUID parent_id, UUID entity_id) {
			for (auto& child : children)
			{
				if (child->GetID() == entity_id) //Have to check it here to avoid duplicates
				{
					return true;
				}
			}
			if (m_ID == parent_id) {
				children.push_back(CreateScope<SceneNode>(entity_id));
				return true;
			}
			else {
				for (auto& child : children) {
					if (child->AddChild(parent_id, entity_id)) {
						break;
					}
				}
			}
			return false;
		}

		bool RemoveChild(UUID entity_id, SceneNode* parent) {
			if (m_ID == entity_id) {
				parent->AddChildren(std::move(children));
				for (size_t i = 0; i < parent->GetChildren()->size(); i++) {
					if (parent->GetChildren()->at(i)->GetID() == entity_id) {
						parent->GetChildren()->erase(parent->GetChildren()->begin() + i);
					}
				}
				return true;
			}
			else {
				for (auto& child : children) {
					if (child->RemoveChild(entity_id, this)) {
						break;
					}
				}
			}
			return false;
		}

		void SetChildren(std::vector<Scope<SceneNode>> new_children) {
			children = std::move(new_children);
		}

		void AddChildren(std::vector<Scope<SceneNode>> new_children) {
			for (auto &child : new_children) {
				children.push_back(std::move(child));
			}
		}

		UUID& GetID() { return m_ID; }
		void SetID(UUID id) { m_ID = id; }
		std::vector<Scope<SceneNode>>* GetChildren() { return &children; }

	private:
		UUID m_ID;
		std::vector<Scope<SceneNode>> children;
	};


	enum class SceneRunType
	{
		Edit,
		Simulation,
		Runtime
	};


	class Scene : public Asset {
	public:
		static Ref<Scene> CreateScene(std::string name = "A scene");
		static Ref<Scene> LoadScene(AssetHandle handle, const AssetMetadata& metadata);
		static Ref<Scene> Copy(Ref<Scene> original_scene);
		bool SaveScene(const std::filesystem::path& folder_path);
		bool SaveScene();
		Scene(std::string name);
		~Scene();

		void ReloadScene();

		Registry* GetRegistry() { return &m_Registry; }

		EntityHandle* CreateEntity(std::string name, Entity* parent);
		EntityHandle* CreateEntity(std::string name, EntityHandle* parent);
		EntityHandle* CreateEntity(std::string name, UUID* parent);
		EntityHandle* CreateEntity(std::string name, nullptr_t parent);

		EntityHandle* CreateEntityByUUID(UUID id, std::string name, Entity* parent);
		EntityHandle* CreateEntityByUUID(UUID id, std::string name, EntityHandle* parent);
		EntityHandle* CreateEntityByUUID(UUID id, std::string name, UUID* parent);
		EntityHandle* CreateEntityByUUID(UUID id, std::string name, nullptr_t parent);

		void DestroyEntity(EntityHandle* id);
		void DestroyEntity(UUID id);
		void ReparentSceneNode(EntityHandle* id, EntityHandle* new_parent_id);
		void ReparentSceneNode(UUID* id, UUID* new_parent_id);

		void OnRuntimeStart();
		void OnRuntimeStop();
		void OnSimulateStart();
		void OnSimulateStop();

		Camera* GetCurrentCamera();
		void SetCurrentCamera(Camera* camera);

		void UpdateScene();

		UUID& GetId() { return m_ID; }

		SceneNode* GetRootNode() { return &m_RootSceneNode; }

		std::string& GetName() { return m_Name; }

		Entity* GetEntity(const UUID& id);
		Entity* GetEntity(EntityHandle* id);

		template<typename T>
		auto GetAllEntitiesByType()
		{
			return m_Registry.GetComponentRegistry<T>();
		}

		void ForEachEntity(std::function<void(const UUID, const Ref<Entity>)> func) const;

		static AssetType GetStaticType() { return AssetType::Scene; } // Good for templated functions
		AssetType GetType() const { return GetStaticType(); }

		bool IsReloading() { return m_IsReloading; }

		Camera* GetPrimaryEntityCamera();

		const SkyboxSettings& GetSkybox() { return m_Skybox; }
		void SetSkybox(SkyboxSettings& skybox);

	private:

		void FindNodeAndParent(SceneNode* current, UUID id, SceneNode** node, SceneNode** parent);

		void UpdateWorldTransform(SceneNode* node, glm::mat4& parentWorldTransform);
		void UpdateTransforms();
		void DrawSystem();
		void SyncPhysicsTransforms();

	private:
		UUID m_ID = UUID();

		Camera* m_CurrentCamera;

		std::string m_Name;

		Registry m_Registry{};

		SceneNode m_RootSceneNode = SceneNode(0);

		std::unordered_map<UUID, Ref<Entity>> entities;

		bool m_IsReloading = false;

		SceneRunType m_SceneState = SceneRunType::Edit;

		SkyboxSettings m_Skybox;

		friend class Entity;
	};
}
