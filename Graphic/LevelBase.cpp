#include "LevelBase.hpp"
#include "Object.hpp"
#include "Graphic.hpp"
#include "Input.hpp"
#include "Engine.hpp"
#include "Light.hpp"
#include "DataManager.hpp"

void LevelBase::Init()
{
	//mCamera = new Camera();

	this->Initialize();
}

void LevelBase::Update()
{
	if (!Engine::mOpenGUI)
	{
		GLFWwindow* window = Window::Windowptr;

		int vertical = 0;
		int horizontal = 0;
		if (Input::IsPressed(GLFW_KEY_W)) vertical += 1;
		if (Input::IsPressed(GLFW_KEY_S)) vertical -= 1;
		if (Input::IsPressed(GLFW_KEY_D)) horizontal += 1;
		if (Input::IsPressed(GLFW_KEY_A)) horizontal -= 1;
		if (vertical != 0 || horizontal != 0)
		{
			mCamera->Update(vertical, horizontal);
		}
	}
	mCamera->Look(Input::mMousepos[0], Input::mMousepos[1], !Engine::mOpenGUI);


	for (auto obj : mObjects)
	{
		obj->Update();
	}

	this->Run();
}

Object* LevelBase::CreateObject()
{
	Object* obj = new Object("Object", mCurrentID++, this);

	return obj;
}

Object* LevelBase::AddObject(Object* obj)
{
	obj->mID = mCurrentID++;
	obj->mLevel = this;
	mObjects.push_back(obj);

	return obj;
}

void LevelBase::Close()
{
	this->ShutDown();

	for (auto obj : mObjects)
	{
		obj->Close();

		delete obj;
	}

	mObjects.clear();

	mLight.clear();


	delete mCamera;
}