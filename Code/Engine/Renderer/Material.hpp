#pragma once
#include <string>
#include "Engine/Renderer/ShaderProgram.hpp"

class Matrix4x4;
class Texture;

//STRUCTS//////////////////////////////////////////////////////////////////////////
//--------------------------------------------------------------------------------------------------------------------------------------------------------
struct RenderState
{
    //ENUMS//////////////////////////////////////////////////////////////////////////
    enum class DepthTestingMode
    {
        ON,
        OFF,
        DISABLE_WRITE,
        //XRAY,
        NUM_MODES
    };

    enum class FaceCullingMode
    {
        CULL_BACK_FACES,
        RENDER_BACK_FACES,
        NUM_MODES
    };

    enum class BlendMode
    {
        ADDITIVE_BLEND,
        ALPHA_BLEND,
        INVERTED_BLEND,
        NUM_MODES
    };
    
    //CONSTRUCTORS//////////////////////////////////////////////////////////////////////////
    RenderState(DepthTestingMode depthTesting = DepthTestingMode::ON, FaceCullingMode faceCulling = FaceCullingMode::CULL_BACK_FACES, BlendMode blendMode = BlendMode::ALPHA_BLEND);
    ~RenderState();

    //FUNCTIONS//////////////////////////////////////////////////////////////////////////
    void SetState() const;
    void ClearState() const;

    //MEMBER VARIABLES//////////////////////////////////////////////////////////////////////////
    DepthTestingMode depthTestingMode;
    FaceCullingMode faceCullingMode;
    BlendMode blendMode;
};

//--------------------------------------------------------------------------------------------------------------------------------------------------------
class Material
{
public:
    //CONSTRUCTORS//////////////////////////////////////////////////////////////////////////
    Material() {};
    Material(ShaderProgram* program, const RenderState& renderState);
    ~Material();

    //FUNCTIONS//////////////////////////////////////////////////////////////////////////
    void SetMatrices(const Matrix4x4& model, const Matrix4x4& view, const Matrix4x4& projection);
    void BindAvailableTextures() const;
    void UnbindAvailableTextures() const;
    void SetDiffuseTexture(const std::string& diffusePath);
    void SetDiffuseTexture(Texture* texture);
    void SetNormalTexture(const std::string& normalPath);
    void SetNormalTexture(Texture* texture);
    void SetEmissiveTexture(const std::string& emissivePath);
    void SetEmissiveTexture(Texture* texture);
    void SetNoiseTexture(const std::string& noisePath);
    void SetNoiseTexture(Texture* texture);
    void SetTexture(const char* texName, unsigned int textureObjectID);
    void SetUpRenderState() const;
    void CleanUpRenderState() const;
    void ReplaceSampler(unsigned int newSamplerID);
    //NOTE: for the array passings, you have to address the uniform like this "gArray[CurrentIndex]:"
    inline bool SetUniform(size_t hashedName, void* value) { m_shaderProgram->SetUniform(hashedName, value); };
    inline void SetVec4Uniform(size_t hashedName, const Vector4& value) { m_shaderProgram->SetVec4Uniform(m_shaderProgram->GetBindPoint(hashedName), value); };
    inline void SetVec3Uniform(size_t hashedName, const Vector3& value) { m_shaderProgram->SetVec3Uniform(m_shaderProgram->GetBindPoint(hashedName), value); };
    inline void SetVec2Uniform(size_t hashedName, const Vector2& value) { m_shaderProgram->SetVec2Uniform(m_shaderProgram->GetBindPoint(hashedName), value); };
    inline void SetFloatUniform(size_t hashedName, float value) { m_shaderProgram->SetFloatUniform(m_shaderProgram->GetBindPoint(hashedName), value); };
    inline void SetIntUniform(size_t hashedName, int value) { m_shaderProgram->SetIntUniform(m_shaderProgram->GetBindPoint(hashedName), value); };
    //Slower variants of the above that constantly look up the indexes.
    inline void SetVec4Uniform(const char* name, const Vector4& value) { m_shaderProgram->SetVec4Uniform(name, value); };
    inline void SetVec3Uniform(const char* name, const Vector3& value) { m_shaderProgram->SetVec3Uniform(name, value); };
    inline void SetVec2Uniform(const char* name, const Vector2& value) { m_shaderProgram->SetVec2Uniform(name, value); };
    inline void SetFloatUniform(const char* name, float value) { m_shaderProgram->SetFloatUniform(name, value); };
    inline void SetIntUniform(const char* name, int value) { m_shaderProgram->SetIntUniform(name, value); };

    //DO NOT USE THESE. They're not actually working. The function for an array expects multiple passed in by reference, and I'm NOT passing any more than 1.
    inline void SetMatrix4x4Uniform(const char* name, Matrix4x4& value, unsigned int arrayIndex) { m_shaderProgram->SetMatrix4x4Uniform(name, value, arrayIndex); };
    inline void SetVec4Uniform(const char* name, const Vector4& value, unsigned int arrayIndex) { m_shaderProgram->SetVec4Uniform(name, value, arrayIndex); };
    inline void SetVec3Uniform(const char* name, const Vector3& value, unsigned int maxNumArrayIndexes) { m_shaderProgram->SetVec3Uniform(name, value, maxNumArrayIndexes); };
    inline void SetFloatUniform(const char* name, float value, unsigned int arrayIndex) { m_shaderProgram->SetFloatUniform(name, value, arrayIndex); };
    inline void SetIntUniform(const char* name, int value, unsigned int arrayIndex) { m_shaderProgram->SetIntUniform(name, value, arrayIndex); };
    
    //MEMBER VARIABLES//////////////////////////////////////////////////////////////////////////
    ShaderProgram* m_shaderProgram;
    RenderState m_renderState;

    unsigned int m_samplerID;
    unsigned int m_diffuseID;
    unsigned int m_normalID;
    unsigned int m_emissiveID;
    unsigned int m_noiseID;
};
