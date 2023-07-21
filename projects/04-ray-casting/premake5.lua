project "04-ray-casting"
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
