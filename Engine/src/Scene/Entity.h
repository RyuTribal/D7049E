#pragma once

#include "Scene.h"
#include "EntityHandle.h"

namespace Engine {

	class Entity {
	public:
		Entity() : m_Name("Entity"), m_Scene(nullptr) {}
		Entity(std::string& name, Scene* scene_ptr) : m_Name(name), m_Scene(scene_ptr) {
			m_Handle = EntityHandle(UUID());
		}
		Entity(UUID id, std::string& name, Scene* scene_ptr) : m_Handle(EntityHandle(id)), m_Name(name), m_Scene(scene_ptr) {}
		~Entity() = default;


		template<typename Type>
		void AddComponent(Type component) {
			if (m_Scene != nullptr) {
				m_Scene->GetRegistry()->Add<Type>(m_Handle.GetID(), component);
			}
		}

		template<typename Type>
		void RemoveComponent() {
			if (m_Scene != nullptr) {
				m_Scene->GetRegistry()->Remove<Type>(m_Handle.GetID());
			}
		}

		template<typename Type>
		Type* GetComponent() {
			if (m_Scene != nullptr) {
				return m_Scene->GetRegistry()->Get<Type>(m_Handle.GetID());
			}
			return nullptr;
		}

		template<typename Type>
		bool HasComponent() {
			if (m_Scene != nullptr) {
				return m_Scene->GetRegistry()->Get<Type>(m_Handle.GetID()) != nullptr ? true : false;
			}
			return false;
		}

		std::string& GetName() { return m_Name; }
		UUID GetID() { return m_Handle.GetID(); }
		EntityHandle* GetHandle() { return &m_Handle; }

	private:
		EntityHandle m_Handle;
		std::string m_Name;

		Scene* m_Scene;
	};
}