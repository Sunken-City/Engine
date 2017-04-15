#include "Engine/Renderer/ShaderProgram.hpp"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <gl/GL.h>
#include <gl/GLU.h>
#include <stdlib.h>
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Input/InputOutputUtils.hpp"
#include "Engine/Renderer/OpenGLExtensions.hpp"
#include "Engine/Core/BuildConfig.hpp"
#include "Engine/Core/Memory/MemoryTracking.hpp"
#include "..\Math\Vector2.hpp"
#include <xstddef>
#include "Renderer.hpp"

//-----------------------------------------------------------------------------------
ShaderProgram::ShaderProgram()
    : m_vertexShaderID(0)
    , m_fragmentShaderID(0)
    , m_shaderProgramID(0)
{

}

//-----------------------------------------------------------------------------------
ShaderProgram::ShaderProgram(const char* vertShaderPath, const char* fragShaderPath)
    : m_vertexShaderID(LoadShader(vertShaderPath, GL_VERTEX_SHADER))
    , m_fragmentShaderID(LoadShader(fragShaderPath, GL_FRAGMENT_SHADER))
    , m_shaderProgramID(CreateAndLinkProgram(m_vertexShaderID, m_fragmentShaderID))
{
    FindAllAttributes();
    FindAllUniforms();
    ASSERT_OR_DIE(m_vertexShaderID != NULL && m_fragmentShaderID != NULL, "Error: Vertex or Fragment Shader was null");
    ASSERT_OR_DIE(m_shaderProgramID != NULL, "Error: Program linking id was null");
    #if defined(TRACK_MEMORY)
        g_memoryAnalytics.TrackShaderAllocation();
    #endif
}

//-----------------------------------------------------------------------------------
ShaderProgram::~ShaderProgram()
{
    glDeleteShader(m_vertexShaderID);
    glDeleteShader(m_fragmentShaderID);
    glDeleteProgram(m_shaderProgramID);
    #if defined(TRACK_MEMORY)
        g_memoryAnalytics.TrackShaderFree();
    #endif
}

//-----------------------------------------------------------------------------------
ShaderProgram* ShaderProgram::CreateFromShaderStrings(const char* vertShaderString, const char* fragShaderString)
{
    ShaderProgram* shader = new ShaderProgram();
    shader->m_vertexShaderID = shader->LoadShaderFromString(vertShaderString, GL_VERTEX_SHADER);
    shader->m_fragmentShaderID = shader->LoadShaderFromString(fragShaderString, GL_FRAGMENT_SHADER);
    shader->m_shaderProgramID = shader->CreateAndLinkProgram(shader->m_vertexShaderID, shader->m_fragmentShaderID);
    ASSERT_OR_DIE(shader->m_vertexShaderID != NULL && shader->m_fragmentShaderID != NULL, "Error: Vertex or Fragment Shader was null");
    ASSERT_OR_DIE(shader->m_shaderProgramID != NULL, "Error: Program linking id was null");
    #if defined(TRACK_MEMORY)
    {
        g_memoryAnalytics.TrackShaderAllocation();
    }
    #endif
    shader->FindAllAttributes();
    shader->FindAllUniforms();
    return shader;
}

//-----------------------------------------------------------------------------------
GLuint ShaderProgram::LoadShader(const char* filename, GLenum shaderType)
{
    char* fileBuffer = FileReadIntoNewBuffer(filename);

    GLuint shaderID = LoadShaderFromString(fileBuffer, shaderType, filename);

    delete fileBuffer;
    fileBuffer = nullptr;
    return shaderID;
}

//-----------------------------------------------------------------------------------
GLuint ShaderProgram::LoadShaderFromString(const char* shaderCode, GLuint shaderType, const char* filename)
{
    GLuint shader_id = glCreateShader(shaderType);
    ASSERT_OR_DIE(shader_id != NULL, "Failed to create shader");

    GLint src_length = strlen(shaderCode);
    glShaderSource(shader_id, 1, &shaderCode, &src_length);

    glCompileShader(shader_id);

    GLint status;
    glGetShaderiv(shader_id, GL_COMPILE_STATUS, &status);
    if (GL_FALSE == status)
    {
        GLint logLength;
        glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &logLength);

        char *buffer = new char[logLength + 1];
        glGetShaderInfoLog(shader_id, logLength, &logLength, buffer);

        buffer[logLength] = '\0'; //or NULL
        glDeleteShader(shader_id);
        std::string bufferString(buffer);

        //Get the full path
        char filePath[_MAX_PATH];
        if (filename)
        {
            _fullpath(filePath, filename, _MAX_PATH);
        }
        else
        {
            filename = "[[From Source]]";
            strcpy_s(filePath, filename);
        }

        std::size_t firstSemicolon = bufferString.find(":");
        std::size_t secondSemicolon = bufferString.find(":", firstSemicolon + 1, 1);
        std::string formattedErrorString = bufferString.substr(secondSemicolon + 1);

        std::vector<std::string>* inParenthesis = ExtractStringsBetween(bufferString, "(", ")");
        int lineNumber = std::stoi(inParenthesis->at(0));
        delete inParenthesis;
        DebuggerPrintf("%s(%d): %s", filePath, lineNumber, formattedErrorString.c_str());

        const char* glVersion = (const char*)glGetString(GL_VERSION);
        const char* glslVersion = (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION);
        ERROR_AND_DIE(Stringf("%s\nIn file: %s\nOn line: %i \n\n %s \nOpenGL version: %s\nGLSL version: %s", formattedErrorString.c_str(), filename, lineNumber, buffer, glVersion, glslVersion));
    }

    //Todo: print errors if failed

    return shader_id;
}

//-----------------------------------------------------------------------------------
GLuint ShaderProgram::CreateAndLinkProgram(GLuint vertexShader, GLuint fragmentShader)
{
    GLuint program_id = glCreateProgram();
    ASSERT_OR_DIE(program_id != NULL, "Failed to create shader program");

    glAttachShader(program_id, vertexShader);
    glAttachShader(program_id, fragmentShader);

    glLinkProgram(program_id);

    GLint status;
    //Get iv gets a 'v'ariable amount of 'i'ntegers
    glGetProgramiv(program_id, GL_LINK_STATUS, &status);
    if (status == GL_FALSE)
    {
        GLint logLength;
        glGetProgramiv(program_id, GL_INFO_LOG_LENGTH, &logLength);

        char *buffer = new char[logLength + 1];
        glGetProgramInfoLog(program_id, logLength, &logLength, buffer);

        buffer[logLength] = '\0'; //or NULL
        glDeleteShader(program_id);
        std::string bufferString(buffer);

        std::size_t firstSemicolon = bufferString.find(":");
        std::size_t secondSemicolon = bufferString.find(":", firstSemicolon + 1, 1);
        std::string formattedErrorString = bufferString.substr(secondSemicolon + 1);

        std::vector<std::string>* inParenthesis = ExtractStringsBetween(bufferString, "(", ")");
        int lineNumber = std::stoi(inParenthesis->at(0));
        delete inParenthesis;
        DebuggerPrintf("(%d): %s", lineNumber, formattedErrorString.c_str());

        const char* glVersion = (const char*)glGetString(GL_VERSION);
        const char* glslVersion = (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION);
        ERROR_AND_DIE(Stringf("%s\nOn line: %i \n\n %s \nOpenGL version: %s\nGLSL version: %s", formattedErrorString.c_str(), lineNumber, buffer, glVersion, glslVersion));
    }
    else
    {
        //Success! Let OpenGL clean up video memory for the shaders
        glDetachShader(program_id, vertexShader);
        glDetachShader(program_id, fragmentShader);
    }

    return program_id;
}

//-----------------------------------------------------------------------------------
void ShaderProgram::ShaderProgramBindProperty(size_t hashedName, GLint count, GLenum type, GLboolean normalize, GLsizei stride, GLsizei offset)
{
    GLint property = -1;
    auto iter = m_attributes.find(hashedName);
    if (iter != m_attributes.end())
    {
        property = iter->second;
    }
    //GLint property = glGetAttribLocation(m_shaderProgramID, name);
    //property = glGetAttribLocation(m_shaderProgramID, "inColor");

    if (property >= 0)
    {
        glEnableVertexAttribArray(property);
        glVertexAttribPointer(property, //Bind point to shader
            count, //Number of data elements passed
            type, //Type of Data
            normalize, //normalize the data for us
            stride, //stride
            (GLvoid*)offset //From that point in memory, how far do we have to go to get the value?
            );
    }
}

//-----------------------------------------------------------------------------------
void ShaderProgram::ShaderProgramBindIntegerProperty(size_t hashedName,  GLint count, GLenum type, GLsizei stride, GLsizei offset)
{
    GLint property = -1;
    auto iter = m_attributes.find(hashedName);
    if (iter != m_attributes.end())
    {
        property = iter->second;
    }
    //GLint property = glGetAttribLocation(m_shaderProgramID, name);
    if (property >= 0)
    {
        glEnableVertexAttribArray(property);
        glVertexAttribIPointer(property, //Bind point to shader
            count, //Number of data elements passed
            type, //Type of Data
            stride, //stride
            (GLvoid*)offset //From that point in memory, how far do we have to go to get the value?
            );
    }
}

//-----------------------------------------------------------------------------------
void ShaderProgram::FindAllAttributes()
{
    GLint numberOfActiveAttributes;
    glGetProgramiv(m_shaderProgramID, GL_ACTIVE_ATTRIBUTES, &numberOfActiveAttributes);
    GLint maxNameLength;
    glGetProgramiv(m_shaderProgramID, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &maxNameLength);
    for (int index = 0; index < numberOfActiveAttributes; index++)
    {
        GLint size;
        GLenum type;
        char* nameBuffer = new char[maxNameLength];
        glGetActiveAttrib(m_shaderProgramID, index, maxNameLength, NULL, &size, &type, nameBuffer);
        m_attributes[std::hash<std::string>{}(nameBuffer)] = glGetAttribLocation(m_shaderProgramID, nameBuffer);
        delete nameBuffer;
    }
}

//-----------------------------------------------------------------------------------
void ShaderProgram::FindAllUniforms()
{
    GLint numberOfActiveUniforms;
    glGetProgramiv(m_shaderProgramID, GL_ACTIVE_UNIFORMS, &numberOfActiveUniforms);
    GLint maxNameLength;
    glGetProgramiv(m_shaderProgramID, GL_ACTIVE_UNIFORM_MAX_LENGTH, &maxNameLength);
    for (int index = 0; index < numberOfActiveUniforms; index++)
    {
        Uniform uniform;
        GLint size;
        GLenum type;
        char* nameBuffer = new char[maxNameLength];
        glGetActiveUniform(m_shaderProgramID, index, maxNameLength, NULL, &size, &type, nameBuffer);
        uniform.name = std::string(nameBuffer);
        switch (type)
        {
        case GL_SAMPLER_2D:
            uniform.type = Uniform::DataType::SAMPLER_2D;
            break;
        case GL_FLOAT_MAT4:
            uniform.type = Uniform::DataType::MATRIX_4X4;
            break;
        case GL_FLOAT_VEC4:
            uniform.type = Uniform::DataType::VECTOR4;
            break;
        case GL_FLOAT_VEC3:
            uniform.type = Uniform::DataType::VECTOR3;
            break;
        case GL_FLOAT_VEC2:
            uniform.type = Uniform::DataType::VECTOR2;
            break;
        case GL_FLOAT:
            uniform.type = Uniform::DataType::FLOAT;
            break;
        case GL_INT:
            uniform.type = Uniform::DataType::INT;
            break;
        default:
            ERROR_RECOVERABLE(Stringf("0x%x was given as a uniform type, but it wasn't found (add support for it as a uniform data type!)", type));
            break;
        }
        uniform.size = size;
        uniform.bindPoint = index;
        uniform.textureIndex = 0;
        size_t hashIndex = std::hash<std::string>{}(uniform.name);
        m_uniforms[hashIndex] = uniform;
        delete nameBuffer;
    }
}

//-----------------------------------------------------------------------------------
void ShaderProgram::BindUniformBuffer(const char* uniformBlockName, GLint bindPoint)
{
    GLuint blockIndex = glGetUniformBlockIndex(m_shaderProgramID, uniformBlockName);
    glUniformBlockBinding(m_shaderProgramID, blockIndex, bindPoint);
}

//-----------------------------------------------------------------------------------
GLint ShaderProgram::GetBindPoint(size_t hashedName)
{
    GLint bindPoint = -1;
    auto iter = m_uniforms.find(hashedName);
    if (iter != m_uniforms.end())
    {
        bindPoint = iter->second.bindPoint;
    }
    return bindPoint;
}

//-----------------------------------------------------------------------------------
bool ShaderProgram::SetUniform(size_t hashedName, void* value)
{
    Uniform* matchingUniform = nullptr;
    auto iter = m_uniforms.find(hashedName);
    if (iter != m_uniforms.end())
    {
        matchingUniform = &(iter->second);
    }
    else
    {
        return false;
    }

    switch (matchingUniform->type)
    {
    case Uniform::DataType::MATRIX_4X4:
        return SetMatrix4x4Uniform(matchingUniform->bindPoint, *static_cast<Matrix4x4*>(value));
    case Uniform::DataType::VECTOR4:
        return SetVec4Uniform(matchingUniform->bindPoint, *static_cast<Vector4*>(value));
    case Uniform::DataType::VECTOR3:
        return SetVec3Uniform(matchingUniform->bindPoint, *static_cast<Vector3*>(value));
    case Uniform::DataType::VECTOR2:
        return SetVec2Uniform(matchingUniform->bindPoint, *static_cast<Vector2*>(value));
    case Uniform::DataType::FLOAT:
        return SetFloatUniform(matchingUniform->bindPoint, *static_cast<float*>(value));
    case Uniform::DataType::INT:
        return SetIntUniform(matchingUniform->bindPoint, *static_cast<int*>(value));
    default:
        break;
    }
    return false;
}

//-----------------------------------------------------------------------------------
bool ShaderProgram::SetVec2Uniform(const char* name, const Vector2 &value)
{
    Renderer::instance->UseShaderProgram(m_shaderProgramID);
    GLint loc = glGetUniformLocation(m_shaderProgramID, name);
    if (loc >= 0)
    {
        glUniform2fv(loc, 1, (GLfloat*)&value);
        return true;
    }
    return false;
}

//-----------------------------------------------------------------------------------
bool ShaderProgram::SetVec2Uniform(GLint bindPoint, const Vector2& value)
{
    Renderer::instance->UseShaderProgram(m_shaderProgramID);
    if (bindPoint >= 0)
    {
        glUniform2fv(bindPoint, 1, (GLfloat*)&value);
        return true;
    }
    return false;
}

//-----------------------------------------------------------------------------------
bool ShaderProgram::SetVec3Uniform(const char *name, const Vector3& value, unsigned int numElements)
{
    Renderer::instance->UseShaderProgram(m_shaderProgramID);
    //"THIS IS A PROBLEM. This looks up the uniform location EVERY TIME, stalling out the pipeline."
    GLint loc = glGetUniformLocation(m_shaderProgramID, name);
    if (loc >= 0)
    {
        glUniform3fv(loc, numElements, (GLfloat*)&value);
        return true;
    }
    return false;
}

//-----------------------------------------------------------------------------------
bool ShaderProgram::SetVec3Uniform(const char *name, const Vector3 &value)
{
    Renderer::instance->UseShaderProgram(m_shaderProgramID);
    //"THIS IS A PROBLEM. This looks up the uniform location EVERY TIME, stalling out the pipeline."
    GLint loc = glGetUniformLocation(m_shaderProgramID, name);
    if (loc >= 0)
    {
        glUniform3fv(loc, 1, (GLfloat*)&value);
        return true;
    }
    return false;
}

//-----------------------------------------------------------------------------------
bool ShaderProgram::SetVec3Uniform(GLint bindPoint, const Vector3& value)
{
    Renderer::instance->UseShaderProgram(m_shaderProgramID);
    if (bindPoint >= 0)
    {
        glUniform3fv(bindPoint, 1, (GLfloat*)&value);
        return true;
    }
    return false;
}

//-----------------------------------------------------------------------------------
bool ShaderProgram::SetVec4Uniform(const char *name, const Vector4 &value, unsigned int numElements)
{
    Renderer::instance->UseShaderProgram(m_shaderProgramID);
    GLint loc = glGetUniformLocation(m_shaderProgramID, name);
    if (loc >= 0)
    {
        glUniform4fv(loc, numElements, (GLfloat*)&value);
        return true;
    }
    return false;
}

//-----------------------------------------------------------------------------------
bool ShaderProgram::SetVec4Uniform(const char *name, const Vector4 &value)
{
    Renderer::instance->UseShaderProgram(m_shaderProgramID);
    GLint loc = glGetUniformLocation(m_shaderProgramID, name);
    if (loc >= 0)
    {
        glUniform4fv(loc, 1, (GLfloat*)&value);
        return true;
    }
    return false;
}

//-----------------------------------------------------------------------------------
bool ShaderProgram::SetVec4Uniform(GLint bindPoint, const Vector4& value)
{
    Renderer::instance->UseShaderProgram(m_shaderProgramID);
    if (bindPoint >= 0)
    {
        glUniform4fv(bindPoint, 1, (GLfloat*)&value);
        return true;
    }
    return false;
}

//-----------------------------------------------------------------------------------
bool ShaderProgram::SetMatrix4x4Uniform(const char* name, const Matrix4x4& value)
{
    Renderer::instance->UseShaderProgram(m_shaderProgramID);
    GLint loc = glGetUniformLocation(m_shaderProgramID, name);
    if (loc >= 0)
    {
        glUniformMatrix4fv(loc, 1, GL_FALSE, (GLfloat*)&value); //location, number of elements, do you want gl to transpose matrix?, matrix
        return true;
    }
    return false;
}

//-----------------------------------------------------------------------------------
bool ShaderProgram::SetMatrix4x4Uniform(const char* name, Matrix4x4& value, unsigned int numberOfElements)
{
    Renderer::instance->UseShaderProgram(m_shaderProgramID);
    GLint loc = glGetUniformLocation(m_shaderProgramID, name);
    if (loc >= 0)
    {
        glUniformMatrix4fv(loc, numberOfElements, GL_FALSE, (GLfloat*)&value); //location, number of elements, do you want gl to transpose matrix?, matrix
        return true;
    }
    return false;
}

//-----------------------------------------------------------------------------------
bool ShaderProgram::SetMatrix4x4Uniform(GLint bindPoint, const Matrix4x4& value)
{
    Renderer::instance->UseShaderProgram(m_shaderProgramID);
    if (bindPoint >= 0)
    {
        glUniformMatrix4fv(bindPoint, 1, GL_FALSE, (GLfloat*)&value); //location, number of elements, do you want gl to transpose matrix?, matrix
        return true;
    }
    return false;
}

//-----------------------------------------------------------------------------------
bool ShaderProgram::SetIntUniform(const char* name, int value, unsigned int arrayIndex)
{
    Renderer::instance->UseShaderProgram(m_shaderProgramID);
    GLint loc = glGetUniformLocation(m_shaderProgramID, name);
    if (loc >= 0)
    {
        glUniform1iv(loc, arrayIndex, (GLint*)&value); //location, number of elements, do you want gl to transpose matrix?, matrix
        return true;
    }
    return false;
}

//-----------------------------------------------------------------------------------
bool ShaderProgram::SetIntUniform(const char* name, int value)
{
    Renderer::instance->UseShaderProgram(m_shaderProgramID);
    GLint loc = glGetUniformLocation(m_shaderProgramID, name);
    if (loc >= 0)
    {
        glUniform1iv(loc, 1, (GLint*)&value); //location, number of elements, do you want gl to transpose matrix?, matrix
        return true;
    }
    return false;
}

//-----------------------------------------------------------------------------------
bool ShaderProgram::SetIntUniform(GLint bindPoint, int value)
{
    Renderer::instance->UseShaderProgram(m_shaderProgramID);
    if (bindPoint >= 0)
    {
        glUniform1iv(bindPoint, 1, (GLint*)&value); //location, number of elements, do you want gl to transpose matrix?, matrix
        return true;
    }
    return false;
}

//-----------------------------------------------------------------------------------
bool ShaderProgram::SetFloatUniform(const char* name, float value, unsigned int arrayIndex)
{
    Renderer::instance->UseShaderProgram(m_shaderProgramID);
    GLint loc = glGetUniformLocation(m_shaderProgramID, name);
    if (loc >= 0)
    {
        glUniform1fv(loc, arrayIndex, (GLfloat*)&value); //location, number of elements, do you want gl to transpose matrix?, matrix
        return true;
    }
    return false;
}

//-----------------------------------------------------------------------------------
bool ShaderProgram::SetFloatUniform(const char* name, float value)
{
    Renderer::instance->UseShaderProgram(m_shaderProgramID);
    GLint loc = glGetUniformLocation(m_shaderProgramID, name);
    if (loc >= 0)
    {
        glUniform1fv(loc, 1, (GLfloat*)&value); //location, number of elements, do you want gl to transpose matrix?, matrix
        return true;
    }
    return false;
}

//-----------------------------------------------------------------------------------
bool ShaderProgram::SetFloatUniform(GLint bindPoint, float value)
{
    Renderer::instance->UseShaderProgram(m_shaderProgramID);
    if (bindPoint >= 0)
    {
        glUniform1fv(bindPoint, 1, (GLfloat*)&value); //location, number of elements, do you want gl to transpose matrix?, matrix
        return true;
    }
    return false;
}
