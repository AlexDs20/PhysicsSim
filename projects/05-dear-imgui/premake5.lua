project "05-dear-imgui"
    kind "WindowedApp"

    includedirs
    {
        "../../deps/glad/include",
        "../../deps/glfw/include",
        "../../deps/glm",
        "../../deps/imgui",
        "../../deps/imgui/backends",
        "include"
    }

    files "src/**.cpp"

    links { "GLAD", "GLFW", "GLM", "IMGUI" }
