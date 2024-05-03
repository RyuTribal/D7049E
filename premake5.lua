include "Dependencies.lua"
include "./vendor/solution_items.lua"

workspace "Helios"
    configurations { "Debug", "Debug-AS", "Release", "Dist" }
    targetdir "build"
    startproject "Editor"
    conformancemode "On"

    language "C++"
    cppdialect "C++20"
    staticruntime "Off"

    solution_items { ".editorconfig" }

    configurations { "Debug", "Release", "Dist" }

    flags { "MultiProcessorCompile" }

    configurations
    {
        "Debug",
        "Release",
        "Dist"
    }

    solution_items { ".editorconfig" }

    defines {
		"_CRT_SECURE_NO_WARNINGS",
		"_SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING",
		"TRACY_ENABLE",
		"TRACY_ON_DEMAND",
		"TRACY_CALLSTACK=10",
        
	}

    filter "action:vs*"
        linkoptions { "/ignore:4099" }
        disablewarnings { "4068" }

	filter "language:C++ or language:C"
		architecture "x86_64"

	filter "configurations:Debug or configurations:Debug-AS"
		optimize "Off"
		symbols "On"

	filter { "system:windows", "configurations:Debug-AS" }	
		flags { "NoRuntimeChecks", "NoIncrementalLink" }

	filter "configurations:Release"
		optimize "On"
		symbols "Default"

	filter "configurations:Dist"
		optimize "Full"
		symbols "Off"

	filter "system:windows"
		buildoptions { "/EHsc", "/Zc:preprocessor", "/Zc:__cplusplus" }


    outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

group "Dependencies"
    include "Engine/vendor/GLFW"
    include "Engine/vendor/Glad"
    include "Engine/vendor/imgui"
    include "Engine/vendor/JoltPhysics/JoltPhysicsPremake.lua"
    include "Engine/vendor/JoltPhysics/JoltViewerPremake.lua"
    include "Engine/vendor/tracy"

group "Core"
    include "Engine"
    include "ScriptCore"

group "Tools"
    include "Editor"
    include "EditorLauncher"

group ""