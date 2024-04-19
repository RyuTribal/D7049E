#pragma once
#include "Scene/Scene.h"

namespace YAML {
	class Emitter;
	class Node;
}

namespace Engine {
	class SceneSerializer
	{
	public:
		static void Serializer(const std::string& directory, Ref<Scene> scene);
		static Ref<Scene> Deserializer(const std::string& filepath);

	private:
		static void TraverseTree(YAML::Emitter& out, SceneNode* scene_node, Ref<Scene> scene);
		static void SerializeEntity(YAML::Emitter& out, Entity* entity);
		static void DeserializeEntity(YAML::Node entity_node, Ref<Scene> scene, UUID* parent_entity_id);
	};
}
