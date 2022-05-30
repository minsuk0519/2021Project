#include "Engine.hpp"
#include "Interface.hpp"
#include "Graphic.hpp"
#include "DataManager.hpp"
#include "Input.hpp"
#include "LevelBase.hpp"
#include "Utility.hpp"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>

int Window::Windowsize[2] = { -1,-1 };
GLFWwindow* Window::Windowptr = nullptr;

LevelBase* Engine::mCurrentLevel = nullptr;
std::vector<LevelBase*> Engine::mLevels;
bool Engine::mOpenGUI = false;

void Engine::Initialize()
{
    glfwSetErrorCallback(Callback::glfw_error_callback);
    if (!glfwInit())
    {
        std::cout << "Error occured : Cannot initialize GLFW!" << std::endl;
        return;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);

    GLFWwindow* window = glfwCreateWindow(1280, 720, "Window", NULL, NULL);
    Window::Windowsize[0] = 1280;
    Window::Windowsize[1] = 720;
    Window::Windowptr = window;
    if (window == NULL)
    {
        glfwTerminate();
        std::cout << "Error occured : Failed to create window!" << std::endl;
        return;
    }

    glfwSetFramebufferSizeCallback(window, Callback::Framebuffer_resize_callback);
    glfwSetKeyCallback(window, Callback::Keyboard_callback);
    glfwSetCursorPosCallback(window, Callback::Mouse_callback);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glfwMakeContextCurrent(window);
    glfwSwapInterval(GL_TRUE);

    bool err = (gladLoadGL() == 0);

    if (err)
    {
        std::cout << "Error occured : Failed to initialize OpenGL Loader GLAD!" << std::endl;
        return;
    }

    DataManager::LoadBaseData();
    Interface::Initialize(window);
    Graphic::Initialize();

    double tempcursorpos[2];
    glfwGetCursorPos(Window::Windowptr, tempcursorpos, (tempcursorpos + 1));
    Input::MouseCallback(static_cast<float>(tempcursorpos[0]), static_cast<float>(tempcursorpos[1]));
}

bool Engine::ShouldUpdate()
{
    return !glfwWindowShouldClose(Window::Windowptr);
}

void Engine::Update()
{
    float hi = Utility::GetTick();

    glfwPollEvents();

    mCurrentLevel->Update();

    Graphic::Draw();

    if (Input::IsPressed(GLFW_KEY_ESCAPE))
    {
        glfwSetWindowShouldClose(Window::Windowptr, true);
    }
    if (Input::IsTriggered(GLFW_KEY_TAB))
    {
        Engine::mOpenGUI = !Engine::mOpenGUI;
        if (Engine::mOpenGUI)
        {
            glfwSetInputMode(Window::Windowptr, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
        else
        {
            glfwSetInputMode(Window::Windowptr, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
    }
    if (Input::IsTriggered(GLFW_KEY_PAGE_UP))
    {
        mCurrentLevel->Close();

        mCurrentLevel = mLevels.at(1);
        DataManager::SetLevel(mCurrentLevel);
    }
    if (Input::IsTriggered(GLFW_KEY_PAGE_DOWN))
    {
        mCurrentLevel->Close();

        mCurrentLevel = mLevels.at(0);

        DataManager::SetLevel(mCurrentLevel);
    }
    if (Input::IsTriggered(GLFW_KEY_1))
    {
        mCurrentLevel->Close();

        mCurrentLevel = mLevels.at(1);

        DataManager::SetLevel(mCurrentLevel);
    }
    if (Input::IsTriggered(GLFW_KEY_2))
    {
        mCurrentLevel->Close();

        mCurrentLevel = mLevels.at(2);

        DataManager::SetLevel(mCurrentLevel);
    }
    if (Input::IsTriggered(GLFW_KEY_3))
    {
        mCurrentLevel->Close();

        mCurrentLevel = mLevels.at(3);

        DataManager::SetLevel(mCurrentLevel);
    }

    Input::Update();

    Interface::Update();

    glfwSwapBuffers(Window::Windowptr);
}

void Engine::Close()
{
    DataManager::SaveBaseData();

    Interface::Close();
    Graphic::Close();

    mCurrentLevel->Close();

    for (auto level : mLevels)
    {
        delete level;
    }
    mLevels.clear();

    glfwDestroyWindow(Window::Windowptr);
    glfwTerminate();
}

void Callback::glfw_error_callback(int error, const char* description)
{
    std::cerr << "GLFW Error " << description << std::endl;
}

void Callback::Framebuffer_resize_callback(GLFWwindow* Window, int width, int height)
{
    glViewport(0, 0, width, height);

    Graphic::ResizeEvent(width, height);
}

void Callback::Keyboard_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    Input::KeyCallback(key, action);
}

void Callback::Mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    Input::MouseCallback(static_cast<float>(xpos), static_cast<float>(ypos));
}
