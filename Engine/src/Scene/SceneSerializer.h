#pragma once

namespace YAML {
	class Emitter;
	class Node;
}

namespace Engine {
	class SceneNode;
	class Scene;
	class Entity;

	class SceneSerializer
	{
	public:
		static void Serializer(const std::filesystem::path& directory, Scene* scene);
		static Ref<Scene> Deserializer(const std::filesystem::path& filepath);

	private:
		static void TraverseTree(YAML::Emitter& out, SceneNode* scene_node, Scene* scene);
		static void SerializeEntity(YAML::Emitter& out, Entity* entity);
		static void DeserializeEntity(YAML::Node entity_node, Ref<Scene> scene, UUID* parent_entity_id);
	};
}
