#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

int main(int argc, char** argv)
{
  int a = glfwInit();
  std::cout << a << std::endl;
  return 0;
}
