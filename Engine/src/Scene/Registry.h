#include <unordered_map>
#include <memory>
#include <typeindex>
#include <type_traits>
#include "Core/UUID.h"
#include "Core/Log.h"

namespace Engine {

    class IComponentContainer {
    public:
        virtual ~IComponentContainer() = default;
        virtual void Remove(UUID entityId) = 0;
    };

    template<typename T>
    class ComponentContainer : public IComponentContainer {
    public:
        void Add(UUID entityId, T component) {
            componentMap[entityId] = component;
        }

        T* Get(UUID entityId) {
            auto it = componentMap.find(entityId);
            if (it != componentMap.end()) {
                return &it->second;
            }
            return nullptr;
        }

        void Remove(UUID entityId) override {
            auto it = componentMap.find(entityId);
            if (it != componentMap.end()) {
                componentMap.erase(entityId);
            }
        }

        std::unordered_map<UUID, T>* map() { return &componentMap; }

    private:
        std::unordered_map<UUID, T> componentMap{};
    };

    class Registry {
    public:
        template<typename T>
        void Add(UUID entityId, T component) {
            const std::type_index typeIndex = std::type_index(typeid(T));
            if (components.find(typeIndex) == components.end()) {
                components[typeIndex] = std::make_shared<ComponentContainer<T>>();
            }
            std::static_pointer_cast<ComponentContainer<T>>(components[typeIndex])->Add(entityId, std::move(component));
        }

        template<typename T>
        T* Get(UUID entityId) {
            const std::type_index typeIndex = std::type_index(typeid(T));
            auto it = components.find(typeIndex);
            if (it != components.end()) {
                return std::static_pointer_cast<ComponentContainer<T>>(it->second)->Get(entityId);
            }
            return nullptr;
        }

        template<typename T>
        void Remove(UUID entityId) {
            const std::type_index typeIndex = std::type_index(typeid(T));
            auto it = components.find(typeIndex);
            if (it != components.end()) {
                std::static_pointer_cast<ComponentContainer<T>>(it->second)->Remove(entityId);
            }
        }

        void RemoveAllFromEntity(UUID entityId) {
            for (auto& [typeIndex, container] : components) {
                container->Remove(entityId);
            }
        }

        template<typename T>
        std::unordered_map<UUID, T>* GetComponentRegistry() {
            const std::type_index typeIndex = std::type_index(typeid(T));
            auto it = components.find(typeIndex);
            if (it != components.end()) {
                return std::static_pointer_cast<ComponentContainer<T>>(it->second)->map();
            }
            return nullptr;
        }

    private:
        std::unordered_map<std::type_index, std::shared_ptr<IComponentContainer>> components;
    };
}
