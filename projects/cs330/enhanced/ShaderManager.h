#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <string>

class ShaderManager
{
public:
    GLuint ID;

    ShaderManager() : ID(0) {}
    ~ShaderManager() { if (ID != 0) glDeleteProgram(ID); }

    void use() { glUseProgram(ID); }

    void LoadShaders(const char* vertexPath, const char* fragmentPath);

    void setBoolValue(const std::string& name, bool value) const {
        glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
    }

    // ADDED: For setting texture toggles and flags
    void setIntValue(const std::string& name, int value) const {
        glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
    }

    void setFloatValue(const std::string& name, float value) const {
        glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
    }

    // ADDED: For setting texture UV scale coordinates
    void setVec2Value(const std::string& name, const glm::vec2& value) const {
        glUniform2fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
    }

    void setVec3Value(const std::string& name, float x, float y, float z) const {
        glUniform3f(glGetUniformLocation(ID, name.c_str()), x, y, z);
    }
    void setVec3Value(const std::string& name, const glm::vec3& value) const {
        glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
    }

    // ADDED: For setting solid or fallback colors
    void setVec4Value(const std::string& name, const glm::vec4& value) const {
        glUniform4fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
    }

    // ADDED: For passing texture slots to fragment shader samplers
    void setSampler2DValue(const std::string& name, int value) const {
        glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
    }

    void setMat4Value(const std::string& name, const glm::mat4& mat) const {
        glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    }
};