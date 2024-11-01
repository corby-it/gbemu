
project_name = "gbemu"
app_name = project_name
tests_name = project_name .. "-tests"


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


    configurations { "debug", "release", "profiling", "coverage" }
    architecture "x86_64"
    location(build_base)

    targetdir(build_base .. "/bin/%{cfg.buildcfg}")

    -- the only files common to both projects are the ones related
    -- to gameboy core emulation
    files {
        src_base .. "/gb/**.cpp",
        src_base .. "/gb/**.h",
        src_base .. "/gbdebug/**.h",
        src_base .. "/gbdebug/**.cpp",
    }

    includedirs {
        src_base,
    }

    externalincludedirs {
        src_base .. "/third-party",
        src_base .. "/third-party/cereal-1.3.2/include",
        src_base .. "/third-party/tracy/public",
    }

    defines {
        -- PROJECT_NAME will be defined for each project to have the correct name
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

    filter { "system:linux", "action:gmake" }
        buildoptions { 
            "-pedantic",
            "-Wno-psabi",
            "-Wno-format-security",
            "-fmessage-length=0",
            "-fsigned-char",
        }

    filter { "system:windows", "action:vs*" }
        startproject(project_name)
        systemversion "latest"
        staticruntime "On"
        defines { 
            "_CRT_SECURE_NO_WARNINGS" 
        }
    
    filter { "system:windows", "action:vs*", "configurations:debug" }
        ignoredefaultlibraries {
            "LIBCMT"
        }


    project(app_name)
        kind "WindowedApp"
        language "C++"
        cppdialect "C++17"

        -- code coverage is only meaningful when running tests
        removeconfigurations { "coverage" }

        defines {
            "PROJECT_NAME=" .. app_name,
        }
        
        -- the app main.cpp is in the app directory
        -- add imgui + glfw files
        files {
            src_base .. "/app/**.cpp",
            src_base .. "/app/**.h",
            src_base .. "/imgui/*.cpp",
            src_base .. "/imgui/*.h",
            src_base .. "/imgui/backends/imgui_impl_glfw.cpp",
            src_base .. "/imgui/backends/imgui_impl_opengl3.cpp",
            src_base .. "/third-party/ImGuiFileDialog/ImGuiFileDialog.cpp",
        }

        externalincludedirs {
            src_base .. "/imgui",
            src_base .. "/imgui/backends",
        }

        filter { "configurations:profiling" }
            -- symbols must be on to get meaningful traces when profiling
            symbols "On"
            optimize "Speed"
            files {
                src_base .. "/third-party/tracy/public/TracyClient.cpp",
            }
            defines { 
                "TRACY_ENABLE",
                "TRACY_NO_SAMPLING",
            }

        filter { "system:linux", "configurations:profiling"}
            -- TracyClient.cpp will produce some warnings when compiled with gcc
            -- so we have to disable the fatal warnings flag
            removeflags { "FatalWarnings" } 

        -- add compile and link flags for GLFW
        filter { "system:linux", "action:gmake" }
            buildoptions { 
                "-Wno-unknown-pragmas", -- required to compile ImGuiFileDialog
                "`pkg-config --cflags glfw3`"
            }
            linkoptions {
                "`pkg-config --static --libs glfw3`"
            }
            links { 
                "GL"
            }

        filter { "system:windows", "action:vs*" }
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


    project(tests_name)
        kind "ConsoleApp"
        language "C++"
        cppdialect "C++17"
        
        removeconfigurations { "profiling" }

        defines {
            "PROJECT_NAME=" .. tests_name
        }

        -- tests, along with their main, are all located under src/tests
        files {
            src_base .. "/tests/**.cpp"
        }

        includedirs {
            src_base .. "/tests"
        }

        -- code coverage uses gcov + lcov which is not available on windows
        filter { "system:windows" }
            removeconfigurations { "coverage" }

        filter { "system:linux", "action:gmake", "configurations:coverage" }
            symbols "On"
            optimize "debug"
            buildoptions { "--coverage" }
            linkoptions { "--coverage" }
