
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


    configurations { "debug", "release", "profiling" }
    architecture "x86_64"
    location(build_base)

    targetdir(build_base .. "/bin/%{cfg.buildcfg}")

    -- don't include main.cpp here, every prject will add its own
    files {
        src_base .. "/gb/**.cpp",
        src_base .. "/gb/**.h",
        src_base .. "/app/**.cpp",
        src_base .. "/app/**.h",
        src_base .. "/gbdebug/**.h",
        src_base .. "/gbdebug/**.cpp",
        src_base .. "/imgui/*.cpp",
        src_base .. "/imgui/*.h",
        src_base .. "/imgui/backends/imgui_impl_glfw.cpp",
        src_base .. "/imgui/backends/imgui_impl_opengl3.cpp",
        src_base .. "/third-party/ImGuiFileDialog/ImGuiFileDialog.cpp",
        src_base .. "/third-party/tracy/public/TracyClient.cpp",
    }

    includedirs {
        src_base,
    }

    externalincludedirs {
        src_base .. "/imgui",
        src_base .. "/imgui/backends",
        src_base .. "/third-party",
        src_base .. "/third-party/tracy/public",
        src_base .. "/third-party/cereal-1.3.2/include",
    }

    defines {
        "PROJECT_NAME=" .. project_name,
        "MAJOR_VER=" ..  _OPTIONS["major_ver"],
        "MINOR_VER=" .. _OPTIONS["minor_ver"],
        "BUILD_VER=" .. _OPTIONS["build_ver"],
    }

    warnings "Extra"
    flags { "FatalWarnings" }
    

    filter "configurations:debug"
        symbols "On"
        optimize "debug"
        debugargs {
            "--test-case-exclude=\"Mooneye tests*\"",
        }

    filter "configurations:release"
        symbols "Off"
        optimize "Speed"
        flags {
            "LinkTimeOptimization"
        }
        defines { 
            "DOCTEST_CONFIG_DISABLE" 
        }

    filter "configurations:profiling"
        symbols "On"
        optimize "Speed"
        flags {
            "LinkTimeOptimization"
        }
        defines { 
            "DOCTEST_CONFIG_DISABLE",
            "TRACY_ENABLE",
            "TRACY_NO_SAMPLING",
        }

    filter { "system:linux", "action:gmake" }
        buildoptions { 
            "-pedantic",
            "-Wno-psabi",
            "-Wno-unknown-pragmas", -- required to compile ImGuiFileDialog
            "-Wno-format-security",
            "-fmessage-length=0",
            "-fsigned-char",
            "-ffunction-sections",
            "-fdata-sections",
            "`pkg-config --cflags glfw3`"
        }
        linkoptions {
            "-Wl,--gc-sections",
            "`pkg-config --static --libs glfw3`"
        }
        links { 
            "GL"
        }

    filter { "system:windows", "action:vs*" }
        startproject(project_name)
        externalincludedirs {
            "$(GLFW_PATH)/include"
        }
        libdirs {
            "$(GLFW_PATH)/lib-vc2022"
        }
        links {
            "opengl32.lib",
            "glfw3_mt.lib"
        }
        systemversion "latest"
        staticruntime "On"
        defines { 
            "_CRT_SECURE_NO_WARNINGS" 
        }
    
    filter { "system:windows", "action:vs*", "configurations:debug" }
        ignoredefaultlibraries {
            "LIBCMT"
        }


    project(project_name)
        kind "WindowedApp"
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

        includedirs {
            src_base .. "/tests"
        }
    