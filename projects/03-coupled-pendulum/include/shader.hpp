#ifndef SHADER_H
#define SHADER_H
#include <glad/glad.h>

#include <string>
#include <glm/glm.hpp>


class Shader
{
  public:
    // constructor reads and builds the shader
    Shader(const char* vertexPath, const char* fragmentPath);
    ~Shader();

    // Use this shade program
    void use();

    // set uniform values
    void setBool(const std::string &name, bool value) const;
    void setInt(const std::string &name, int value) const;
    void setFloat(const std::string &name, float value) const;
    void set2f(const std::string &name, const glm::vec2& vec) const;
    void set3f(const std::string &name, const glm::vec3& vec) const;
    void set4f(const std::string &name, const glm::vec4& vec) const;
    void setMat4f(const std::string &name, const glm::mat4 &mat) const;

  private:
    // program ID
    unsigned int ID;

    void checkCompileErrors(unsigned int shader, std::string type);
};
#endif
