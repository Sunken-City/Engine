#pragma once
#include "Engine/Math/Matrix4x4.hpp"
#include "Engine/Renderer/Vertex.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

class Material;
class Mesh;

typedef unsigned int GLuint;
typedef void (BindMeshToVAOForVertex)(GLuint vao, GLuint vbo, GLuint ibo, ShaderProgram* program);
class MeshRenderer
{
public:
    //CONSTRUCTORS//////////////////////////////////////////////////////////////////////////
    MeshRenderer();
    MeshRenderer(Mesh* mesh, Material* material);
    ~MeshRenderer();

    //FUNCTIONS//////////////////////////////////////////////////////////////////////////
    void Render() const;
    void RotateAround(float degrees, const Vector3& axis);
    void SetModelMatrix(const Matrix4x4& newModelMatrix);
    void SetPosition(const Vector3& worldPosition);
    void SetVec3Uniform(const char* uniformName, const Vector3& value);
    
    //MEMBER VARIABLES//////////////////////////////////////////////////////////////////////////
    Material* m_material;
    Mesh* m_mesh;
    Matrix4x4 m_model;

private:
    GLuint m_vaoID;
    mutable BindMeshToVAOForVertex* m_lastVertexBindFunctionPointer = nullptr;

    MeshRenderer(const MeshRenderer&);
    void BindToVAO() const;
};