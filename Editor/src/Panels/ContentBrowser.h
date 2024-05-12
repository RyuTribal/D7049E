#pragma once
#include <Engine.h>
#include <filesystem>
#include <unordered_map>
#include <string>
#include <imgui/imgui.h>

using namespace Engine;

namespace EditorPanels {
	struct ButtonSettings
	{
		float ThumbnailSize = 112.f;
		float ButtonSize = 128.f;
		int ImageButtonPadding = 5;
		float ColumnPadding = 16.f;
	};

	struct DirectoryElement
	{
		std::filesystem::path FullFilePath;
		bool IsRegistered = false;
		bool IsDirectory = false;
		ImTextureID TextureID = 0;
	};


	class ContentBrowser
	{
	public:
		static void Create()
		{
			if (!s_Instance)
			{
				s_Instance = new ContentBrowser();
			}
		}

		static void Recreate()
		{
			s_Instance = new ContentBrowser();
		}

		static void Render(Ref<Scene> scene)
		{
			Create();
			s_Instance->SetActiveScene(scene);
			s_Instance->RenderImpl();
		}

		ContentBrowser();

	private:
		void RenderImpl();
		void RenderDirectoryTree(const std::filesystem::path& directory, bool root_dir);
		void RenderContentView();
		void RenderEntry(const std::filesystem::directory_entry& entry);
		void SetupContentView();
		void RenderTreeNode(const std::filesystem::directory_entry& entry, ImGuiTreeNodeFlags baseFlags);
		void RenderSubDirectories(const std::filesystem::path& directory, ImGuiTreeNodeFlags baseFlags);
		void RenderTreeNodes(const std::filesystem::path& directory, ImGuiTreeNodeFlags flags);

		void HandleItemSelection(const std::filesystem::path& path);
		void HandleDoubleClick(const std::filesystem::directory_entry& entry);
		void HandleDragAndDrop(const std::filesystem::path& path);
		void HandleContextMenu(const std::filesystem::path& path, const std::string& filenameStr);
		void DisplayBackButton(float thumbnailSize, float padding);
		void HandleBackgroundContextMenu();
		bool IsPathPartOfCurrentDirectory(const std::filesystem::path& path);

		void SetActiveScene(Ref<Scene> scene) { m_Scene = scene; }

		bool ShouldShowFile(const std::filesystem::path& path);

		void LoadDirectoryElement(const std::filesystem::path& path);

		void RefreshBrowser();

	private:
		static ContentBrowser* s_Instance;
		std::filesystem::path m_RootDirectory;
		std::filesystem::path m_CurrentDirectory;
		std::filesystem::path m_RenamingPath;
		std::filesystem::path m_DraggingPath;
		bool m_IsRenaming = false;
		char m_NameBuffer[256] = {};
		Ref<Scene> m_Scene;
		ButtonSettings m_ButtonSettings{};
		bool m_ShowRegisteredAssetsOnly = true;
		std::unordered_map<std::string, DirectoryElement> m_DirectoryElements;
	};
}
