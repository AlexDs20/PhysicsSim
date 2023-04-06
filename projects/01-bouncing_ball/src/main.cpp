#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <filesystem>
#include <iostream>
#include <string.h>
#include <math.h>
#include <vector>

#include "shader.hpp"
#include "camera.hpp"
#include "utils.hpp"


const unsigned int SCR_WIDTH = 1024;
const unsigned int SCR_HEIGHT = 768;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 5.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// Frame time (so that movement is same regardless of fps)
float deltaTime = 0.0f;
float lastFrame = 0.0f;


int main(int argc, char** argv)
{
    // ------------------------------------
    //      Init glfw, glad, window
    // ------------------------------------
    // glfw: initialize and configure
    // ----------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef _MAC
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT_GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Bouncing Ball", NULL, NULL);

    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent( window );
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);


    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    Shader blockShader("../projects/01-bouncing_ball/resources/shaders/vertex.vs", \
            "../projects/01-bouncing_ball/resources/shaders/fragment.fs");

    Shader lightShader("../projects/01-bouncing_ball/resources/shaders/light_cube.vs", \
            "../projects/01-bouncing_ball/resources/shaders/light_cube.fs");

    // ------------------------------------
    //          SHAPE
    // ------------------------------------
    float vertices[] =
    {
        -0.5f, -0.5f, -0.5f,   0.0f,  0.0f, -1.0f,         // Front, bottom first up then     2---3
         0.5f, -0.5f, -0.5f,   0.0f,  0.0f, -1.0f,         //                                 | \ |
        -0.5f,  0.5f, -0.5f,   0.0f,  0.0f, -1.0f,         //                                 0---1
         0.5f,  0.5f, -0.5f,   0.0f,  0.0f, -1.0f,

        -0.5f, -0.5f,  0.5f,   0.0f,  0.0f,  1.0f,         // Back, bottom first up then
         0.5f, -0.5f,  0.5f,   0.0f,  0.0f,  1.0f,
        -0.5f,  0.5f,  0.5f,   0.0f,  0.0f,  1.0f,
         0.5f,  0.5f,  0.5f,   0.0f,  0.0f,  1.0f,

        -0.5f,  0.5f, -0.5f,   0.0f,  1.0f,  0.0f,          // TOP
         0.5f,  0.5f, -0.5f,   0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f,  0.5f,   0.0f,  1.0f,  0.0f,
         0.5f,  0.5f,  0.5f,   0.0f,  1.0f,  0.0f,

        -0.5f,  -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,           // Bottom
         0.5f,  -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
        -0.5f,  -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
         0.5f,  -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,

         -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,          //  LEFT
         -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
         -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
         -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,

          0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,          //  Right
          0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
          0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
          0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
    };

    unsigned int indices[] = {
        0, 1, 2, // first triangle
        1, 2, 3, // second triangle

        4, 5, 6,
        5, 6, 7,

        // 8, 9, 10,
        // 9, 10, 11,

        12, 13, 14,
        13, 14, 15,

        16, 17, 18,
        17, 18, 19,

        20, 21, 22,
        21, 22, 23,
    };

    std::vector<glm::vec3> cubePositions = {
        glm::vec3( 0.0f,  0.0f,  0.0f),
    };


    unsigned int VBO, EBO, VAO;

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*) (3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // light
    unsigned int lightVAO;
    glGenVertexArrays(1, &lightVAO);
    glBindVertexArray(lightVAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // wireframe mode, good for debugging
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    // glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    // Define light stuff
    glm::vec3 lightPos(0.0f, 1.5f, 0.0f);
    glm::vec3 lightColor(1.0f, 1.0f, 1.0f);

    glm::vec3 blockColor(1.0f, 0.5f, 0.31f);

    // Transforms
    glm::mat4 proj;
    glm::mat4 view;
    glm::mat4 model;

    glEnable(GL_DEPTH_TEST);

    struct Material {
        glm::vec3 Position;
        glm::vec3 Velocity;
    };

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        float time = glfwGetTime();
        deltaTime = time - lastFrame;
        lastFrame = time;

        // process inputs
        processInput(window);

        // rendering commands
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Set camera transforms
        proj = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        view = camera.GetViewMatrix();

        // Rotate the light
        float amp = 1.5f;
        lightPos.x = amp * sin(time);
        lightPos.z = amp * cos(time);

        // Select shader program, bind VAO and draw!
        blockShader.use();
        blockShader.set3f("objectColor", blockColor);
        blockShader.set3f("lightColor", lightColor);
        blockShader.set3f("lightPos", lightPos);
        blockShader.set3f("viewPos", camera.Position);
        blockShader.setMat4f("proj", proj);
        blockShader.setMat4f("view", view);

        glBindVertexArray(VAO);
        // Go through each box to render
        for (std::vector<glm::vec3>::size_type i=0; i!=cubePositions.size(); ++i)
        {
            model = glm::mat4(1.0f);
            model = glm::translate(model, cubePositions[i]);
            model = glm::scale(model, glm::vec3(3.0f, 1.0f, 3.0f));
            // float angle = 20.0f * (i+1);
            // model = glm::rotate(model, glm::radians(angle*time), glm::normalize(glm::vec3(0.4f, 1.0f, 0.3f)));
            blockShader.setMat4f("model", model);
            glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
        }

        // Position the light!
        lightShader.use();
        model = glm::mat4(1.0f);
        model = glm::translate(model, lightPos);
        model = glm::scale(model, glm::vec3(0.2f));
        lightShader.set3f("lightColor", lightColor);
        lightShader.setMat4f("model", model);
        lightShader.setMat4f("proj", proj);
        lightShader.setMat4f("view", view);
        glBindVertexArray(lightVAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

        // swap buffers and poll IO events (key pressed/released, ...)
        // -----------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // glfw: terminate, clear all previous allocated GLFW resources
    // ------------------------------------------------------------
    glDeleteVertexArrays(1, &lightVAO);
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
