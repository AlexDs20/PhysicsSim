project "01-bouncing_ball"
    kind "WindowedApp"

    includedirs
    {
        "../../deps/glad/include",
        "../../deps/glfw/include",
        "../../deps/glm",
        "include"
    }

    files "src/**"

    links { "GLAD", "GLFW", "GLM" }
