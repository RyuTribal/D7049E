
local rootPath = path.getabsolute(".")

workspace "Project"
    architecture "x64"
    startproject "App"

    configurations
    {
        "Debug",
        "Release",
        "Dist"
    }

    outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

    IncludeDir = {}
    IncludeDir["GLFW"] = "Engine/vendor/GLFW/include"
    IncludeDir["Glad"] = "Engine/vendor/Glad/include"
    IncludeDir["ImGui"] = "Engine/vendor/imgui"
    IncludeDir["glm"] = "Engine/vendor/glm"

group "Dependencies"
    include "Engine/vendor/GLFW"
    include "Engine/vendor/Glad"
    include "Engine/vendor/imgui"
group ""

project "Engine"
    location "Engine"
    kind "StaticLib"
    staticruntime "off"
    language "C++"
    cppdialect "C++17"
    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

    pchheader "pch.h"
    pchsource "Engine/src/pch.cpp"

    files
    {
        "%{prj.name}/src/**.h",
        "%{prj.name}/src/**.hpp",
        "%{prj.name}/src/**.cpp",
        "%{prj.name}/src/**.c",
        "%{prj.name}/vendor/glm/glm/**.hpp",
        "%{prj.name}/vendor/glm/glm/**.inl"
    }

    removefiles
    {
        "%{prj.name}/src/Platform/**"
    }

    libdirs
    {
        "%{prj.name}/vendor/GLFW/lib-vc2022",
    }

    links
    {
        "GLFW",
        "Glad",
        "ImGui",
    }

    defines
    {
        "_CRT_SECURE_NO_WARNINGS",
        "ROOT_PATH=\"" .. rootPath .. "/" .. "%{prj.name}\""
    }

    includedirs
    {
        "%{prj.name}/vendor/spdlog/include",
        "%{prj.name}/vendor/stb",
        "%{IncludeDir.GLFW}",
        "%{IncludeDir.Glad}",
        "%{IncludeDir.ImGui}",
        "%{IncludeDir.glm}",
        "%{prj.name}/src",
    }

    filter "system:windows"
        systemversion "latest"
        defines
        {
            "PLATFORM_WINDOWS",
            "BUILD_DLL",
            "GLFW_INCLUDE_NONE"
        }
        
        links 
        {
            "opengl32.lib"
        }

        files
        {
            "%{prj.name}/src/Platform/Windows/**.cpp",
            "%{prj.name}/src/Platform/Windows/**.h",
        }
    filter "system:linux"
        systemversion "latest"
        defines
        {
            "PLATFORM_LINUX",
            "BUILD_DLL",
            "GLFW_INCLUDE_NONE"
        }

        links
        {
            "GL"
        }
        files
        {
            "%{prj.name}/src/Platform/Linux/**.cpp",
            "%{prj.name}/src/Platform/Linux/**.h",
        }

    filter "configurations:Debug"
        defines "DEBUG"
        runtime "Debug"
        symbols "on"

    filter "configurations:Release"
        defines "RELEASE"
        runtime "Release"
        optimize "on"

    filter "configurations:Dist"
        defines "DIST"
        runtime "Release"
        optimize "on"

project "App"
    kind "ConsoleApp"
    location "App"
    staticruntime "off"
    language "C++"
    cppdialect "C++17"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")
    files
    {
        "%{prj.name}/src/**.h",
        "%{prj.name}/src/**.hpp",
        "%{prj.name}/src/**.cpp",
        "%{prj.name}/src/**.c",
    }

    includedirs
    {
        "Engine/vendor/spdlog/include",
        "%{IncludeDir.glm}",
        "%{IncludeDir.Glad}",
        "Engine/vendor",
        "Engine/src",
    }

    links
    {
        "Engine",
        "Glad"
    }

    defines
    {
        "ROOT_PATH=\"" .. rootPath .. "/" .. "%{prj.name}\""
    }

    filter "system:windows"
        systemversion "latest"
        defines
        {
            "PLATFORM_WINDOWS",
        }
        links 
        {
            "opengl32.lib"
        }

    filter "system:linux"
        systemversion "latest"
        defines
        {
            "PLATFORM_LINUX",
            
        }
        links 
        {
            "GL"
        }

    filter "configurations:Debug"
        defines "DEBUG"
        runtime "Debug"
        symbols "on"

    filter "configurations:Release"
        defines "RELEASE"
        runtime "Release"
        optimize "on"

    filter "configurations:Dist"
        defines "DIST"
        runtime "Release"
        optimize "on"
