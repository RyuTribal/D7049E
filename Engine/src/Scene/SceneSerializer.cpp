#include "pch.h"
#include "SceneSerializer.h"
#include "Serialization/YAMLSerializer.h"
#include "Scene/Entity.h"
#include "Scene/Components.h"
#include "Assets/AssetTypes.h"
#include "Assets/ModelImporter.h"
#include "Scene/Scene.h"
#include "Assets/DesignAssetManager.h"

namespace Engine{

	void SceneSerializer::Serializer(const std::filesystem::path& directory, Scene* scene)
	{
		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "Scene";
		out << YAML::BeginMap;
		out << YAML::Key << "Handle" << YAML::Value << scene->Handle;
		out << YAML::Key << "Name" << YAML::Value << scene->GetName();
		out << YAML::EndMap;

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

		std::filesystem::path filepath = directory;

		if (filepath.has_extension())
		{
			filepath = filepath.parent_path();
		}
		
		if (!std::filesystem::exists(filepath))
		{
			std::filesystem::create_directories(filepath);
		}

		// Full path for the file
		filepath = filepath / std::filesystem::path(scene->GetName() + DesignAssetManager::GetFileExtensionFromAssetType(AssetType::Scene));
		if (!filepath.is_absolute())
		{
			filepath = Project::GetFullFilePath(filepath);
		}
		std::ofstream fout(filepath);
		HVE_CORE_ASSERT(fout, "Failed to open file for writing: {0}", filepath);

		fout << out.c_str();
		fout.close();
		HVE_CORE_ASSERT(!fout.fail(), "Failed to write data to file: {0}", filepath);
		HVE_CORE_TRACE_TAG("Scene Serializer", "Scene saved successfully to: {0}", filepath);
		
	}
	Ref<Scene> SceneSerializer::Deserializer(const std::filesystem::path& filepath)
	{
		std::string full_file_path = Project::GetFullFilePath(filepath).string();
		YAML::Node root_node = YAML::LoadFile(full_file_path);
		if (!root_node["Scene"]) 
		{ 
			HVE_CORE_ERROR_TAG("Scene Deserializer", "File {0}, is not a proper scene file", filepath.string());
			return nullptr;
		};

		AssetHandle handle = root_node["Scene"]["Handle"].as<AssetHandle>(0);
		std::string sceneName = root_node["Scene"]["Name"].as<std::string>();
		Ref<Scene> new_scene = CreateRef<Scene>(sceneName);

		if (root_node["Entities"])
		{
			YAML::Node entities = root_node["Entities"];
			for (YAML::const_iterator it = entities.begin(); it != entities.end(); ++it)
			{
				YAML::Node entity_node = *it;
				DeserializeEntity(entity_node, new_scene, nullptr);
			}
		}

		return new_scene;
	}

	void SceneSerializer::TraverseTree(YAML::Emitter& out, SceneNode* scene_node, Scene* scene)
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

		if (entity->HasComponent<ScriptComponent>())
		{
			auto scriptcomp = entity->GetComponent<ScriptComponent>();
			out << YAML::Key << "Script";
			out << YAML::BeginMap;
			out << YAML::Key << "ClassName" << YAML::Value << scriptcomp->Name;
			out << YAML::EndMap;
		}

		if (entity->HasComponent<CameraComponent>())
		{
			auto& camera = entity->GetComponent<CameraComponent>()->camera;
			out << YAML::Key << "Camera";
			out << YAML::BeginMap;
			out << YAML::Key << "ProjectionType" << YAML::Value << (int)camera.GetType();
			out << YAML::Key << "PerspectiveFOV" << YAML::Value << camera.GetFOVY();
			out << YAML::Key << "Near" << YAML::Value << camera.GetNear();
			out << YAML::Key << "Far" << YAML::Value << camera.GetFar();
			out << YAML::Key << "Zoom" << YAML::Value << camera.GetZoomDistance();
			out << YAML::Key << "IsPrimary" << YAML::Value << entity->GetComponent<CameraComponent>()->IsPrimary;
			out << YAML::EndMap;
		}

		if (entity->HasComponent<PointLightComponent>())
		{
			auto light = entity->GetComponent<PointLightComponent>()->light;
			out << YAML::Key << "PointLight";
			out << YAML::BeginMap;
			out << YAML::Key << "Color" << YAML::Value << light.GetColor();
			out << YAML::Key << "Position" << YAML::Value << light.GetPosition();
			out << YAML::Key << "Intensity" << YAML::Value << light.GetIntensity();
			out << YAML::Key << "AttenuationFactors" << YAML::Value << glm::vec3(light.GetConstantAttenuation(), light.GetLinearAttenuation(), light.GetQuadraticAttenuation());
			out << YAML::EndMap;
		}

		if (entity->HasComponent<DirectionalLightComponent>())
		{
			auto light = entity->GetComponent<DirectionalLightComponent>()->light;
			out << YAML::Key << "DirectionalLight";
			out << YAML::BeginMap;
			out << YAML::Key << "Color" << YAML::Value << light.GetColor();
			out << YAML::Key << "Direction" << YAML::Value << light.GetDirection();
			out << YAML::Key << "Intensity" << YAML::Value << light.GetIntensity();
			out << YAML::EndMap;
		}

		if (entity->HasComponent<MeshComponent>())
		{
			auto mesh = entity->GetComponent<MeshComponent>()->mesh;
			out << YAML::Key << "Mesh";
			out << YAML::BeginMap;
			out << YAML::Key << "Handle" << YAML::Value << mesh->GetMeshSource()->Handle;
			out << YAML::EndMap;
		}

		if (entity->HasComponent<BoxColliderComponent>())
		{
			auto collider = entity->GetComponent<BoxColliderComponent>();
			out << YAML::Key << "BoxCollider";
			out << YAML::BeginMap;
			out << YAML::Key << "HalfSize" << YAML::Value << collider->HalfSize;
			out << YAML::Key << "Offset" << YAML::Value << collider->Offset;
			out << YAML::Key << "MotionType" << YAML::Value << FromMotionTypeToString(collider->MotionType);
			out << YAML::EndMap;
		}

		if (entity->HasComponent<SphereColliderComponent>())
		{
			auto collider = entity->GetComponent<SphereColliderComponent>();
			out << YAML::Key << "SphereCollider";
			out << YAML::BeginMap;
			out << YAML::Key << "Radius" << YAML::Value << collider->Radius;
			out << YAML::Key << "Offset" << YAML::Value << collider->Offset;
			out << YAML::Key << "MotionType" << YAML::Value << FromMotionTypeToString(collider->MotionType);
			out << YAML::EndMap;
		}
	}
	void SceneSerializer::DeserializeEntity(YAML::Node entity_node, Ref<Scene> scene, UUID* parent_entity_id)
	{
		UUID entity_id = entity_node["Entity"].as<uint64_t>();
		std::string name = "New Entity";
		if (entity_node["Tag"])
		{
			name = entity_node["Tag"].as<std::string>();
		}
		EntityHandle* entity = scene->CreateEntityByUUID(entity_id, name, parent_entity_id);

		TransformComponent entity_transform{};

		if (entity_node["LocalTransformComponent"])
		{
			entity_transform.local_transform.translation = entity_node["LocalTransformComponent"]["Position"].as<glm::vec3>(glm::vec3(0.0f));
			entity_transform.local_transform.rotation = entity_node["LocalTransformComponent"]["Rotation"].as<glm::vec3>(glm::vec3(0.0f));
			entity_transform.local_transform.scale = entity_node["LocalTransformComponent"]["Scale"].as<glm::vec3>(glm::vec3(0.0f));
		}

		if (entity_node["WorldTransformComponent"])
		{
			entity_transform.local_transform.translation = entity_node["WorldTransformComponent"]["Position"].as<glm::vec3>(glm::vec3(0.0f));
			entity_transform.local_transform.rotation = entity_node["WorldTransformComponent"]["Rotation"].as<glm::vec3>(glm::vec3(0.0f));
			entity_transform.local_transform.scale = entity_node["WorldTransformComponent"]["Scale"].as<glm::vec3>(glm::vec3(0.0f));
		}

		scene->GetEntity(entity)->AddComponent<TransformComponent>(entity_transform);

		if (entity_node["Camera"])
		{
			auto camera_component = CameraComponent{};
			camera_component.camera = Camera(entity_node["Camera"]["ProjectionType"].as<int>() == 1 ? CameraType::PERSPECTIVE : CameraType::ORTHOGRAPHIC);
			camera_component.camera.SetFovy(entity_node["Camera"]["PerspectiveFOV"].as<float>());
			camera_component.camera.SetNear(entity_node["Camera"]["Near"].as<float>());
			camera_component.camera.SetFar(entity_node["Camera"]["Far"].as<float>());
			if (entity_node["Camera"]["Zoom"])
			{
				camera_component.camera.SetZoomDistance(entity_node["Camera"]["Zoom"].as<float>());
			}
			camera_component.IsPrimary = entity_node["Camera"]["IsPrimary"].as<bool>();
			scene->GetEntity(entity)->AddComponent<CameraComponent>(camera_component);
		}

		if (entity_node["Script"])
		{
			auto script_component = ScriptComponent{};
			script_component.Name = entity_node["Script"]["ClassName"].as<std::string>();
			scene->GetEntity(entity)->AddComponent<ScriptComponent>(script_component);
		}

		

		if (entity_node["PointLight"])
		{
			auto light = PointLightComponent{};
			light.light = PointLight();
			light.light.SetColor(entity_node["PointLight"]["Color"].as<glm::vec3>(glm::vec3(0.0f)));
			light.light.SetPosition(entity_node["PointLight"]["Position"].as<glm::vec3>(glm::vec3(0.0f)));
			light.light.SetIntensity(entity_node["PointLight"]["Intensity"].as<float>());
			glm::vec3 attenuation_factors = entity_node["PointLight"]["AttenuationFactors"].as<glm::vec3>(glm::vec3(0.0f));
			light.light.SetConstantAttenuation(attenuation_factors.x);
			light.light.SetLinearAttenuation(attenuation_factors.y);
			light.light.SetQuadraticAttenuation(attenuation_factors.z);
			scene->GetEntity(entity)->AddComponent<PointLightComponent>(light);
		}

		if (entity_node["DirectionalLight"])
		{
			auto light = DirectionalLightComponent{};
			light.light = DirectionalLight();
			glm::vec3 color = entity_node["DirectionalLight"]["Color"].as<glm::vec3>(glm::vec3(0.0f));
			light.light.SetColor(color);
			glm::quat direction = entity_node["DirectionalLight"]["Direction"].as<glm::quat>(glm::quat(1.0f, 0.f, 0.f, 0.f));
			light.light.SetDirection(direction);
			light.light.SetIntensity(entity_node["DirectionalLight"]["Intensity"].as<float>());
			scene->GetEntity(entity)->AddComponent<DirectionalLightComponent>(light);
		}

		if (entity_node["Mesh"])
		{
			MeshComponent mesh_comp{};
			Ref<MeshSource> source = AssetManager::GetAsset<MeshSource>(entity_node["Mesh"]["Handle"].as<AssetHandle>(0));
			mesh_comp.mesh = CreateRef<Mesh>(source);
			scene->GetEntity(entity)->AddComponent<MeshComponent>(mesh_comp);
		}

		if (entity_node["BoxCollider"])
		{
			BoxColliderComponent collider{};
			collider.HalfSize = entity_node["BoxCollider"]["HalfSize"].as<glm::vec3>(glm::vec3(0.5f));
			collider.Offset = entity_node["BoxCollider"]["Offset"].as<glm::vec3>(glm::vec3(0.0f));
			collider.MotionType = FromStringToMotionType(entity_node["BoxCollider"]["MotionType"].as<std::string>());
			scene->GetEntity(entity)->AddComponent<BoxColliderComponent>(collider);
		}

		if (entity_node["SphereCollider"])
		{
			SphereColliderComponent collider{};
			collider.Radius = entity_node["SphereCollider"]["Radius"].as<float>();
			collider.Offset = entity_node["SphereCollider"]["Offset"].as<glm::vec3>(glm::vec3(0.0f));
			collider.MotionType = FromStringToMotionType(entity_node["SphereCollider"]["MotionType"].as<std::string>());
			scene->GetEntity(entity)->AddComponent<SphereColliderComponent>(collider);
		}


		if (entity_node["Children"])
		{
			for (const auto& child_node : entity_node["Children"])
			{
				DeserializeEntity(child_node, scene, &entity_id);
			}
		}

	}
}
