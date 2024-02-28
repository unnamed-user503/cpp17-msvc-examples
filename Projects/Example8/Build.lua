project "Example8"
    kind          "ConsoleApp" -- ConsoleApp WindowedApp SharedLib StaticLib
    language      "C++"
    cppdialect    "C++17"
    staticruntime "off"
    characterset  "MBCS"

    targetdir ("%{wks.location}/Binaries/" .. OutputDir .. "/%{prj.name}")
    objdir    ("%{wks.location}/Binaries-Intermediates/" .. OutputDir .. "/%{prj.name}")

    vspropertysettings
    {
        VcpkgTriplet = "x64-windows-static",
        VcpkgEnabled = "true",
    }

    files
    {
        "Example8.exe.manifest", "Source/**.cpp", "Source/**.hpp", "Source/**.h",
    }

    includedirs
    {
        "Source",
    }

    links
    {
        "mfplat.lib", "mfreadwrite.lib", "mfuuid.lib",
    }