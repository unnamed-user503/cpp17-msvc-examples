project "Example4"
    kind          "ConsoleApp" -- ConsoleApp WindowedApp SharedLib StaticLib
    language      "C++"
    cppdialect    "C++17"
    staticruntime "off"
    characterset  "MBCS"

    pchheader "Pch.hpp"
    pchsource "Source/Pch.cpp"

    targetdir ("%{wks.location}/Binaries/" .. OutputDir .. "/%{prj.name}")
    objdir    ("%{wks.location}/Binaries-Intermediates/" .. OutputDir .. "/%{prj.name}")

    vspropertysettings
    {
        VcpkgTriplet = "x64-windows-static",
        VcpkgEnabled = "true",
    }

    files
    {
        "Example4.exe.manifest", "Source/**.cpp", "Source/**.hpp", "Source/**.h",
    }

    includedirs
    {
        "Source",
    }

    links
    {
    }
