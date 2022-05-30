#pragma once
#include <IMGUI/imgui.h>
#include <IMGUI/imgui_impl_glfw.h>
#include <IMGUI/imgui_impl_opengl3.h>

#include <string>
#include <vector>

#include <glm/glm.hpp>

struct GLFWwindow;
class Shader;
class Program;
enum SHADER_TYPE;
class Object;
class LevelBase;

class Interface
{
private:
	static void ShaderSetting();
	static void ShaderSourceSetting(Program* program, bool changed, const std::string& title, SHADER_TYPE type);
	static void ObjectsSetting(Object* obj);
public:
	static void Initialize(GLFWwindow* window);
	static void Update();
	static void Close();

	static Object* mDetail;

	static float AMBIENT;
	static float DIFFUSE;
};