#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/random.hpp>

#include <filesystem>
#include <iostream>
#include <string.h>
#include <math.h>
#include <vector>

#include "shader.hpp"
#include "camera.hpp"
#include "utils.hpp"
#include "object.hpp"
#include "setupGL.hpp"
#include "physics.hpp"


const unsigned int SCR_WIDTH = 1920;
const unsigned int SCR_HEIGHT = 1080;

// camera
Camera camera(glm::vec3(0.5f, 0.5f, 0.5f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// Frame time (so that movement is same regardless of fps)
float deltaTime = 0.0f;
float lastFrame = 0.0f;


const std::string block_v_shader("../projects/01-bouncing_ball/resources/shaders/vertex.vs");
const std::string block_f_shader("../projects/01-bouncing_ball/resources/shaders/fragment.fs");
const std::string light_v_shader("../projects/01-bouncing_ball/resources/shaders/light_cube.vs");
const std::string light_f_shader("../projects/01-bouncing_ball/resources/shaders/light_cube.fs");

int main(int argc, char** argv)
{
    std::string title = "PhysicsSim";
    unsigned int width = 1920;
    unsigned int height = 1080;
    GLFWwindow *window = setupGL(title, width, height);

    Shader blockShader(block_v_shader.c_str(), block_f_shader.c_str());

    Shader lightShader(light_v_shader.c_str(), light_f_shader.c_str());

    // Create a cube -> light and box to keep balls inside
    Cube cube;

    glm::vec3 cubePosition(0.0f,  0.0f,  0.0f);

    // Define light stuff
    glm::vec3 lightPos(0.0f, 0.6f, 0.0f);
    glm::vec3 lightColor(1.0f, 1.0f, 1.0f);
    glm::vec3 blockColor(1.0f, 0.5f, 0.31f);

    Sphere2 sphere;

    unsigned int n_spheres = 200;
    std::vector<Sphere> spheres(n_spheres);

    double xmin=-0.5f, xmax=0.5f;         // The box is centered at (0,0,0) and side of size of 0.5.
    double vmin=-0.01f, vmax=0.01f;

    for (std::vector<Sphere>::iterator it = spheres.begin(); it!=spheres.end(); ++it){
        it->pos = glm::linearRand(glm::vec3(xmin), glm::vec3(xmax));
        it->vel = glm::linearRand(glm::vec3(vmin), glm::vec3(vmax));
        it->color = glm::linearRand(glm::vec3(0.0f), glm::vec3(1.0f));
        it->radius = 0.05;
        it->m = M_PI * it->radius * it->radius;
    }

    // Transforms
    glm::mat4 proj;
    glm::mat4 view;
    glm::mat4 model;


    glEnable(GL_DEPTH_TEST);
    unsigned int n_substeps = 10;

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        float time = glfwGetTime();
        deltaTime = time - lastFrame;
        lastFrame = time;

        // Time for frame
        std::streamsize prec = std::cout.precision();
        std::cout << std::setprecision(5) << deltaTime << " ms\r" << std::setprecision(prec) << std::flush;

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

        // Select shader program and set uniforms
        blockShader.use();
        blockShader.set3f("objectColor", blockColor);
        blockShader.set3f("lightColor", lightColor);
        blockShader.set3f("lightPos", lightPos);
        blockShader.set3f("viewPos", camera.Position);
        blockShader.setMat4f("proj", proj);
        blockShader.setMat4f("view", view);

        // Render the box as see-through
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        model = glm::mat4(1.0f);
        model = glm::translate(model, cubePosition);
        blockShader.setMat4f("model", model);
        cube.Draw();
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        // Sphere
        // Use same shader as for block
        for (std::vector<Sphere>::iterator it=spheres.begin(); it!=spheres.end(); ++it){
            // Draw
            model = glm::mat4(1.0f);
            model = glm::translate(model, it->pos);
            model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f) * it->radius);
            blockShader.setMat4f("model", model);
            blockShader.set3f("objectColor", it->color);
            sphere.Draw();

            // Create substeps for stability
            float dt = deltaTime / n_substeps;
            for (unsigned int step=0; step!= n_substeps; ++step){
                // Move the ball i.e. update position and speed
                move(*it, dt, cubePosition);

                // Check for collisions
                for (std::vector<Sphere>::iterator it2=spheres.begin(); it2!=spheres.end(); ++it2){
                    if (it != it2)
                        collision(*it, *it2);
                }
            }
        }

        // Draw the light!
        lightShader.use();
        model = glm::mat4(1.0f);
        model = glm::translate(model, lightPos);
        model = glm::scale(model, glm::vec3(0.2f));
        lightShader.set3f("lightColor", lightColor);
        lightShader.setMat4f("model", model);
        lightShader.setMat4f("proj", proj);
        lightShader.setMat4f("view", view);
        cube.Draw();

        // swap buffers and poll IO events (key pressed/released, ...)
        // -----------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // glfw: terminate, clear all previous allocated GLFW resources
    // ------------------------------------------------------------
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
