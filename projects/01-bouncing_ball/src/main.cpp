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


struct Sphere {
    glm::vec3 pos;
    glm::vec3 vel;
    glm::vec3 color;
    float radius;
    float m;
};

float energy(glm::vec3 pos, glm::vec3 v){
    glm::vec3 g(0.0f, -10.f, 0.0f);
    return -g.y*pos.y + 0.5 * dot(v, v);
};


void collision(Sphere& s1, Sphere& s2){
    // elasticity
    float e = 1.00f;

    glm::vec3 normal(s2.pos - s1.pos);
    float d = glm::length(normal);

    if (d > s1.radius + s2.radius)
        return;

    normal /= d;

    float corr = (s1.radius + s2.radius - d) / 2.0f;

    s1.pos -= corr * normal;
    s2.pos += corr * normal;

    float v1n = glm::dot(s1.vel, normal);
    float v2n = glm::dot(s2.vel, normal);

    float v1n_new = ((s1.m * v1n) + (s2.m * v2n) - s2.m * (v1n - v2n) * e)/(s1.m + s2.m);
    float v2n_new = ((s1.m * v1n) + (s2.m * v2n) - s1.m * (v2n - v1n) * e)/(s1.m + s2.m);

    glm::vec3 v1t = s1.vel - glm::dot(s1.vel, normal) * normal;
    glm::vec3 v2t = s2.vel - glm::dot(s2.vel, normal) * normal;

    s1.vel = v1t + v1n_new * normal;
    s2.vel = v2t + v2n_new * normal;
};


void move(Sphere& s, float dt, glm::vec3 centerBox){
    /*
     F = ma
     F = m dv/dt
        dv = (F/m) * dt
        vi - vi-1 = (F/m) * dt
        vi = vi-1 + (F/m) * dt
    v = dx/dt
        dx = v * dt
        xi = xi-1 + v * dt
    */
    glm::vec3 g(0.0f, -10.f, 0.0f);
    float dh;

    s.vel += g * dt;
    s.pos += s.vel * dt;

    if (s.pos.y - s.radius < centerBox.y - 0.5){
        // Remove the potential energy gained from the velocity
        // dE = 0 => mgdh = -mvdv => dv = -gdh/v => v = v - gdh/v
        dh = centerBox.y - 0.5 - (s.pos.y - s.radius) ;
        s.vel.y -= g.y * dh / s.vel.y;
        s.pos.y = centerBox.y - 0.5 + s.radius;
        s.vel.y = -s.vel.y;
    }
    if (s.pos.x + s.radius > centerBox.x + 0.5 ){
        s.pos.x = centerBox.x + 0.5 - s.radius;
        s.vel.x = -s.vel.x;
    } else if (s.pos.x - s.radius < centerBox.x - 0.5){
        s.pos.x = centerBox.x - 0.5 + s.radius;
        s.vel.x = -s.vel.x;
    }
    if (s.pos.z + s.radius > centerBox.z + 0.5 ){
        s.pos.z = centerBox.z + 0.5 - s.radius;
        s.vel.z = -s.vel.z;
    } else if (s.pos.z - s.radius < centerBox.z - 0.5){
        s.pos.z = centerBox.z - 0.5 + s.radius;
        s.vel.z = -s.vel.z;
    }
}


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
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "PhysicsSim", NULL, NULL);

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

    // Create a cube -> light and box to keep balls inside
    Cube cube;

    std::vector<glm::vec3> cubePositions = {
        glm::vec3(0.0f,  0.0f,  0.0f),
    };

    // Define light stuff
    glm::vec3 lightPos(0.0f, 0.6f, 0.0f);
    glm::vec3 lightColor(1.0f, 1.0f, 1.0f);

    glm::vec3 blockColor(1.0f, 0.5f, 0.31f);

    Sphere2 sphere;

    unsigned int n_spheres = 50;
    std::vector<Sphere> spheres(n_spheres);

    for (unsigned int i=0; i!= n_spheres; ++i){
        spheres[i].pos = glm::ballRand(0.25f);         // The box is centered at (0,0,0) and side of size of 0.5.
        spheres[i].vel = glm::ballRand(0.25f);
        spheres[i].color = glm::ballRand(0.5f)+0.5f;
        spheres[i].radius = 0.05;
        spheres[i].m = M_PI * spheres[i].radius * spheres[i].radius;
    }

    // Transforms
    glm::mat4 proj;
    glm::mat4 view;
    glm::mat4 model;

    glEnable(GL_DEPTH_TEST);

    unsigned int n_substeps = 5;

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

        // Go through each box to render
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        for (std::vector<glm::vec3>::size_type i=0; i!=cubePositions.size(); ++i)
        {
            model = glm::mat4(1.0f);
            model = glm::translate(model, cubePositions[i]);
            blockShader.setMat4f("model", model);
            cube.Draw();
        }
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        // Sphere
        // Use same shader as for block
        for (std::vector<Sphere>::size_type i = 0; i!=spheres.size(); ++i){
            // Draw
            model = glm::mat4(1.0f);
            model = glm::translate(model, spheres[i].pos);
            model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f) * spheres[i].radius);
            blockShader.setMat4f("model", model);
            blockShader.set3f("objectColor", spheres[i].color);
            sphere.Draw();

            // Create substeps for stability
            float dt = deltaTime / n_substeps;
            for (unsigned int step=0; step!= n_substeps; ++step){
                // Move the ball i.e. update position and speed
                move(spheres[i], dt, cubePositions[0]);

                // Check for collisions
                for (std::vector<Sphere>::size_type j=0; j != spheres.size(); ++j){
                    if (i != j)
                        collision(spheres[i], spheres[j]);
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
