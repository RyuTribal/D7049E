#pragma once
#include "Registry.h"
#include "Renderer/Camera.h"
#include "EntityHandle.h"

namespace Engine {

	class Entity;

	class SceneNode {
	public:
		SceneNode(UUID entity_id) : m_ID(entity_id) {}
		~SceneNode() = default;

		bool AddChild(UUID parent_id, UUID entity_id) {
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
		std::vector<Scope<SceneNode>>* GetChildren() { return &children; }

	private:
		UUID m_ID;
		std::vector<Scope<SceneNode>> children;
	};

	class Scene {
	public:
		static Ref<Scene> CreateScene(std::string name = "A scene");
		Scene(std::string name);
		~Scene();

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

		Camera* GetCurrentCamera();

		void SetCurrentCamera(Ref<Camera> camera);

		void UpdateScene();

		UUID& GetId() { return m_ID; }

		SceneNode* GetRootNode() { return &m_RootSceneNode; }

		std::string& GetName() { return m_Name; }

		Entity* GetEntity(UUID& id);
		Entity* GetEntity(EntityHandle* id);

		void ForEachEntity(std::function<void(const UUID, const Ref<Entity>)> func) const;

	private:

		void FindNodeAndParent(SceneNode* current, UUID id, SceneNode** node, SceneNode** parent);

		void UpdateWorldTransform(SceneNode* node, glm::mat4& parentWorldTransform);
		void UpdateTransforms();
		void DrawSystem();

	private:
		UUID m_ID = UUID();

		Ref<Camera> m_CurrentCamera;

		std::string m_Name;

		Registry m_Registry{};

		SceneNode m_RootSceneNode = SceneNode(m_ID);

		std::unordered_map<UUID, Ref<Entity>> entities;

		friend class Entity;
	};
}
