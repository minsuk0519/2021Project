#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <bitset>

class Input
{
private:
	static std::bitset<GLFW_KEY_LAST> mPressed;
	static std::bitset<GLFW_KEY_LAST> mTriggered;

public:
	static float mMousepos[2];

	static bool IsTriggered(int keycode);
	static bool IsReleased(int keycode);
	static bool IsPressed(int keycode);

	static void Update();
	static void KeyCallback(int key, int action);

	static void MouseCallback(float x, float y);
};

