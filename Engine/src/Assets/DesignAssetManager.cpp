#include "pch.h"
#include "DesignAssetManager.h"
#include "AssetImporter.h"
#include "Serialization/YAMLSerializer.h"
#include "Project/Project.h"

namespace Engine {

	static std::map<std::filesystem::path, AssetType> s_AssetExtensionMap = {
		{ ".hvescn", AssetType::Scene },
		{ ".hvereg", AssetType::AssetRegistry },
		{ ".hvemat", AssetType::Material },
		{ ".fbx", AssetType::MeshSource },
		{ ".FBX", AssetType::MeshSource },
		{ ".gltf", AssetType::MeshSource },
		{ ".glb", AssetType::MeshSource },
		{ ".png", AssetType::Texture },
		{ ".jpg", AssetType::Texture },
		{ ".jpeg", AssetType::Texture },
		{ ".hdr", AssetType::CubeMap },
		{ ".wav", AssetType::Audio },
		{ ".ogg", AssetType::Audio },
		{ ".mp3", AssetType::Audio }
	};


	Ref<Asset> DesignAssetManager::GetAsset(AssetHandle handle)
	{
		if (!IsAssetHandleValid(handle))
		{
			return nullptr;
		}

		if (IsAssetLoaded(handle))
		{
			return m_LoadedAssets.at(handle);
		}

		const AssetMetadata& metadata = GetMetadata(handle);

		Ref<Asset> asset = AssetImporter::Import(handle, metadata);

		m_LoadedAssets[handle] = asset;

		if (!asset)
		{
			HVE_CORE_ERROR_TAG("Design Asset Importer", "Asset {0} could not load", handle);
			return nullptr;
		}

		return asset;
	}

	bool DesignAssetManager::IsAssetHandleValid(AssetHandle handle) const
	{
		return m_AssetRegistry.Contains(handle);
	}
	bool DesignAssetManager::IsAssetLoaded(AssetHandle handle) const
	{
		return m_LoadedAssets.contains(handle);
	}
	const AssetMetadata& DesignAssetManager::GetMetadata(AssetHandle handle) const
	{
		static AssetMetadata s_NullMetadata;
		if (!IsAssetHandleValid(handle))
		{
			return s_NullMetadata;
		}

		return m_AssetRegistry.Get(handle);
	}

	AssetType DesignAssetManager::GetAssetTypeFromFileExtension(const std::filesystem::path& extension)
	{
		if (s_AssetExtensionMap.find(extension) == s_AssetExtensionMap.end())
		{
			// HVE_CORE_WARN("Could not find AssetType for {}", extension);
			return AssetType::None;
		}
		return s_AssetExtensionMap.at(extension);
	}

	const std::string DesignAssetManager::GetFileExtensionFromAssetType(const AssetType type)
	{
		for (auto file_extension : s_AssetExtensionMap)
		{
			if (file_extension.second == type)
			{
				return file_extension.first.string();
			}
		}

		HVE_CORE_WARN_TAG("Design Asset Manager", "File of type {0} did not have a file extension", Utils::AssetTypeToString(type));
		return "";
	}

	AssetHandle DesignAssetManager::ImportAsset(const std::filesystem::path& file_path)
	{
		AssetHandle handle;
		AssetMetadata metadata;
		std::filesystem::path relativePath = std::filesystem::relative(file_path, Project::GetActive()->GetSettings().RootPath);
		metadata.FilePath = relativePath;
		metadata.Type = GetAssetTypeFromFileExtension(file_path.extension());
		if (metadata.Type == AssetType::None)
		{
			return 0;
		}

		if (GetHandleByPath(file_path) != 0)
		{
			return 0; // means it's already imported
		}

		Ref<Asset> asset = AssetImporter::Import(handle, metadata);
		if (asset)
		{
			asset->Handle = handle;
			m_LoadedAssets[handle] = asset;
			m_AssetRegistry[handle] = metadata;
			SerializeAssetRegistry();
			return handle;
		}
		HVE_CORE_ERROR_TAG("Asset Importer", "Failed to import asset at path {0}", file_path.string());
		return 0;
	}
	void DesignAssetManager::RegisterAsset(AssetHandle handle, const std::filesystem::path& file_path)
	{
		std::filesystem::path new_file_path = file_path;
		if (file_path.is_absolute())
		{
			new_file_path = std::filesystem::relative(file_path, Project::GetActive()->GetSettings().RootPath);
		}
		AssetMetadata metadata{};
		metadata.FilePath = new_file_path;
		HVE_CORE_ASSERT(new_file_path.has_extension(), "This file has no extension!");
		metadata.Type = GetAssetTypeFromFileExtension(new_file_path.extension());
		m_AssetRegistry[handle] = metadata;
		SerializeAssetRegistry();
	}
	void DesignAssetManager::UnregsiterAsset(AssetHandle handle)
	{
		bool has_handle = m_AssetRegistry.Contains(handle);
		if (m_AssetRegistry.Contains(handle))
		{
			uint32_t has_erased = m_AssetRegistry.Remove(handle);
			SerializeAssetRegistry();
		}
	}
	bool DesignAssetManager::IsAssetRegistered(const std::filesystem::path& file_path)
	{
		for (const auto& [handle, metadata] : m_AssetRegistry)
		{
			
			if (Project::ArePathsEqual(metadata.FilePath, file_path))
			{
				return true;
			}
		}
		return false;
	}
	AssetHandle DesignAssetManager::GetHandleByPath(const std::filesystem::path& file_path)
	{
		for (const auto& [handle, metadata] : m_AssetRegistry)
		{
			std::filesystem::path relative_path = file_path;
			if (file_path.is_absolute())
			{
				relative_path = std::filesystem::relative(file_path, Project::GetActive()->GetSettings().RootPath);
			}
			if (metadata.FilePath == relative_path)
			{
				return handle;
			}
		}
		return 0;
	}
	void DesignAssetManager::SerializeAssetRegistry()
	{
		std::filesystem::path path = Project::GetActive()->GetSettings().RootPath;
		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "AssetRegistry" << YAML::Value;
		out << YAML::BeginSeq;
		for (const auto& [handle, metadata] : m_AssetRegistry)
		{
			if (!metadata.IsEmbedded)
			{
				out << YAML::BeginMap;
				out << YAML::Key << "Handle" << YAML::Value << handle;
				out << YAML::Key << "FilePath" << YAML::Value << metadata.FilePath.string();
				out << YAML::Key << "Type" << YAML::Value << Utils::AssetTypeToString(metadata.Type);
				out << YAML::EndMap;
			}
		}
		out << YAML::EndSeq;
		out << YAML::EndMap;
		std::filesystem::path full_path = path / Project::GetActive()->GetSettings().AssetRegistryPath;
		std::ofstream fout(full_path.string());
		HVE_CORE_ASSERT(fout, "Failed to open file for writing: {0}", full_path.string());

		fout << out.c_str();
		fout.close();
		HVE_CORE_ASSERT(!fout.fail(), "Failed to write data to file: {0}", full_path.string());
		HVE_CORE_TRACE_TAG("Asset Registry Serializer", "Project saved successfully to: {0}", full_path.string());

	}
	void DesignAssetManager::DeserializeAssetRegistry()
	{
		std::filesystem::path root_path = Project::GetActive()->GetSettings().RootPath;
		std::filesystem::path asset_path = Project::GetActive()->GetSettings().AssetRegistryPath;
		std::filesystem::path path = root_path / asset_path;
		YAML::Node data = YAML::LoadFile(path.string());

		auto rootNode = data["AssetRegistry"];
		if (!rootNode)
		{
			HVE_CORE_ERROR_TAG("Registry Desirializer", "Failed to deserialize asset registry!");
			return;
		}

		for (const auto& node : rootNode)
		{
			AssetHandle handle = node["Handle"].as<AssetHandle>(0);
			auto& metadata = m_AssetRegistry[handle];
			metadata.FilePath = std::filesystem::path(node["FilePath"].as<std::string>());
			metadata.Type = Utils::StringToAssetType(node["Type"].as<std::string>());
		}
	}
}
