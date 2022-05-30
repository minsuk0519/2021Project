#pragma once
#include <vector>

struct GLFWwindow;
class LevelBase;

namespace Callback
{
	static void glfw_error_callback(int error, const char* description);

	void Framebuffer_resize_callback(GLFWwindow* Window, int width, int height);

	void Keyboard_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

	void Mouse_callback(GLFWwindow* window, double xpos, double ypos);
}

struct Window
{
	static int Windowsize[2];
	static GLFWwindow* Windowptr;
	static float GetAspectRatio();
};

struct Engine
{
	static LevelBase* mCurrentLevel;
	static std::vector<LevelBase*> mLevels;
	static bool mOpenGUI;

	static void Initialize();
	static bool ShouldUpdate();
	static void Update();
	static void Close();
};