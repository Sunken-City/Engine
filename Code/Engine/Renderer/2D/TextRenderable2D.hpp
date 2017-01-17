#pragma once
#include "Renderable2D.hpp"
#include <string>
#include "Engine\Math\Transform2D.hpp"

class BufferedMeshRenderer;
class AABB2;
class BitmapFont;

class TextRenderable2D : public Renderable2D
{
public:
    ////CONSTRUCTORS/////////////////////////////////////////////////////////////////////
    TextRenderable2D(const std::string& text, const Transform2D& position, int orderingLayer = 0, bool isEnabled = true);
    virtual ~TextRenderable2D();

    //FUNCTIONS/////////////////////////////////////////////////////////////////////
    virtual void Update(float deltaSeconds) override;
    virtual void Render(BufferedMeshRenderer& renderer) override;
    virtual AABB2 GetBounds() override;
    virtual bool IsCullable() override { return false; };

    //MEMBER VARIABLES/////////////////////////////////////////////////////////////////////
    std::string m_text;
    Transform2D m_transform;
    BitmapFont* m_font;
};