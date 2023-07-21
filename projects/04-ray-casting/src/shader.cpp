#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <glm/glm.hpp>

#include "shader.hpp"


Shader::Shader(const char* vertexPath, const char* fragmentPath)
{
    // get source code from file
    std::string vertexCode;
    std::string fragmentCode;
    std::ifstream vShaderFile;
    std::ifstream fShaderFile;

    // ensure ifstream can throw exceptions
    vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

    try
    {
      // open files
      vShaderFile.open(vertexPath);
      fShaderFile.open(fragmentPath);

      // read file's buffer contents into streams
      std::stringstream vShaderStream, fShaderStream;
      vShaderStream << vShaderFile.rdbuf();
      fShaderStream << fShaderFile.rdbuf();

      // close file handlers
      vShaderFile.close();
      fShaderFile.close();

      // convert stream into string
      vertexCode = vShaderStream.str();
      fragmentCode = fShaderStream.str();
    }
    catch(std::ifstream::failure e)
    {
      std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
    }
    const char* vShaderCode = vertexCode.c_str();
    const char* fShaderCode = fragmentCode.c_str();

    // compile
    unsigned int vertex, fragment;
    int success;
    char infoLog[512];

    // compile vertex
    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vShaderCode, NULL);
    glCompileShader(vertex);
    checkCompileErrors(vertex, "VERTEX");

    // compile fragment
    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fShaderCode, NULL);
    glCompileShader(fragment);
    checkCompileErrors(fragment, "FRAGMENT");

    // shader program
    ID = glCreateProgram();
    glAttachShader(ID, vertex);
    glAttachShader(ID, fragment);
    glLinkProgram(ID);
    checkCompileErrors(ID, "PROGRAM");

    glDeleteShader(vertex);
    glDeleteShader(fragment);
}

Shader::~Shader(){
    glDeleteProgram(ID);
}

void Shader::use()
{
    glUseProgram(ID);
}

void Shader::checkCompileErrors(unsigned int shader, std::string type){
    int success;
    char infoLog[1024];

    if (type != "PROGRAM") {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

        if (!success) {
          glGetShaderInfoLog(shader, 1024, NULL, infoLog);
          std::cout << "ERROR::SHADER::" << type << "::COMPILATION_FAILED\n" << infoLog << std::endl;
        }
    } else {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);

        if (!success) {
            glGetProgramInfoLog(shader, 1024, NULL, infoLog);
            std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
        }
    }
}

// Set Uniforms
void Shader::setBool(const std::string &name, bool value) const
{
    glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
}

void Shader::setInt(const std::string &name, int value) const
{
    glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
}

void Shader::setFloat(const std::string &name, float value) const
{
    glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
}

void Shader::set2f(const std::string &name, const glm::vec2& vec) const
{
    glUniform2fv(glGetUniformLocation(ID, name.c_str()), 1, &vec[0]);
}

void Shader::set3f(const std::string &name, const glm::vec3& vec) const
{
    glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, &vec[0]);
}

void Shader::set4f(const std::string &name, const glm::vec4& vec) const
{
    glUniform4fv(glGetUniformLocation(ID, name.c_str()), 1, &vec[0]);
}

void Shader::setMat4f(const std::string &name, const glm::mat4 &mat) const
{
    glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}
