project "libvorbis"
    kind          "StaticLib" -- ConsoleApp WindowedApp SharedLib StaticLib
    language      "C"
    warnings      "off"
    staticruntime "off"
    characterset  "MBCS"

    targetdir ("%{wks.location}/Binaries/" .. OutputDir .. "/%{prj.name}")
    objdir    ("%{wks.location}/Binaries-Intermediates/" .. OutputDir .. "/%{prj.name}")

    vspropertysettings
    {
        VcpkgTriplet = "x64-windows-static",
        VcpkgEnabled = "true",
        PublicIncludeDirectories = "include",
    }

    defines
    {
        "LIBOGG_EXPORTS"
    }

    files
    {
        "lib/**.c", "lib/**.h", "include/**.h",
    }

    excludes
    {
        "lib/barkmel.c", "lib/psytune.c", "lib/tone.c"
    }

    includedirs
    {
        "include",
    }

    links
    {
        "libogg"
    }
