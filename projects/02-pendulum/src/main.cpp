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
Camera camera(glm::vec3(0.5f, 0.5f, 1.5f));
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

void print(glm::vec3 v){
    std::cout << v.x << ", " << v.y << ", " << v.z << std::endl;
};

int main(int argc, char** argv)
{
    std::string title = "PhysicsSim";
    unsigned int width = 1920;
    unsigned int height = 1080;
    GLFWwindow *window = setupGL(title, width, height);

    Shader blockShader(block_v_shader.c_str(), block_f_shader.c_str());

    Shader lightShader(light_v_shader.c_str(), light_f_shader.c_str());

    // Create a cube -> light and box to keep balls inside
    MeshCube mesh_cube;
    // Create a sphere -> object moving
    MeshSphere mesh_sphere;

    // Define light stuff
    Cube light_cube;
    light_cube.pos = glm::vec3(0.0f, 0.6f, 0.0f);
    light_cube.color = glm::vec3(1.0f, 0.5f, 0.31f);
    glm::vec3 lightColor(1.0f, 1.0f, 1.0f);

    Cube axle(glm::vec3(0.02f, 1.0f, 0.02f));
    axle.pos = glm::vec3(0.0f, -0.5f, 0.0f);
    axle.color = glm::vec3(1.0f, 0.5f, 0.31f);


    Sphere sphere;
    sphere.pos = glm::normalize(glm::vec3(0.7f, 0.7f, 0.0f));
    sphere.vel = glm::vec3(-1.0f, -1.0f, 1.0f);
    sphere.color = glm::linearRand(glm::vec3(0.0f), glm::vec3(1.0f));
    sphere.radius = 0.05;
    sphere.m = M_PI * sphere.radius * sphere.radius;

    // Transforms
    glm::mat4 proj;
    glm::mat4 view;
    glm::mat4 model;

    glEnable(GL_DEPTH_TEST);
    unsigned int n_substeps = 100;

    glm::vec3 center(0.0f, 0.0f, 0.0f);
    float radius = 1.0f;

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
        light_cube.pos.x = amp * sin(time);
        light_cube.pos.z = amp * cos(time);

        // Select shader program and set uniforms
        // Create substeps for stability
        float dt = deltaTime / n_substeps;
        blockShader.use();
        blockShader.set3f("lightColor", lightColor);
        blockShader.set3f("lightPos", light_cube.pos);
        blockShader.set3f("viewPos", camera.Position);
        blockShader.setMat4f("proj", proj);
        blockShader.setMat4f("view", view);

        for (unsigned int step=0; step!=n_substeps; ++step){
            // plot constraint
            model = glm::mat4(1.0f);
            model = glm::translate(model, center);
            model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f) * radius);
            blockShader.setMat4f("model", model);
            blockShader.set3f("objectColor", glm::vec3(0.3, 0.5, 0.5));
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            mesh_sphere.Draw();
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

            // plot dynamics
            model = glm::mat4(1.0f);
            model = glm::translate(model, sphere.pos);
            model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f) * sphere.radius);
            blockShader.setMat4f("model", model);
            blockShader.set3f("objectColor", sphere.color);
            mesh_sphere.Draw();

            // Move the ball i.e. update position and speed
            move(sphere, dt, center, radius);

            // Draw "axle"
            axle.pos = center + (sphere.pos - center) / 2.0f;
            glm::vec3 cross = glm::cross(glm::vec3(0.0f, -1.0f, 0.0f), sphere.pos - center);
            glm::vec3 rotAx = glm::normalize(cross);
            float angle = glm::asin( glm::length(cross) / ( 1 * glm::length(sphere.pos - center) )  );
            if (sphere.pos.y > center.y)
                angle = glm::radians(180.0f) - angle;

            model = glm::mat4(1.0f);
            model = glm::translate(model, axle.pos);
            model = glm::rotate(model, angle, rotAx);
            model = glm::scale(model, axle.size);
            blockShader.setMat4f("model", model);
            blockShader.set3f("objectColor", axle.color);
            mesh_cube.Draw();
        }

        // Draw the light!
        lightShader.use();
        model = glm::mat4(1.0f);
        model = glm::translate(model, light_cube.pos);
        model = glm::scale(model, glm::vec3(0.2f));
        lightShader.set3f("lightColor", lightColor);
        lightShader.setMat4f("model", model);
        lightShader.setMat4f("proj", proj);
        lightShader.setMat4f("view", view);
        mesh_cube.Draw();

        // swap buffers and poll IO events (key pressed/released, ...)
        // -----------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // glfw: terminate, clear all previous allocated GLFW resources
    // ------------------------------------------------------------
    glfwDestroyWindow(window);
    // glfwTerminate();
    return 0;
}
