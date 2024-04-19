project "Engine"
    kind "StaticLib"
    staticruntime "off"
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
        "vendor/glm/glm/**.inl",

    }

    removefiles
    {
        "src/Platform/**"
    }

    libdirs
    {
        "vendor/GLFW/lib-vc2022",
        "vendor/assimp/lib/x64"
    }

    links
    {
        "GLFW",
        "Glad",
        "ImGui",
        "JoltPhysics",
        "%{Library.Tracy}",
        "Ws2_32",
        "Dbghelp",
        "assimp-vc143-mt.lib"
    }

    defines
    {
        "_CRT_SECURE_NO_WARNINGS",
        "ROOT_PATH=\"" .. rootPath .. "/" .. "%{prj.name}\"",
        "JPH_USE_LZCNT",
        "JPH_USE_TZCNT",
        "JPH_USE_FMADD",
    }

    includedirs
    {
        "vendor/spdlog/include",
        "vendor/stb",
        "vendor/SoLoud",
        "%{IncludeDir.GLFW}",
        "%{IncludeDir.Glad}",
        "%{IncludeDir.ImGui}",
        "%{IncludeDir.glm}",
        "%{IncludeDir.Jolt}",
        "%{IncludeDir.Jolt}/Jolt",
        "%{IncludeDir.Tracy}",
        "%{IncludeDir.Assimp}",
        "src/",
    }

    flags { "NoPCH" }

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