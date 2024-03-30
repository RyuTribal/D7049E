include "Dependencies.lua"

workspace "Helios"
    architecture "x64"
    startproject "Editor"

    configurations
    {
        "Debug",
        "Release",
        "Dist"
    }

    outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

group "Dependencies"
    include "Engine/vendor/GLFW"
    include "Engine/vendor/Glad"
    include "Engine/vendor/imgui"
group ""
    include "Engine"

group "Tools"
    include "Editor"

group "Misc"
    include "App"