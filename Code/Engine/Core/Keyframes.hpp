#pragma once

//-----------------------------------------------------------------------------------
template <typename T>
class KeyFrame
{
    T m_value;
    float m_parametric;
};

//-----------------------------------------------------------------------------------
template <typename T>
class KeyFramedProperty
{
    //-----------------------------------------------------------------------------------
    void AddKeyFrame(float paramKey, const T& valueAtKey);

    //-----------------------------------------------------------------------------------
    const T EvaluateAtParametric(float parametricValue);

    std::vector<KeyFrame<T>> m_keyframes;
};

//-----------------------------------------------------------------------------------
template <typename T>
class KeyFramedAnimation : public KeyFramedProperty
{
    T Evaluate(float time);
    Update(float deltaSeconds);

    KeyFramedProperty<T> m_keyframes;
    float m_durationSeconds;
    float m_currentSeconds;
};