#pragma once

#include <glm/glm.hpp>

#include <vector>
#include <iostream>

struct Camera;
class Object;
class DirLight;
class Light;

class LevelBase
{
private:
	friend class Interface;
	friend class DataManager;

protected:
	std::vector<Object*> mObjects;
	glm::vec3 mBackground = glm::vec3(0.45f, 0.55f, 0.60f);

	unsigned int mCurrentID = 0;
	virtual void Initialize() = 0;
	virtual void Run() = 0;
	virtual void ShutDown() = 0;
public:
	unsigned int mID = -1;
	std::string mName = "Level";

	std::vector<Light*> mLight;
	DirLight* mSun = nullptr;
	Camera* mCamera = nullptr;

	void Init();
	void Update();
	Object* CreateObject();
	Object* AddObject(Object* obj);
	void Close();

};