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
    float e = 1.;

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
        glm::vec3(0.0f,  0.0f,  0.0f),
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
    glm::vec3 lightPos(0.0f, 0.6f, 0.0f);
    glm::vec3 lightColor(1.0f, 1.0f, 1.0f);

    glm::vec3 blockColor(1.0f, 0.5f, 0.31f);

    // Transforms
    glm::mat4 proj;
    glm::mat4 view;
    glm::mat4 model;

    glEnable(GL_DEPTH_TEST);

    // Create a sphere
    std::vector<float> s_vertices;
    std::vector<float> s_normals;
    std::vector<float> s_texCoords;

    unsigned int sectorCount = 32;
    unsigned int stackCount = 16;
    float radius = 1.0f;

    float x, y, z, xy;                              // vertex position
    float nx, ny, nz, lengthInv = 1.0f / radius;    // vertex normal
    float s, t;                                     // vertex texCoord

    float sectorStep = 2 * M_PI / sectorCount;
    float stackStep = M_PI / stackCount;
    float sectorAngle, stackAngle;

    for(int i = 0; i <= stackCount; ++i)
    {
        stackAngle = M_PI / 2 - i * stackStep;        // starting from pi/2 to -pi/2
        xy = radius * std::cos(stackAngle);             // r * cos(u)
        z = radius * std::sin(stackAngle);              // r * sin(u)

        // add (sectorCount+1) vertices per stack
        // the first and last vertices have same position and normal, but different tex coords
        for(int j = 0; j <= sectorCount; ++j)
        {
            sectorAngle = j * sectorStep;           // starting from 0 to 2pi

            // vertex position (x, y, z)
            x = xy * std::cos(sectorAngle);             // r * cos(u) * cos(v)
            y = xy * std::sin(sectorAngle);             // r * cos(u) * sin(v)
            s_vertices.push_back(x);
            s_vertices.push_back(y);
            s_vertices.push_back(z);

            // normalized vertex normal (nx, ny, nz)
            nx = x * lengthInv;
            ny = y * lengthInv;
            nz = z * lengthInv;
            s_vertices.push_back(nx);
            s_vertices.push_back(ny);
            s_vertices.push_back(nz);

            // vertex tex coord (s, t) range between [0, 1]
            s = (float)j / sectorCount;
            t = (float)i / stackCount;
            s_texCoords.push_back(s);
            s_texCoords.push_back(t);
        }
    }

    // generate CCW index list of sphere triangles
    // k1--k1+1
    // |  / |
    // | /  |
    // k2--k2+1
    std::vector<int> s_indices;
    std::vector<int> s_lineIndices;
    int k1, k2;
    for(int i = 0; i < stackCount; ++i)
    {
        k1 = i * (sectorCount + 1);     // beginning of current stack
        k2 = k1 + sectorCount + 1;      // beginning of next stack

        for(int j = 0; j < sectorCount; ++j, ++k1, ++k2)
        {
            // 2 triangles per sector excluding first and last stacks
            // k1 => k2 => k1+1
            if(i != 0)
            {
                s_indices.push_back(k1);
                s_indices.push_back(k2);
                s_indices.push_back(k1 + 1);
            }

            // k1+1 => k2 => k2+1
            if(i != (stackCount-1))
            {
                s_indices.push_back(k1 + 1);
                s_indices.push_back(k2);
                s_indices.push_back(k2 + 1);
            }

            // store indices for lines
            // vertical lines for all stacks, k1 => k2
            s_lineIndices.push_back(k1);
            s_lineIndices.push_back(k2);
            if(i != 0)  // horizontal lines except 1st stack, k1 => k+1
            {
                s_lineIndices.push_back(k1);
                s_lineIndices.push_back(k1 + 1);
            }
        }
    }

    unsigned int VBO_s, VAO_s, EBO_s;

    glGenVertexArrays(1, &VAO_s);
    glGenBuffers(1, &VBO_s);
    glGenBuffers(1, &EBO_s);

    glBindVertexArray(VAO_s);

    glBindBuffer(GL_ARRAY_BUFFER, VBO_s);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * s_vertices.size(), &s_vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_s);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(float) * s_indices.size(), &s_indices[0], GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*) (0 * sizeof(float)));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*) (3 * sizeof(float)));
    glEnableVertexAttribArray(1);


    std::vector<Sphere> spheres(5);

    spheres[0].pos = glm::vec3(0.0f, 0.5f, 0.0f);
    spheres[0].vel = glm::vec3(0.33f, 0.0f, 0.2f);
    spheres[0].color = glm::vec3(0.7f, 0.2f, 0.2f);
    spheres[0].radius = 0.05;
    spheres[0].m = M_PI * spheres[0].radius * spheres[0].radius;

    spheres[1].pos = glm::vec3(0.3f, 0.5f, 0.0f);
    spheres[1].vel = glm::vec3(0.33f, 0.0f, 0.2f);
    spheres[1].color = glm::vec3(0.3f, 0.6f, 0.3f);
    spheres[1].radius = 0.15;
    spheres[1].m = M_PI * spheres[1].radius * spheres[1].radius;

    spheres[2].pos = glm::vec3(0.3f, 0.1f, 0.0f);
    spheres[2].vel = glm::vec3(0.33f, 0.0f, 0.2f);
    spheres[2].color = glm::vec3(0.3f, 0.6f, 0.3f);
    spheres[2].radius = 0.02;
    spheres[2].m = M_PI * spheres[2].radius * spheres[2].radius;

    spheres[3].pos = glm::vec3(0.3f, 0.0f, 0.1f);
    spheres[3].vel = glm::vec3(0.33f, 0.0f, 0.2f);
    spheres[3].color = glm::vec3(0.3f, 0.6f, 0.3f);
    spheres[3].radius = 0.07;
    spheres[3].m = M_PI * spheres[3].radius * spheres[3].radius;

    spheres[4].pos = glm::vec3(0.2f, 0.0f, 0.2f);
    spheres[4].vel = glm::vec3(0.33f, 0.0f, 0.2f);
    spheres[4].color = glm::vec3(0.3f, 0.6f, 0.3f);
    spheres[4].radius = 0.1;
    spheres[4].m = M_PI * spheres[4].radius * spheres[4].radius;

    unsigned int n_substeps = 300;

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
            // model = glm::scale(model, glm::vec3(1.0f, 0.5f, 1.0f));
            // float angle = 20.0f * (i+1);
            // model = glm::rotate(model, glm::radians(angle*time), glm::normalize(glm::vec3(0.4f, 1.0f, 0.3f)));
            blockShader.setMat4f("model", model);
            glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
        }

        // Sphere
        // Use same shader as for block
        glBindVertexArray(VAO_s);

        for (std::vector<Sphere>::size_type i = 0; i!=spheres.size(); ++i){
            // Draw
            model = glm::mat4(1.0f);
            model = glm::translate(model, spheres[i].pos);
            model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f) * spheres[i].radius);
            blockShader.setMat4f("model", model);
            blockShader.set3f("objectColor", spheres[i].color);
            glDrawElements(GL_TRIANGLES, (unsigned int)s_indices.size(), GL_UNSIGNED_INT, 0);

            // Create substeps for stability
            float dt = deltaTime / n_substeps;
            for (unsigned int step=0; step!= n_substeps; ++step){
                // Move the ball i.e. update position and speed
                move(spheres[i], dt, cubePositions[0]);

                // Check for collisions
                for (std::vector<Sphere>::size_type j=0; j!=spheres.size(); ++j){
                    if (i != j)
                        collision(spheres[i], spheres[j]);
                }
            }
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
