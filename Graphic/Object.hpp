#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <string>
#include <vector>

class LevelBase;
class Model;

enum OBJ_TYPE
{
	LIGHT_DIR = 0,
	LIGHT_POINT = 1,
	WALL = 2
};

class Object
{
private:
	friend class Interface;
	friend class LevelBase;
	friend class DataManager;
	friend class Graphic;

protected:
	float mShinessFactor = 0.6f;

	glm::vec3 mMaterialAmbientVector	= glm::vec3(0.0215f, 0.1745f, 0.0215f);
	glm::vec3 mMaterialDiffuseVector	= glm::vec3(0.07568f, 0.61424f, 0.07568f);
	glm::vec3 mMaterialSpecularVector	= glm::vec3(0.633f, 0.727811f, 0.633f);

	unsigned int mID = -1;
	std::string mName = "Object";
public:
	Model* mModel = nullptr;

	unsigned int mFaceSize = 36;
	LevelBase* mLevel = nullptr;
	
	std::vector<unsigned int> mTextures;
	Object(std::string name = "Object", unsigned int id = -1, LevelBase* level = nullptr);

	unsigned int mProgramID = -1;
	unsigned int mVAO = -1;
	glm::vec3 mPosition = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 mRot = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 mScale = glm::vec3(1.0f, 1.0f, 1.0f);
	glm::mat4 mTransform;
	
	virtual void Initialize();
	virtual void Update();
	virtual void Close();

	virtual void Draw();
	void DrawStencil();
	virtual void DrawShadow();

	void SetColor(glm::vec3 col);

	virtual Object* CopyObject() const;

	virtual ~Object();

	unsigned int GetID() const;
};