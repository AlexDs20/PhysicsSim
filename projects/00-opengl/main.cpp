#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>


// Callback to resize Viewport
void framebuffer_size_callback(GLFWwindow* window, int width, int height);

// Process Input and how to handle it
void processInput(GLFWwindow* window);


int main(int argc, char** argv)
{
    // glfw: initialize and configure
    //      Use glfw3.3
    //      Use only core functionalities
    // ----------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(800, 600, "LearnOpenGL", NULL, NULL);

    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent( window );

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // Register callback after window is created and before render loop
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // process inputs
        processInput(window);

        // rendering commands
        // state setting color
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        // state using color
        glClear(GL_COLOR_BUFFER_BIT);

        // swap buffers and poll IO events (key pressed/released, ...)
        // -----------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // glfw: terminate, clear all previous allocated GLFW resources
    // ------------------------------------------------------------
    glfwTerminate();
    return 0;
}


void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window)
{
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}
