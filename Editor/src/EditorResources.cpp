#include "EditorResources.h"

using namespace Engine;

namespace Editor {

	std::unordered_map<std::string, Ref<Texture2D>> EditorResources::FileIcons{};

	void EditorResources::Init()
	{
		const std::vector<std::pair<std::string, std::string>> iconFiles = {
		{".gltf", "GLTF.png"}, {".fbx", "FBX.png"}, {".FBX", "FBX.png"}, {".jpg", "JPG.png"},
		{".jpeg", "JPG.png"}, {".png", "PNG.png"}, {".wav", "WAV.png"},
		{".ogg", "OGG.png"}, {".mp3", "MP3.png"}, {".hvescn", "SCENE.png"},
		{".cs", "CSHARP.png"},
		{"default", "File.png"}, {"", "FOLDER.png"}, {"back", "BACK.png"}, {"unregistered", "UNREG.png"},
		{"play", "PLAY.PNG"}, {"pause", "PAUSE.PNG"}, {"stop", "STOP.PNG"}
		};

		for (const auto& [ext, file] : iconFiles)
		{
			FileIcons[ext] = TextureImporter::ImportWithPath("Resources/Icons/" + file);
		}
	}
}