project "IMGUI"
    kind "StaticLib"
    language "C++"
    architecture "x86_64"

    includedirs
    {
        "imgui/",
        "imgui/backends/",
        "imgui/misc/cpp/"
    }

    files
    {
        "imgui/*.cpp",
        -- "imgui/*.h",
        -- "imgui/backends/imgui_impl_opengl3.h",
        "imgui/backends/imgui_impl_opengl3.cpp",
        -- "imgui/backends/imgui_impl_glfw.h",
        "imgui/backends/imgui_impl_glfw.cpp",
        -- "imgui/misc/cpp/imgui_stdlib.h",
        "imgui/misc/cpp/imgui_stdlib.cpp",
    }

    filter "configurations:Debug"
        runtime "Debug"
        symbols "On"

    filter "configurations:Release"
        runtime "Release"
        optimize "On"

