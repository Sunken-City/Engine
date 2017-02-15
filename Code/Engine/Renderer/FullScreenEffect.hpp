#pragma once
#include "Material.hpp"
#include "Engine/Math/MathUtils.hpp"

class FullScreenEffect
{
public:
    FullScreenEffect(Material* material);

    Material* m_material;
    uchar m_visibilityFilter;
};