#include "Wall.hpp"
#include "Graphic.hpp"
#include "LevelBase.hpp"
#include "Light.hpp"
#include "Texture.hpp"

#include <glad/glad.h>

void Wall::Initialize()
{
}

void Wall::Update()
{
	Object::Update();
}

void Wall::Close()
{
}

Object* Wall::CopyObject() const
{
	Wall* newobject = new Wall();

	newobject->mVAO = mVAO;
	newobject->mProgramID = mProgramID;
	newobject->mPosition = mPosition;
	newobject->mRot = mRot;
	newobject->mScale = mScale;

	newobject->mFaceSize = mFaceSize;
	newobject->mName = mName;

	newobject->mShinessFactor = mShinessFactor;
	newobject->mMaterialAmbientVector = mMaterialAmbientVector;
	newobject->mMaterialDiffuseVector = mMaterialDiffuseVector;
	newobject->mMaterialSpecularVector = mMaterialSpecularVector;

	return dynamic_cast<Object*>(newobject);
}

void Wall::Draw()
{
	unsigned int number = Graphic::mPrograms.at(mProgramID)->mNumber;
	glUseProgram(number);

	glUniformMatrix4fv(100, 1, GL_FALSE, glm::value_ptr(mTransform));

	auto light = mLevel->mLight.at(0);

	unsigned int loc = glGetUniformLocation(number, "lightPos");
	glUniform3fv(loc, 1, glm::value_ptr(light->mPosition));

	glUniform3fv(101, 1, glm::value_ptr(mLevel->mCamera->mPosition));

	loc = glGetUniformLocation(number, "Texture");
	glUniform1i(loc, 0);

	loc = glGetUniformLocation(number, "normalTex");
	glUniform1i(loc, 1);

	loc = glGetUniformLocation(number, "parallaxTex");
	glUniform1i(loc, 2);

	loc = glGetUniformLocation(number, "scale2d");
	glUniform2fv(loc, 1, glm::value_ptr(mScale));

	glBindVertexArray(Graphic::mVertexArrayObject[GL::VAO::WALL_RECT]);

	int slot = 0;
	for (auto t : mTextures)
	{
		Graphic::mTextures[t]->Bind(slot++);
	}

	glDrawArrays(GL_TRIANGLES, 0, 6);
}

// wall won't cast shadow itself...
void Wall::DrawShadow() 
{
	mTransform = glm::scale(mTransform, glm::vec3(1.0f, 1.0f, 0.01f));

	Object::DrawShadow();

	mTransform = glm::scale(mTransform, glm::vec3(1.0f, 1.0f, 100.f));
}
