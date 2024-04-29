project "ScriptCore"
    kind "SharedLib"
    staticruntime "off"
    language "C#"
    dotnetframework "4.7.2"
    targetdir (os.getenv("HVE_ROOT_DIR") .. "/Editor/Resources/Scripts")
    objdir (os.getenv("HVE_ROOT_DIR") .. "/Editor/Resources/Scripts/Intermediates")
    namespace "Helios"

    links { 
        os.getenv("HVE_ROOT_DIR") .. "/Editor/mono/lib/mono/4.5/System.Numerics",
        os.getenv("HVE_ROOT_DIR") .. "/Editor/mono/lib/mono/4.5/System.Numerics.Vectors"
    }

    files
    {
        "Source/**.cs",
        "Properties/**.cs",
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