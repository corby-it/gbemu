
project_name = "gbemu"


workspace(project_name)
    src_base = "src"
    build_base = "build"

    -- version numbers can be passed to this script as parameters (for example from a build system)
    newoption {
        trigger     = "major_ver",
        value       = "n",
        description = "Major version number",
        default     = "0"
    }
    newoption {
        trigger     = "minor_ver",
        value       = "n",
        description = "Minor version number",
        default     = "1"
    }
    newoption {
        trigger     = "build_ver",
        value       = "n",
        description = "Build version number",
        default     = "0"
    }


    configurations { "debug", "release" }
    architecture "x86_64"
    location(build_base)

    targetdir(build_base .. "/bin/%{cfg.buildcfg}")

    -- don't include the main file here, every prject will add its own
    files {
        src_base .. "/gb/**.cpp",
        src_base .. "/gb/**.h",
        src_base .. "/ui/**.cpp",
        src_base .. "/ui/**.h",
        src_base .. "/imgui/*.cpp",
        src_base .. "/imgui/*.h",
        src_base .. "/imgui/backends/imgui_impl_glfw.cpp",
        src_base .. "/imgui/backends/imgui_impl_opengl3.cpp"
    }

    includedirs {
        src_base,
    }

    externalincludedirs {
        src_base .. "/imgui",
        src_base .. "/imgui/backends",
    }

    defines {
        "PROJECT_NAME=" .. project_name,
        "MAJOR_VER=" ..  _OPTIONS["major_ver"],
        "MINOR_VER=" .. _OPTIONS["minor_ver"],
        "BUILD_VER=" .. _OPTIONS["build_ver"]
    }

    flags {
        "FatalWarnings"
    }
    

    filter "configurations:debug"
        symbols "On"
        optimize "debug"

    filter "configurations:release"
        symbols "Off"
        optimize "Speed"
        flags {
            "LinkTimeOptimization"
        }
        defines { "DOCTEST_CONFIG_DISABLE" }

    filter { "system:linux", "action:gmake" }
        buildoptions { 
            "-Wall",
            "-Wextra",
            "-pedantic",
            "-Werror",
            "-Wno-psabi",
            "-fmessage-length=0",
            "-fsigned-char",
            "-ffunction-sections",
            "-fdata-sections",
            "-flto=auto",
            "`pkg-config --cflags glfw3`"
        }
        linkoptions {
            "-Wl,--gc-sections",
            "-flto=auto",
            "`pkg-config --static --libs glfw3`"
        }
        links { 
            "GL"
        }

    filter { "system:windows", "action:vs*" }
        externalincludedirs {
            "$(LIBRARIES_PATH)/glfw-3.3.8.bin.WIN64/include"
        }
        libdirs {
            "$(LIBRARIES_PATH)/glfw-3.3.8.bin.WIN64/lib-vc2022"
        }
        links {
            "opengl32.lib",
            "glfw3.lib"
        }
        warnings "Extra"


    project(project_name)
        kind "ConsoleApp"
        language "C++"
        cppdialect "C++17"
        
        -- use the regular main.cpp file
        files {
            src_base .. "/main.cpp"
        }

    project(project_name .. "-tests")
        kind "ConsoleApp"
        language "C++"
        cppdialect "C++17"

        -- tests, along with their main, are all located under src/tests
        files {
            src_base .. "/tests/**.cpp"
        }
    