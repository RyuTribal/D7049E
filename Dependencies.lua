IncludeDir = {}
IncludeDir["GLFW"] = "%{wks.location}/Engine/vendor/GLFW/include"
IncludeDir["Glad"] = "%{wks.location}/Engine/vendor/Glad/include"
IncludeDir["ImGui"] = "%{wks.location}/Engine/vendor/imgui"
IncludeDir["glm"] = "%{wks.location}/Engine/vendor/glm"
IncludeDir["Jolt"] = "%{wks.location}/Engine/vendor/JoltPhysics/JoltPhysics"
IncludeDir["Tracy"] = "%{wks.location}/Engine/vendor/tracy/tracy/public"
IncludeDir["Assimp"] = "%{wks.location}/Engine/vendor/assimp/include"

rootPath = path.getabsolute(".")

LibraryDir = {}

Library = {}
Library["Jolt"] = "JoltPhysics";
Library["Tracy"] = "Tracy";

Binaries = {}

