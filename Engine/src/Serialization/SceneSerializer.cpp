#include "pch.h"
#include "SceneSerializer.h"
#include "YAMLSerializer.h"
#include "Scene/Entity.h"
#include "Scene/Components.h"
#include "Assets/AssetTypes.h"

namespace Engine{
	void SceneSerializer::Serializer(const std::string& directory, Ref<Scene> scene)
	{
		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "Scene" << YAML::Value << scene->GetName();

		auto rootNode = scene->GetRootNode();
		if (rootNode)
		{
			out << YAML::Key << "Entities";
			out << YAML::BeginSeq;
			auto children = rootNode->GetChildren(); // We do this here because we dont want to serialize the root node
			for (size_t i = 0; i < children->size(); i++)
			{
				TraverseTree(out, children->at(i).get(), scene);
			}
			out << YAML::EndSeq;
		}

		out << YAML::EndMap;
		
		std::filesystem::path dirPath(directory);
		if (!std::filesystem::exists(dirPath))
		{
			std::filesystem::create_directories(dirPath);
		}

		// Full path for the file
		std::string filepath = directory + scene->GetName() + Utils::GetFileEndings(AssetType::SceneAsset);
		std::ofstream fout(filepath);
		HVE_CORE_ASSERT(fout, "Failed to open file for writing: {0}", filepath);

		fout << out.c_str();
		fout.close();
		HVE_CORE_ASSERT(!fout.fail(), "Failed to write data to file: {0}", filepath);
		HVE_CORE_TRACE_TAG("Scene Serializer", "Scene saved successfully to: {0}", filepath);
		
	}
	Ref<Scene> SceneSerializer::Deserializer(const std::string& filepath)
	{
		return nullptr;
	}

	void SceneSerializer::TraverseTree(YAML::Emitter& out, SceneNode* scene_node, Ref<Scene> scene)
	{
		out << YAML::BeginMap;
		SerializeEntity(out, scene->GetEntity(scene_node->GetID()));

		if (!scene_node->GetChildren()->empty())
		{
			out << YAML::Key << "Children";
			out << YAML::BeginSeq;
			for (size_t i = 0; i < scene_node->GetChildren()->size(); i++)
			{
				TraverseTree(out, scene_node->GetChildren()->at(i).get(), scene);
			}
			out << YAML::EndSeq;
		}
		out << YAML::EndMap;

	}
	void SceneSerializer::SerializeEntity(YAML::Emitter& out, Entity* entity)
	{
		out << YAML::Key << "Entity" << YAML::Value << entity->GetID();

		if (entity->HasComponent<TagComponent>())
		{
			out << YAML::Key << "Tag" << YAML::Value << entity->GetComponent<TagComponent>()->name;
		}


		if (entity->HasComponent<TransformComponent>())
		{
			auto transform = entity->GetComponent<TransformComponent>();
			out << YAML::Key << "LocalTransformComponent";
			out << YAML::BeginMap;
			out << YAML::Key << "Position" << YAML::Value << transform->local_transform.translation;
			out << YAML::Key << "Rotation" << YAML::Value << transform->local_transform.rotation;
			out << YAML::Key << "Scale" << YAML::Value << transform->local_transform.scale;
			out << YAML::EndMap;

			out << YAML::Key << "WorldTransformComponent";
			out << YAML::BeginMap;
			out << YAML::Key << "Position" << YAML::Value << transform->world_transform.translation;
			out << YAML::Key << "Rotation" << YAML::Value << transform->world_transform.rotation;
			out << YAML::Key << "Scale" << YAML::Value << transform->world_transform.scale;
			out << YAML::EndMap;
		}

		if (entity->HasComponent<CameraComponent>())
		{
			auto camera = entity->GetComponent<CameraComponent>()->camera;
			out << YAML::Key << "Camera";
			out << YAML::BeginMap;
			out << YAML::Key << "ProjectionType" << YAML::Value << (int)camera->GetType();
			out << YAML::Key << "PerspectiveFOV" << YAML::Value << camera->GetFOVY();
			out << YAML::Key << "Near" << YAML::Value << camera->GetFar();
			out << YAML::Key << "Far" << YAML::Value << camera->GetNear();
			out << YAML::EndMap;
		}

		if (entity->HasComponent<PointLightComponent>())
		{
			auto light = entity->GetComponent<PointLightComponent>()->light;
			out << YAML::Key << "PointLight";
			out << YAML::BeginMap;
			out << YAML::Key << "Color" << YAML::Value << light->GetColor();
			out << YAML::Key << "Position" << YAML::Value << light->GetPosition();
			out << YAML::Key << "Intensity" << YAML::Value << light->GetIntensity();
			out << YAML::Key << "AttenuationFactors" << YAML::Value << glm::vec3(light->GetConstantAttenuation(), light->GetLinearAttenuation(), light->GetQuadraticAttenuation());
			out << YAML::EndMap;
		}

		if (entity->HasComponent<DirectionalLightComponent>())
		{
			auto light = entity->GetComponent<DirectionalLightComponent>()->light;
			out << YAML::Key << "DirectionalLight";
			out << YAML::BeginMap;
			out << YAML::Key << "Color" << YAML::Value << light->GetColor();
			out << YAML::Key << "Direction" << YAML::Value << light->GetDirection();
			out << YAML::Key << "Intensity" << YAML::Value << light->GetIntensity();
			out << YAML::EndMap;
		}

		if (entity->HasComponent<MeshComponent>())
		{
			auto mesh = entity->GetComponent<MeshComponent>()->mesh;
			out << YAML::Key << "Mesh";
			out << YAML::BeginMap;
			out << YAML::Key << "FilePath" << YAML::Value << mesh->GetMetaData().MeshPath;
			out << YAML::EndMap;
		}
	}
}
