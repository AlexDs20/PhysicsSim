#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtx/norm.hpp>

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
#include "ray.hpp"


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


int main(int argc, char** argv)
{
    std::string title = "PhysicsSim";
    unsigned int width = 1920;
    unsigned int height = 1080;
    GLFWwindow *window = setupGL(title, width, height);

    Shader blockShader(block_v_shader.c_str(), block_f_shader.c_str());

    Shader lightShader(light_v_shader.c_str(), light_f_shader.c_str());

    // Create a cube -> light
    MeshCube mesh_cube;

    // Create a sphere -> object moving
    MeshSphere mesh_sphere;

    // Define light stuff
    Cube light_cube;
    light_cube.pos = glm::vec3(0.0f, 0.6f, 0.0f);
    light_cube.color = glm::vec3(1.0f, 0.5f, 0.31f);
    glm::vec3 lightColor(1.0f, 1.0f, 1.0f);

    Cube axle(glm::vec3(0.05f, 1.0f, 0.05f));
    axle.pos = glm::vec3(0.0f, -0.5f, 0.0f);
    axle.color = glm::vec3(1.0f, 0.5f, 0.31f);


    unsigned int n_spheres = 8;
    std::vector<Sphere> spheres(n_spheres);

    for (std::vector<Sphere>::iterator it=spheres.begin(); it != spheres.end(); ++it){
        it->pos = glm::ballRand(1.0f);
        it->prev_pos = it->pos;

        // Set other spheres properties
        it->vel = glm::ballRand(0.5f);
        it->color = glm::linearRand(glm::vec3(0.0f), glm::vec3(1.0f));
        it->radius = glm::linearRand(0.1f, 0.2f);
        it->m = (4/3) * M_PI * it->radius * it->radius * it->radius;
    }

    // Transforms
    glm::mat4 proj;
    glm::mat4 view;
    glm::mat4 model;

    glEnable(GL_DEPTH_TEST);
    unsigned int n_substeps = 100;

    Sphere *ClickedObject = nullptr;
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

        // Find object if clicked and move it accordingly
        glm::vec3 clicked_object_pos;
        if ((camera.ray != nullptr) && (ClickedObject == nullptr)){
            // TODO: Check for intersection with correct object
            ClickedObject = ObjectRayCast(*camera.ray, spheres);
            //ClickedObject = &sphere;
            clicked_object_pos = ClickedObject->pos;
        }

        if ((camera.ray != nullptr) && (ClickedObject != nullptr)){
            // Move object to mouse position in the plane whose normal goes through the object position and perpendicular to the camera looking direction
            float beta = glm::length(clicked_object_pos - camera.Position);
            ClickedObject->pos = camera.Position + beta * camera.Front;
        }
        if ((camera.ray == nullptr) && (ClickedObject != nullptr))
            ClickedObject = nullptr;

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
        blockShader.use();
        blockShader.set3f("lightColor", lightColor);
        blockShader.set3f("lightPos", light_cube.pos);
        blockShader.set3f("viewPos", camera.Position);
        blockShader.set3f("objectColor", spheres[0].color);
        blockShader.setMat4f("proj", proj);
        blockShader.setMat4f("view", view);

        float dt = deltaTime / n_substeps;
        for (unsigned int step=0; step!=n_substeps; ++step){
            // move the spheres
            for (std::vector<Sphere>::iterator it = spheres.begin(); it != spheres.end(); ++it){
                it->prev_pos = it->pos;
                // move(sphere, dt);

                // Plot the spheres
                model = glm::mat4(1.0f);
                model = glm::translate(model, it->pos);
                model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f) * it->radius);
                blockShader.setMat4f("model", model);
                mesh_sphere.Draw();

                // Update velocities
                it->vel = (it->pos - it->prev_pos) / dt;
            }
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
