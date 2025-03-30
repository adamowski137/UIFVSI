#include "Shader.hpp"
#include <cstdint>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

Shader::Shader(std::string vertexPath, std::string fragmentPath) {
  const char *vertexCode, *fragmentCode;
  std::ifstream vertexFile, fragmentFile;

  vertexFile.open(vertexPath);
  fragmentFile.open(fragmentPath);

  std::stringstream vertexStream, fragmentStream;

  vertexStream.exceptions(std::ifstream::failbit | std::ifstream::badbit);
  fragmentStream.exceptions(std::ifstream::failbit | std::ifstream::badbit);

  vertexStream << vertexFile.rdbuf();
  fragmentStream << fragmentFile.rdbuf();

  vertexFile.close();
  fragmentFile.close();

  std::string vertexString = vertexStream.str();
  std::string fragmentString = fragmentStream.str();

  vertexCode = vertexString.c_str();
  fragmentCode = fragmentString.c_str();

  uint32_t vId, fId;
  vId = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vId, 1, &vertexCode, NULL);
  glCompileShader(vId);
  checkCompileErrors(vId, "Vertex");

  fId = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fId, 1, &fragmentCode, NULL);
  glCompileShader(fId);
  checkCompileErrors(fId, "Fragment");

  m_id = glCreateProgram();
  glAttachShader(m_id, vId);
  glAttachShader(m_id, fId);
  glLinkProgram(m_id);
  checkCompileErrors(m_id, "Program");

  glDeleteShader(vId);
  glDeleteShader(fId);
}

void Shader::checkCompileErrors(GLuint shader, std::string type) {
  GLint success;
  GLchar infoLog[1024];
  if (type != "Program") {
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
      glGetShaderInfoLog(shader, 1024, NULL, infoLog);
      throw std::runtime_error(type + infoLog);
    } else {
      glGetProgramiv(shader, GL_LINK_STATUS, &success);
      if (!success) {
        glGetProgramInfoLog(shader, 1024, NULL, infoLog);
        throw std::runtime_error(type + infoLog);
      }
    }
  }
}
