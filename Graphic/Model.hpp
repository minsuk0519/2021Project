#pragma once

#include <vector>
#include <string>

class Texture;

enum MODEL_TYPE
{
	MODEL_OBJ = 0,
	MODEL_GLTF = 1
};

class Model
{
private:
	friend class DataManager;
	friend class Object;

	std::string mName;
	
	std::vector<unsigned int> mSizes;
	unsigned int mNumMesh;
	unsigned int* mVAOs;
	unsigned int* mVBOs;
	unsigned int* mEBOs;

	Texture* mDiffuse = nullptr;
	Texture* mSpecular = nullptr;

	unsigned int mInstanceNum;

	float mInnertime = 0.0f;
public:
	bool mIsInstance = false;
	bool Load(std::string name);

	void Draw();

	void Close();
};

