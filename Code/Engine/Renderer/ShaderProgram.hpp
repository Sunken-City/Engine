#pragma once
#include <string>
#include <vector>
#include <map>

class Vector2;
class Vector3;
class Vector4;
class Matrix4x4;

//--------------------------------------------------------------------------------------------------------------------------------------------------------
struct Uniform
{
    //ENUMS//////////////////////////////////////////////////////////////////////////
    enum class DataType
    {
        SAMPLER_2D,
        MATRIX_4X4,
        VECTOR2,
        VECTOR3,
        VECTOR4,
        FLOAT,
        INT,
        NUM_TYPES
    };

    std::string name;
    DataType type;
    int bindPoint = -1;
    unsigned int size;
    unsigned int textureIndex;
};

//----------------------------------------------------------------------------------------------------------------------------------------------------------
class ShaderProgram
{
public:
    //Typedef'd so that we don't need to include Windows.h
    typedef unsigned int GLuint;
    typedef int GLint;
    typedef int GLsizei;
    typedef unsigned int GLenum;
    typedef bool GLboolean;

    //CONSTRUCTORS//////////////////////////////////////////////////////////////////////////
    ShaderProgram();
    ShaderProgram(const char* vertShaderPath, const char* fragShaderPath);
    ~ShaderProgram();

    //FUNCTIONS//////////////////////////////////////////////////////////////////////////
    static ShaderProgram* CreateFromShaderStrings(const char* vertShaderString, const char* fragShaderString);
    GLuint LoadShader(const char* filename, GLuint shaderType);
    GLuint LoadShaderFromString(const char* shaderCode, GLuint shaderType, const char* filename = nullptr);
    GLuint CreateAndLinkProgram(GLuint vs, GLuint fs);
    void ShaderProgramBindProperty(size_t hashedName, GLint count, GLenum type, GLboolean normalize, GLsizei stride, GLsizei offset);
    void ShaderProgramBindIntegerProperty(size_t hashedName, GLint count, GLenum type, GLsizei stride, GLsizei offset);
    void FindAllAttributes();
    void FindAllUniforms();
    void BindUniformBuffer(const char* uniformBlockName, GLint bindPoint);
    GLint GetBindPoint(size_t hashedName);

    //SETTING UNIFORMS/////////////////////////////////////////////////////////////////////
    bool SetUniform(size_t hashedName, void* value);
    bool SetVec2Uniform(GLint bindPoint, const Vector2& value);
    bool SetVec3Uniform(GLint bindPoint, const Vector3& value);
    bool SetVec4Uniform(GLint bindPoint, const Vector4& value);
    bool SetMatrix4x4Uniform(GLint bindPoint, const Matrix4x4& value);
    bool SetFloatUniform(GLint bindPoint, float value);
    bool SetIntUniform(GLint bindPoint, int value);


    //DEPRECATED FUNCTIONS, AVOID USING:
    //These functions look up the bind point every time, and will be removed in the future.
    bool SetVec2Uniform(const char* name, const Vector2& value);
    bool SetVec3Uniform(const char* name, const Vector3& value);
    bool SetVec3Uniform(const char *name, const Vector3& value, unsigned int arrayIndex);
    bool SetVec4Uniform(const char* name, const Vector4& value);
    bool SetVec4Uniform(const char *name, const Vector4& value, unsigned int arrayIndex);
    bool SetMatrix4x4Uniform(const char* name, const Matrix4x4& value);
    bool SetMatrix4x4Uniform(const char* name, Matrix4x4& value, unsigned int arrayIndex);
    bool SetIntUniform(const char* name, int value);
    bool SetIntUniform(const char* name, int value, unsigned int arrayIndex);
    bool SetFloatUniform(const char* name, float value);
    bool SetFloatUniform(const char* name, float value, unsigned int arrayIndex);


    //MEMBER VARIABLES//////////////////////////////////////////////////////////////////////////
    GLuint m_vertexShaderID;
    GLuint m_fragmentShaderID;
    GLuint m_shaderProgramID;
    std::map<size_t, Uniform> m_uniforms;
    std::map<size_t, GLint> m_attributes;

private:
    ShaderProgram(const ShaderProgram&);
};