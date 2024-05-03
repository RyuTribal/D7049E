IncludeDir = {}
IncludeDir["GLFW"] = "%{wks.location}/Engine/vendor/GLFW/include"
IncludeDir["Glad"] = "%{wks.location}/Engine/vendor/Glad/include"
IncludeDir["ImGui"] = "%{wks.location}/Engine/vendor/imgui"
IncludeDir["glm"] = "%{wks.location}/Engine/vendor/glm"
IncludeDir["Jolt"] = "%{wks.location}/Engine/vendor/JoltPhysics/JoltPhysics"
IncludeDir["Tracy"] = "%{wks.location}/Engine/vendor/tracy/tracy/public"
IncludeDir["Assimp"] = "%{wks.location}/Engine/vendor/assimp/include"
IncludeDir["YamlCpp"] = "%{wks.location}/Engine/vendor/yaml-cpp/include"
IncludeDir["mono"] = "%{wks.location}/Engine/vendor/mono/include"
IncludeDir["FileWatcher"] = "%{wks.location}/Engine/vendor/filewatch/include"


rootPath = path.getabsolute(".")

LibraryDir = {}
LibraryDir["mono"] = "%{wks.location}/Engine/vendor/mono/lib/%{cfg.buildcfg}"

Library = {}
Library["Jolt"] = "JoltPhysics";
Library["Tracy"] = "Tracy";
Library["mono"] = "%{LibraryDir.mono}/libmono-static-sgen.lib"

Binaries = {}

-- Platform specific libraries
Library["WinSock"] = "Ws2_32.lib"
Library["WinMM"] = "Winmm.lib"
Library["WinVersion"] = "Version.lib"
Library["BCrypt"] = "Bcrypt.lib"
Library["DebugHelp"] = "Dbghelp.lib"

