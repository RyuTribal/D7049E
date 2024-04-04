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

		UUID GetID() { return m_ID; }
		std::vector<Scope<SceneNode>>* GetChildren() { return &children; }

	private:
		UUID m_ID;
		std::vector<Scope<SceneNode>> children;
	};

	class Scene {
	public:
		static std::pair<Ref<Scene>, EntityHandle*> CreateScene(std::string name = "A scene");
		static std::pair<Ref<Scene>, EntityHandle*> CreateScene(Ref<Camera> starting_camera, std::string name = "A scene");
		Scene(std::string name);
		~Scene();

		Registry* GetRegistry() { return &m_Registry; }

		EntityHandle* CreateEntity(std::string name, Entity* parent);
		void DestroyEntity(EntityHandle* id);
		void ReparentSceneNode(EntityHandle* id, EntityHandle* new_parent_id);

		Camera* GetCurrentCamera();

		void SetCurrentCamera(EntityHandle* camera_entity_id) { m_CurrentCamera = entities[camera_entity_id->GetID()]; }

		void UpdateScene();

		UUID GetId() { return m_ID; }

		SceneNode* GetRootNode() { return &m_RootSceneNode; }

		std::string& GetName() { return m_Name; }

		Entity* GetEntity(EntityHandle* id) { return entities[id->GetID()].get(); }

	private:

		void FindNodeAndParent(SceneNode* current, UUID id, SceneNode** node, SceneNode** parent);

		void UpdateWorldTransform(SceneNode* node, glm::mat4& parentWorldTransform);
		void UpdateTransforms();
		void DrawSystem();

	private:
		UUID m_ID = UUID();

		Ref<Entity> m_CurrentCamera;

		std::string m_Name;

		Registry m_Registry{};

		SceneNode m_RootSceneNode = SceneNode(m_ID);

		std::unordered_map<UUID, Ref<Entity>> entities;

		friend class Entity;
	};
}