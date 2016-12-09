#include "Engine/UI/WidgetBase.hpp"
#include "Engine/Math/Vector2.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Fonts/BitmapFont.hpp"
#include "../Math/Matrix4x4.hpp"

//-----------------------------------------------------------------------------------
WidgetBase::WidgetBase()
{
    m_propertiesForAllStates.Set<Vector2>("Offset", Vector2::ZERO);
    m_propertiesForAllStates.Set<Vector2>("Size", Vector2::ONE);
    m_propertiesForAllStates.Set<RGBA>("BackgroundColor", RGBA::WHITE);
    m_propertiesForAllStates.Set<RGBA>("EdgeColor", RGBA::WHITE);
    m_propertiesForAllStates.Set<float>("Opacity", 1.0f);
}

//-----------------------------------------------------------------------------------
WidgetBase::~WidgetBase()
{
    for (WidgetBase* child : m_children)
    {
        delete child;
    }
}

//-----------------------------------------------------------------------------------
void WidgetBase::Update(float deltaSeconds)
{
    for (WidgetBase* child : m_children)
    {
        child->Update(deltaSeconds);
    }
}

//-----------------------------------------------------------------------------------
void WidgetBase::Render() const
{
    for (WidgetBase* child : m_children)
    {
        child->Render();
    }
}

//-----------------------------------------------------------------------------------
void WidgetBase::AddChild(WidgetBase* child)
{
    m_children.push_back(child);
    child->m_parent = this;
}

//-----------------------------------------------------------------------------------
AABB2 WidgetBase::GetSmallestBoundsAroundChildren()
{
    AABB2 smallestBounds = AABB2(Vector2::ZERO, Vector2::ZERO);

    for (WidgetBase* child : m_children)
    {
        AABB2 childBounds = child->GetBounds();
        smallestBounds.mins.x = (childBounds.mins.x < smallestBounds.mins.x) ? childBounds.mins.x : smallestBounds.mins.x;
        smallestBounds.mins.y = (childBounds.mins.y < smallestBounds.mins.y) ? childBounds.mins.y : smallestBounds.mins.y;
        smallestBounds.maxs.x = (childBounds.maxs.x > smallestBounds.maxs.x) ? childBounds.maxs.x : smallestBounds.maxs.x;
        smallestBounds.maxs.y = (childBounds.maxs.y > smallestBounds.maxs.y) ? childBounds.maxs.y : smallestBounds.maxs.y;
    }

    return smallestBounds;
}

//-----------------------------------------------------------------------------------
void WidgetBase::BuildFromXMLNode(XMLNode& node)
{
    const char* horizontalOffset = node.getAttribute("HorizontalOffset");
    const char* verticalOffset = node.getAttribute("VerticalOffset");
    const char* textColorAttribute = node.getAttribute("TextColor");
    const char* textSizeAttribute = node.getAttribute("FontSize");

    Vector2 offset = m_propertiesForAllStates.Get<Vector2>("Offset");

    if (horizontalOffset)
    {
        offset.x = std::stof(horizontalOffset);
    }
    if (verticalOffset)
    {
        offset.y = std::stof(verticalOffset);
    }

    m_propertiesForAllStates.Set<Vector2>("Offset", offset);
    m_propertiesForAllStates.Set<Vector2>("Size", Vector2::ONE);
    m_propertiesForAllStates.Set<RGBA>("BackgroundColor", RGBA::WHITE);
    m_propertiesForAllStates.Set<RGBA>("EdgeColor", RGBA::WHITE);
    m_propertiesForAllStates.Set<float>("Opacity", 1.0f);
}

//-----------------------------------------------------------------------------------
Vector2 WidgetBase::GetParentOffsets() const
{
    Vector2 parentOffsets = Vector2::ZERO;
    WidgetBase* parent = m_parent;
    while (parent)
    {
        parentOffsets += parent->m_propertiesForAllStates.Get<Vector2>("Offset");
        parent = parent->m_parent;
    }
    return parentOffsets;
}

//-----------------------------------------------------------------------------------
Matrix4x4 WidgetBase::GetModelMatrix() const
{
    Matrix4x4 model = Matrix4x4::IDENTITY;
    Matrix4x4::MatrixMakeTranslation(&model, Vector3(m_propertiesForAllStates.Get<Vector2>("Offset"), 0.0f));
    return model;
}
