#pragma once

#include "Engine/Math/Vector3.hpp"
#include "Engine/TextRendering/StringEffectFragment.hpp"
#include <deque>
#include "Engine/Math/Transform2D.hpp"

class MeshRenderer;
class BitmapFont;
class ShaderProgram;

//ENUMS/////////////////////////////////////////////////////////////////////
enum TextAlignment
{
    LEFT_ALIGNED,
    RIGHT_ALIGNED,
    CENTER_ALIGNED
};

//STRUCTS/////////////////////////////////////////////////////////////////////
struct TextInfo
{
    Transform2D transform;
    BitmapFont* baseFont;
    TextAlignment textAlignment = TextAlignment::LEFT_ALIGNED;
};

//-----------------------------------------------------------------------------------------------
class TextBox
{
public:

    //CONSTRUCTORS/////////////////////////////////////////////////////////////////////
    TextBox() {};
    TextBox(const TextInfo& info) {};
    TextBox(const Vector3& bottomLeft, const Vector3& upVector, const Vector3& rightVector, float width, float height, float scale, BitmapFont* baseFont, TextAlignment textAlignment = TextAlignment::LEFT_ALIGNED);
    ~TextBox();

    //FUNCTIONS/////////////////////////////////////////////////////////////////////
    void SetFromXMLNode(const struct XMLNode& node);
    void ResetAnimation();
    void Update(float deltaSeconds);
    void Render() const;
    inline void ShowBox() { m_renderBox = true; };
    inline void HideBox() { m_renderBox = false; };
    inline Vector3 GetPosition() { return m_bottomLeft; };
    inline void SetPosition(const Vector3& newPosition) { m_bottomLeft = newPosition; };

    //MEMBER VARIABLES/////////////////////////////////////////////////////////////////////
    TextBox* prev;
    TextBox* next;
    unsigned int m_orderingLayer;

    //PRIVATE FUNCTIONS/////////////////////////////////////////////////////////////////////
private:
    void EvaluateLine(std::deque<StringEffectFragment>& currLine, std::deque<StringEffectFragment>& fragmentQueue);
    void ConstructMeshes();
    void InitializeBorder();
    void DeleteTextRenderers();

    //MEMBER VARIABLES/////////////////////////////////////////////////////////////////////
    MeshRenderer* m_borderRenderer;
    std::vector<StringEffectFragment> m_fragments;
    std::vector<MeshRenderer*> m_textRenderers;
    Vector3 m_bottomLeft;
    Vector3 m_upVector;
    Vector3 m_rightVector;
    BitmapFont* m_baseFont;
    ShaderProgram* m_textShader;
    float m_width;
    float m_height;
    float m_scale;
    float m_totalTimeSinceReset;
    TextAlignment m_alignment;
    bool m_renderBox;
};