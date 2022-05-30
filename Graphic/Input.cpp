#include "Input.hpp"
#include "Graphic.hpp"
#include "Engine.hpp"

std::bitset<GLFW_KEY_LAST> Input::mPressed;
std::bitset<GLFW_KEY_LAST> Input::mTriggered;

float Input::mMousepos[2] = { 0.0f, 0.0f };

bool Input::IsTriggered(int keycode)
{
    return mTriggered[keycode];
}

bool Input::IsReleased(int keycode)
{
    return !mPressed[keycode];
}

bool Input::IsPressed(int keycode)
{
    return mPressed[keycode];
}

void Input::Update()
{
    mTriggered.reset();
}

void Input::KeyCallback(int key, int action)
{
    if (action == GLFW_RELEASE)
    {
        mPressed[key] = false;
        return;
    }

    if (mPressed[key] == false)
    {
        mTriggered[key] = true;
    }
    mPressed[key] = true;
}

void Input::MouseCallback(float x, float y)
{
    mMousepos[0] = x - Window::Windowsize[0] / 2;
    mMousepos[1] = Window::Windowsize[1] / 2 - y;
}
