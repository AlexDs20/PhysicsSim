#ifndef UTILS_H
#define UTILS_H
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include "camera.hpp"

extern Camera camera;
extern float lastX, lastY;
extern bool firstMouse;

extern float deltaTime;
extern float lastFrame;

// Callback to resize Viewport
void framebuffer_size_callback(GLFWwindow* window, int width, int height);

// Callback for the mouse input
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

// Process Input and how to handle it
void processInput(GLFWwindow* window);

int logShaderCompile(const unsigned int shaderId, const char* shaderType);

int logProgramLink(const unsigned int shaderProgram);
#endif
