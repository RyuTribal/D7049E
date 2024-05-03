project "EditorLauncher"
    kind "ConsoleApp"
    staticruntime "off"

    targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")
    files
    {
        "src/**.h",
        "src/**.hpp",
        "src/**.cpp",
        "src/**.c",
    }

    includedirs
    {
        "%{wks.location}/Engine/vendor/spdlog/include",
        "%{IncludeDir.glm}",
        "%{IncludeDir.Glad}",
        "%{IncludeDir.Jolt}",
        "%{IncludeDir.Jolt}/Jolt",
        "%{IncludeDir.Tracy}",
        "%{IncludeDir.Assimp}",
        "%{wks.location}/Engine/vendor",
        "%{wks.location}/Engine/src",
        "%{wks.location}/Engine/vendor/nativefiledialog-extended/src/include",
        "%{wks.location}/Engine/vendor/SoLoud",
    }

    links
    {
        "Engine",
        "Glad",
        "%{Library.Tracy}",
        "%{wks.location}/Engine/vendor/nativefiledialog-extended/lib/nfd.lib",
        
    }

    defines
    {
        "ROOT_PATH=\"" .. rootPath .. "/" .. "%{prj.name}\"",
        'EDITOR_EXECUTABLE_PATH="'.. rootPath ..'/bin/' .. outputdir .. '/Editor/Editor.exe"',
        'EDITOR_WORKING_DIRECTORY="'.. rootPath ..'/Editor/"'
    }

    postbuildcommands
    {
        '{COPY} "%{wks.location}/Engine/vendor/assimp/shared/x64/assimp-vc143-mt.dll" "%{cfg.targetdir}"'
    }

    filter "system:windows"
        systemversion "latest"
        defines
        {
            "PLATFORM_WINDOWS"
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
