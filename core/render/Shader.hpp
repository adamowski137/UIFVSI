#pragma once

#include "Matrix.hpp"
#include "Vector.hpp"
#include <GL/glew.h>
#include <cstdint>
#include <string>

enum class ShaderType { OBJECT, POINT, CURVE, SURFACE };

class Shader {
public:
  Shader(std::string vertexShaderPath, std::string fragmentShaderPath);
  Shader(std::string vertexShaderPath, std::string fragmentShaderPath,
         std::string tessalationControlPath,
         std::string tessalationEvaluationPath);
  inline void use() const { glUseProgram(m_id); };

  inline void setBool(const std::string &name, bool value) const {
    glUniform1i(glGetUniformLocation(m_id, name.c_str()), (int)value);
  }
  inline void setUInt(const std::string &name, uint32_t value) const {
    glUniform1ui(glGetUniformLocation(m_id, name.c_str()), value);
  }
  inline void setInt(const std::string &name, int value) const {
    glUniform1i(glGetUniformLocation(m_id, name.c_str()), value);
  }
  inline void setFloat(const std::string &name, float value) const {
    glUniform1f(glGetUniformLocation(m_id, name.c_str()), value);
  }
  inline void setVec2(const std::string &name,
                      const math137::Vector2f &value) const {
    glUniform2fv(glGetUniformLocation(m_id, name.c_str()), 1, value.data());
  }
  inline void setVec2(const std::string &name, float x, float y) const {
    glUniform2f(glGetUniformLocation(m_id, name.c_str()), x, y);
  }
  inline void setVec3(const std::string &name,
                      const math137::Vector3f &value) const {
    glUniform3fv(glGetUniformLocation(m_id, name.c_str()), 1, value.data());
  }
  inline void setVec3(const std::string &name, float x, float y,
                      float z) const {
    glUniform3f(glGetUniformLocation(m_id, name.c_str()), x, y, z);
  }
  inline void setVec4(const std::string &name,
                      const math137::Vector4f &value) const {
    glUniform4fv(glGetUniformLocation(m_id, name.c_str()), 1, value.data());
  }
  inline void setVec4(const std::string &name, float x, float y, float z,
                      float w) const {
    glUniform4f(glGetUniformLocation(m_id, name.c_str()), x, y, z, w);
  }
  inline void setMat2(const std::string &name,
                      const math137::Matrix<float, 2, 2> &mat) const {
    glUniformMatrix2fv(glGetUniformLocation(m_id, name.c_str()), 1, GL_TRUE,
                       mat.data());
  }
  inline void setMat3(const std::string &name,
                      const math137::Matrix<float, 3, 3> &mat) const {
    glUniformMatrix3fv(glGetUniformLocation(m_id, name.c_str()), 1, GL_TRUE,
                       mat.data());
  }
  inline void setMat4(const std::string &name,
                      const math137::Matrix4f &mat) const {
    glUniformMatrix4fv(glGetUniformLocation(m_id, name.c_str()), 1, GL_TRUE,
                       mat.data());
  }

private:
  void checkCompileErrors(uint32_t shader, std::string type);
  std::string getShaderCode(std::string filePath);

  uint32_t m_id;
};
