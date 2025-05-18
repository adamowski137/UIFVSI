#include "Shader.hpp"
#include <cstdint>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

Shader::Shader(std::string vertexPath, std::string fragmentPath) {

  std::string vertexCode = getShaderCode(vertexPath);
  std::string fragmentCode = getShaderCode(fragmentPath);

  const char *vs = vertexCode.c_str();
  const char *fs = fragmentCode.c_str();

  uint32_t vId, fId;
  vId = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vId, 1, &vs, NULL);
  glCompileShader(vId);
  checkCompileErrors(vId, "Vertex");

  fId = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fId, 1, &fs, NULL);
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
Shader::Shader(std::string vertexShaderPath, std::string fragmentShaderPath,
               std::string tessalationControlPath,
               std::string tessalationEvaluationPath) {
  std::string vertexCode = getShaderCode(vertexShaderPath).c_str();
  std::string fragmentCode = getShaderCode(fragmentShaderPath).c_str();
  std::string tessalationControlCode =
      getShaderCode(tessalationControlPath).c_str();
  std::string tessalationEvaluationCode =
      getShaderCode(tessalationEvaluationPath).c_str();

  const char *vs = vertexCode.c_str();
  const char *fs = fragmentCode.c_str();
  const char *tcs = tessalationControlCode.c_str();
  const char *tes = tessalationEvaluationCode.c_str();

  uint32_t vId, fId, teId, tcId;
  vId = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vId, 1, &vs, NULL);
  glCompileShader(vId);
  checkCompileErrors(vId, "Vertex");

  tcId = glCreateShader(GL_TESS_CONTROL_SHADER);
  glShaderSource(tcId, 1, &tcs, NULL);
  glCompileShader(tcId);
  checkCompileErrors(tcId, "Tessalation Control");

  teId = glCreateShader(GL_TESS_EVALUATION_SHADER);
  glShaderSource(teId, 1, &tes, NULL);
  glCompileShader(teId);
  checkCompileErrors(teId, "Tessalation Evaluation");

  fId = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fId, 1, &fs, NULL);
  glCompileShader(fId);
  checkCompileErrors(fId, "Fragment");

  m_id = glCreateProgram();
  glAttachShader(m_id, vId);
  glAttachShader(m_id, tcId);
  glAttachShader(m_id, teId);
  glAttachShader(m_id, fId);
  glLinkProgram(m_id);
  checkCompileErrors(m_id, "Program");

  glDeleteShader(vId);
  glDeleteShader(tcId);
  glDeleteShader(teId);
  glDeleteShader(fId);
}

std::string Shader::getShaderCode(std::string path) {
  std::ifstream shaderFile;

  shaderFile.open(path);

  std::stringstream stream;

  stream.exceptions(std::ifstream::failbit | std::ifstream::badbit);

  stream << shaderFile.rdbuf();

  shaderFile.close();

  return stream.str();
}

void Shader::checkCompileErrors(GLuint shader, std::string type) {
  GLint success;
  GLchar infoLog[1024];
  if (type != "Program") {
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
      glGetShaderInfoLog(shader, 1024, NULL, infoLog);
      throw std::runtime_error(type + infoLog);
    }
  } else {
    glGetProgramiv(shader, GL_LINK_STATUS, &success);
    if (!success) {
      glGetProgramInfoLog(shader, 1024, NULL, infoLog);
      throw std::runtime_error(type + infoLog);
    }
  }
}
