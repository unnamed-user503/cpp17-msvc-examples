workspace "Workspace"
    architecture "x86_64"
    startproject "Example1"

    configurations
    {
        "Debug", "Release", "Distribute",
    }

    flags
    {
        -- "MultiProcessorCompile"
    }

    defines
    {
        "NOMINMAX", "WIN32_LEAN_AND_MEAN", "STRICT", "STRICT_TYPED_ITEMIDS",
    }

    filter "system:windows"
        systemversion "latest"

        buildoptions
        {
            "/EHsc", "/Zc:preprocessor", "/Zc:__cplusplus", "/utf-8",
        }

        defines
        {
            "_WINDOWS"
        }

    filter "configurations:Debug"
        runtime  "Debug"
        symbols  "On"
        optimize "Off"

        defines
        {
            "_DEBUG"
        }

    filter "configurations:Release"
        runtime  "Release"
        symbols  "On"
        optimize "On"

        defines
        {
            "NDEBUG"
        }

    filter "configurations:Distribute"
        runtime  "Release"
        symbols  "Off"
        optimize "On"

        defines
        {
            "NDEBUG"
        }

OutputDir = "%{cfg.architecture}-%{cfg.system}-%{cfg.buildcfg:lower()}"

include "Vendor/Premake5/Customization/Core.lua"

group "Dependencies"
group ""

include "Projects/Example1/Build.lua"
include "Projects/Example2/Build.lua"
include "Projects/Example3/Build.lua"
include "Projects/Example4/Build.lua"
include "Projects/Example5/Build.lua"
include "Projects/Example6/Build.lua"
include "Projects/Example7/Build.lua"
include "Projects/Example8/Build.lua"
