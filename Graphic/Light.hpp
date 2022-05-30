#pragma once
#include "Object.hpp"

#include <glm/glm.hpp>

class Texture;
class DataManager;
class Interface;

class Light : public Object
{
private:
	friend class Graphic;
	friend class DataManager;
	friend class Interface;

	float mConstant = 1.0f;
	float mLinear = 0.07f;
	float mQuadratic = 0.017f;

	virtual void Init() = 0;
	virtual void Shutdown() = 0;
	virtual void Update() = 0;

protected:
	Texture* mDepthTexture;
	unsigned int mFrameBufferObject;

	unsigned int mDepthProgramID;
public:
	void Initialize();

	glm::mat4 mLightMat;

	virtual void SendMatData() = 0;
	virtual void SendLightData(unsigned int index) = 0;

	void DrawOnTex();
	virtual void DrawShadow() override;

	virtual void Draw() override;
	void Close() override;

	virtual ~Light();
};

class DirLight : public Light
{
private:
	void Init() override;
	void Shutdown() override;
	void Update() override;
public:
	glm::vec3 mDirection = glm::vec3(0.0f,-1.0f,0.0f);

	void SendMatData() override;
	void SendLightData(unsigned int index) override;

	virtual ~DirLight();
};

class PointLight : public Light
{
private:
	unsigned int mLightID;

	std::vector<glm::mat4> mDepthTransforms;
	glm::mat4 mDepthProj;

	float mNear_plane = 1.f;
	float mFar_plane = 100.0f;
	
	void Init() override;
	void Shutdown() override;
	void Update() override;
public:
	void SendMatData() override;
	void SendLightData(unsigned int index) override;

	virtual ~PointLight();
};

