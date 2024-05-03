include(os.getenv("HVE_ROOT_DIR") .. "/vendor/premake/premake_customization/solution_items.lua")
workspace "SandBox"
architecture "x86_64"
startproject "SandBox"
configurations { "Debug", "Release", "Dist" }
flags {
	"MultiProcessorCompile",
}
project "SandBox"
kind "SharedLib"
language "C#"
dotnetframework "4.7.2"
targetdir "../Binaries"
objdir "../Binaries/Intermediates"
namespace "SandBox"
location"../Assets/Scripts"
links {
	os.getenv("HVE_ROOT_DIR") .. "/Editor/Resources/Scripts/ScriptCore",
	os.getenv("HVE_ROOT_DIR") .. "/Editor/mono/lib/mono/4.5/System.Numerics",
	os.getenv("HVE_ROOT_DIR") .. "/Editor/mono/lib/mono/4.5/System.Numerics.Vectors",
}
files {
	"../Assets/Scripts/**.cs",
}
vpaths {
	["Scripts"] = {"../Assets/Scripts/**.cs"},
}
filter "configurations:Debug"
optimize "Off"
symbols "Default"
filter "configurations:Release"
optimize "On"
symbols "Default"
filter "configurations:Dist"
optimize "Full"
symbols "Off"
group "Helios"
include(os.getenv("HVE_ROOT_DIR") .. "/ScriptCore")
