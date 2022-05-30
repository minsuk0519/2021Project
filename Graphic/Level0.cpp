#include "Level0.hpp"
#include "Graphic.hpp"
#include "Object.hpp"
#include "Engine.hpp"
#include "Input.hpp"
#include "Model.hpp"
#include "Texture.hpp"

#include <iostream>

void Level0::Initialize()
{

}

void Level0::Run()
{
    if (!Engine::mOpenGUI)
    {
        if (Input::IsTriggered(GLFW_KEY_F)) mCamera->mIsFlashlight = !mCamera->mIsFlashlight;
    }
}

void Level0::ShutDown()
{
}
