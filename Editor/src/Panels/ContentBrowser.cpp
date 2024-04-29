#include "ContentBrowser.h"

namespace EditorPanels {
	ContentBrowser* ContentBrowser::s_Instance = nullptr;

	ContentBrowser::ContentBrowser()
	{
		m_RootDirectory = Project::GetActive()->GetSettings().RootPath / Project::GetActive()->GetSettings().AssetPath;
		m_CurrentDirectory = m_RootDirectory;

		const std::vector<std::pair<std::string, std::string>> iconFiles = {
			{".gltf", "GLTF.png"}, {".fbx", "FBX.png"}, {".FBX", "FBX.png"}, {".jpg", "JPG.png"},
			{".jpeg", "JPG.png"}, {".png", "PNG.png"}, {".wav", "WAV.png"},
			{".ogg", "OGG.png"}, {".mp3", "MP3.png"}, {".hvescn", "SCENE.png"},
			{"default", "File.png"}, {"", "FOLDER.png"}, {"back", "BACK.png"}, {"unregistered", "UNREG.png"}
		};

		for (const auto& [ext, file] : iconFiles)
		{
			m_FileIcons[ext] = TextureImporter::ImportWithPath("Resources/Icons/" + file);
		}
	}

	void ContentBrowser::RenderImpl()
	{

		ImGui::Columns(2);
		ImGui::SetColumnWidth(0, 400);

		float footerHeight = ImGui::GetFrameHeightWithSpacing();
		float availableHeight = ImGui::GetContentRegionAvail().y - footerHeight;

		ImGui::BeginChild("DirectoryTreeScroll", ImVec2(0, availableHeight));
		RenderDirectoryTree(m_RootDirectory, true);
		ImGui::EndChild();

		ImGui::NextColumn();

		ImGui::BeginChild("ContentViewScroll", ImVec2(0, availableHeight));
		RenderContentView();
		ImGui::EndChild();

		ImGui::Columns(1);
		ImGui::Separator();
		ImGui::Checkbox("Show registered assets only", &m_ShowRegisteredAssetsOnly);
	}

	void ContentBrowser::RenderDirectoryTree(const std::filesystem::path& directory, bool root_dir)
	{
		ImGuiTreeNodeFlags baseFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;

		if (root_dir)
		{
			ImGui::BeginChild("Directory Tree");
			RenderTreeNodes(directory, baseFlags | ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Selected);
			ImGui::EndChild();
		}
		else
		{
			RenderSubDirectories(directory, baseFlags);
		}
	}

	void ContentBrowser::RenderTreeNodes(const std::filesystem::path& directory, ImGuiTreeNodeFlags flags)
	{
		if (ImGui::TreeNodeEx(directory.filename().string().c_str(), flags))
		{
			HandleItemSelection(directory);
			RenderDirectoryTree(directory, false);
			ImGui::TreePop();
		}
	}

	void ContentBrowser::RenderSubDirectories(const std::filesystem::path& directory, ImGuiTreeNodeFlags baseFlags)
	{
		for (const auto& entry : std::filesystem::directory_iterator(directory))
		{
			RenderTreeNode(entry, baseFlags);
		}
	}

	void ContentBrowser::RenderTreeNode(const std::filesystem::directory_entry& entry, ImGuiTreeNodeFlags baseFlags)
	{
		if (!entry.is_directory() && !ShouldShowFile(entry.path()))
		{
			return;
		}

		auto& path = entry.path();
		auto filenameString = path.filename().string();
		ImGuiTreeNodeFlags nodeFlags = baseFlags;

		if (entry.is_directory())
		{
			nodeFlags |= ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_OpenOnArrow;
		}
		else
		{
			nodeFlags |= ImGuiTreeNodeFlags_Leaf;
		}

		if (m_CurrentDirectory == path)
		{
			nodeFlags |= ImGuiTreeNodeFlags_Selected;
		}

		ImVec2 uv0 = ImVec2(0.0f, 1.0f);
		ImVec2 uv1 = ImVec2(1.0f, 0.0f);
		auto icon_id = entry.is_directory() ? m_FileIcons[""]->GetRendererID() : m_FileIcons["default"]->GetRendererID();

		ImGui::PushID(path.string().c_str()); // Unique ID for the node
		bool nodeOpen = ImGui::TreeNodeEx("##tree_browser_node", nodeFlags);

		if (ImGui::IsItemClicked())
		{
			if (entry.is_directory() && !ImGui::IsItemToggledOpen())
			{
				m_CurrentDirectory = path;
			}
		}

		HandleDragAndDrop(path);

		ImGui::SameLine();
		ImGui::Image((void*)(intptr_t)icon_id, ImVec2(16.f, 16.f), uv0, uv1);
		ImGui::SameLine();
		ImGui::Text("%s", filenameString.c_str());

		if (nodeOpen)
		{
			if (entry.is_directory())
			{
				RenderDirectoryTree(path, false);
			}
			ImGui::TreePop();
		}

		ImGui::PopID(); // Match the PushID
	}

	void ContentBrowser::RenderContentView()
	{
		SetupContentView();

		for (const auto& entry : std::filesystem::directory_iterator(m_CurrentDirectory))
		{
			RenderEntry(entry);
		}

		ImGui::Columns(1);
		ImGui::EndChild();
	}

	bool ContentBrowser::ShouldShowFile(const std::filesystem::path& path)
	{
		auto asset_manager = Project::GetActiveDesignAssetManager();
		if (asset_manager->GetAssetTypeFromFileExtension(path.extension()) == AssetType::None)
		{
			return false;
		}

		if (m_ShowRegisteredAssetsOnly && !asset_manager->IsAssetRegistered(path))
		{
			return false;
		}
		return true;
	}

	void ContentBrowser::SetupContentView()
	{
		ImGui::BeginChild("Content view");
		float cellSize = m_ButtonSettings.ThumbnailSize + m_ButtonSettings.ColumnPadding;
		float panelWidth = ImGui::GetContentRegionAvail().x;
		int columnCount = std::max(1, int(panelWidth / cellSize));

		DisplayBackButton(m_ButtonSettings.ThumbnailSize, m_ButtonSettings.ColumnPadding);
		ImGui::Columns(columnCount, 0, false);
		HandleBackgroundContextMenu();
	}

	void ContentBrowser::RenderEntry(const std::filesystem::directory_entry& entry)
	{
		if (!entry.is_directory() && !ShouldShowFile(entry.path()))
		{
			return;
		}
		const auto& path = entry.path();
		auto filenameStr = path.filename().stem().string();
		auto extensionStr = path.extension().string();
		ImGui::PushID(filenameStr.c_str());

		ImVec2 uv0 = ImVec2(0.0f, 1.0f);
		ImVec2 uv1 = ImVec2(1.0f, 0.0f);
		uint32_t texture_id = m_FileIcons[extensionStr] ? m_FileIcons[extensionStr]->GetRendererID() : m_FileIcons["default"]->GetRendererID();
		if (!entry.is_directory() && !Project::GetActiveDesignAssetManager()->IsAssetRegistered(entry.path()))
		{
			texture_id = m_FileIcons["unregistered"]->GetRendererID();
		}

		ImGui::BeginGroup();
		ImVec2 pos = ImGui::GetCursorScreenPos();

		std::string full_name = filenameStr;
		if (!entry.is_directory() && !Project::GetActiveDesignAssetManager()->IsAssetRegistered(entry.path()))
		{
			full_name += extensionStr;
		}

		ImVec2 buttonSize = ImVec2(m_ButtonSettings.ThumbnailSize, m_ButtonSettings.ThumbnailSize + ImGui::CalcTextSize(full_name.c_str()).y + m_ButtonSettings.ImageButtonPadding);

		ImGui::Button("##inv_button", buttonSize);

		HandleDoubleClick(entry);
		HandleDragAndDrop(path);
		float text_width = ImGui::CalcTextSize(full_name.c_str()).x;
		ImGui::SetCursorScreenPos({ pos.x, pos.y + m_ButtonSettings.ThumbnailSize + m_ButtonSettings.ImageButtonPadding });
		HandleContextMenu(path, filenameStr);

		ImGui::SetCursorScreenPos(pos);
		ImGui::Image(reinterpret_cast<void*>(static_cast<intptr_t>(texture_id)), ImVec2(m_ButtonSettings.ThumbnailSize, m_ButtonSettings.ThumbnailSize), uv0, uv1);

		ImGui::SetCursorScreenPos({ pos.x + (m_ButtonSettings.ThumbnailSize - text_width) * 0.5f, pos.y + m_ButtonSettings.ThumbnailSize + m_ButtonSettings.ImageButtonPadding });
		if (!m_IsRenaming || m_RenamingPath != path)
		{
			ImGui::TextWrapped(full_name.c_str());
		}

		ImGui::EndGroup();

		ImGui::NextColumn();
		ImGui::PopID();
	}

	bool ContentBrowser::IsPathPartOfCurrentDirectory(const std::filesystem::path& path)
	{
		return m_CurrentDirectory == path || m_CurrentDirectory.string().find(path.string()) == 0;
	}

	
	void ContentBrowser::HandleItemSelection(const std::filesystem::path& path)
	{
		if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
		{
			m_CurrentDirectory = path;
		}
	}

	void ContentBrowser::HandleDoubleClick(const std::filesystem::directory_entry& entry)
	{
		if (ImGui::IsMouseDoubleClicked(0) && ImGui::IsItemHovered())
		{
			if (entry.is_directory())
			{
				m_CurrentDirectory = entry.path();
			}
			else
			{
				if (!Project::GetActiveDesignAssetManager()->IsAssetRegistered(entry.path()))
				{
					Project::GetActiveDesignAssetManager()->ImportAsset(entry.path());
				}
			}
		}
	}


	void ContentBrowser::HandleDragAndDrop(const std::filesystem::path& path)
	{
		if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
		{
			m_DraggingPath = path;
			ImGui::SetDragDropPayload("CONTENT_BROWSER_ITEM", &m_DraggingPath, sizeof(std::filesystem::path));
			ImGui::Text("Move %s", path.filename().string().c_str());
			ImGui::EndDragDropSource();
		}

		if (std::filesystem::is_directory(path) && ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
			{
				auto droppedPath = *(const std::filesystem::path*)payload->Data;
				std::filesystem::path newPath = path / droppedPath.filename();
				std::filesystem::rename(droppedPath, newPath);
				if (!std::filesystem::is_directory(droppedPath) && Project::GetActiveDesignAssetManager()->IsAssetRegistered(droppedPath))
				{
					AssetHandle handle = Project::GetActiveDesignAssetManager()->GetHandleByPath(droppedPath);
					Project::GetActiveDesignAssetManager()->UnregsiterAsset(handle);
					Project::GetActiveDesignAssetManager()->RegisterAsset(handle, newPath);
					if (droppedPath.extension() != Project::GetActiveDesignAssetManager()->GetFileExtensionFromAssetType(AssetType::Scene))
					{
						m_Scene->ReloadScene();
					}
				}
			}
			ImGui::EndDragDropTarget();
		}
	}


	void ContentBrowser::HandleContextMenu(const std::filesystem::path& path, const std::string& filenameStr)
	{
		if (ImGui::BeginPopupContextItem(("Context" + filenameStr).c_str()))
		{
			if (ImGui::MenuItem("Rename"))
			{
				m_RenamingPath = path;
				m_IsRenaming = true;
				strcpy(m_NameBuffer, filenameStr.c_str());
				ImGui::OpenPopup("RenamePopup");
			}
			if (ImGui::MenuItem("Delete"))
			{
				std::filesystem::remove_all(path);
				if (m_RenamingPath == path)
				{
					m_IsRenaming = false;
				}
			}
			ImGui::EndPopup();
		}
		if (m_IsRenaming && m_RenamingPath == path)
		{
			if (!ImGui::IsAnyItemActive() && !ImGui::IsMouseClicked(0))
			{
				ImGui::SetKeyboardFocusHere();
			}

			if (ImGui::InputText("##rename", m_NameBuffer, sizeof(m_NameBuffer), ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll))
			{
				std::filesystem::path newPath = m_RenamingPath.parent_path() / m_NameBuffer; // Use only user input for the new name
				if (std::filesystem::exists(newPath))
				{
					HVE_ERROR_TAG("Content browser", "File already exists!");
				}
				else
				{
					std::filesystem::rename(m_RenamingPath, newPath);
					m_IsRenaming = false;
				}
			}
			if (ImGui::IsItemDeactivated())
			{
				m_IsRenaming = false;
			}
		}
	}


	void ContentBrowser::DisplayBackButton(float thumbnailSize, float padding)
	{
		ImGui::BeginDisabled(m_CurrentDirectory == m_RootDirectory);
		ImVec2 uv0 = ImVec2(0.0f, 1.0f);
		ImVec2 uv1 = ImVec2(1.0f, 0.0f);
		auto texture_id = m_FileIcons["back"]->GetRendererID();

		if (ImGui::ImageButton(reinterpret_cast<void*>(static_cast<intptr_t>(texture_id)), ImVec2(24.0f, 24.0f), uv0, uv1))
		{
			m_CurrentDirectory = m_CurrentDirectory.parent_path();
		}
		ImGui::EndDisabled();
	}

	void ContentBrowser::HandleBackgroundContextMenu()
	{
		if (ImGui::BeginPopupContextWindow())
		{
			if (ImGui::MenuItem("Add New Folder"))
			{
				std::filesystem::path newFolderPath = m_CurrentDirectory / "New Folder";
				if (std::filesystem::create_directory(newFolderPath))
				{
					m_RenamingPath = newFolderPath;
					m_IsRenaming = true;
					strcpy(m_NameBuffer, "New Folder");
				}
			}
			ImGui::EndPopup();
		}
	}

}