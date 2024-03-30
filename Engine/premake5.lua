project "Engine"
    kind "StaticLib"
    staticruntime "off"
    language "C++"
    cppdialect "C++17"
    targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

    pchheader "pch.h"
    pchsource "src/pch.cpp"

    files
    {
        "src/**.h",
        "src/**.hpp",
        "src/**.cpp",
        "src/**.c",
        "vendor/glm/glm/**.hpp",
        "vendor/glm/glm/**.inl"
    }

    removefiles
    {
        "src/Platform/**"
    }

    libdirs
    {
        "vendor/GLFW/lib-vc2022",
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
        "vendor/spdlog/include",
        "vendor/stb",
        "%{IncludeDir.GLFW}",
        "%{IncludeDir.Glad}",
        "%{IncludeDir.ImGui}",
        "%{IncludeDir.glm}",
        "src",
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
            "src/Platform/Windows/**.cpp",
            "src/Platform/Windows/**.h",
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
            "src/Platform/Linux/**.cpp",
            "src/Platform/Linux/**.h",
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