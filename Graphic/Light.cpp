#include "Light.hpp"
#include "Texture.hpp"
#include "Graphic.hpp"
#include "LevelBase.hpp"
#include "Model.hpp"

#include <vector>

void Light::Initialize()
{
    glGenFramebuffers(1, &mFrameBufferObject);
    glBindFramebuffer(GL_FRAMEBUFFER, mFrameBufferObject);
    mDepthTexture = new Texture();

    this->Init();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Light::DrawOnTex()
{
    glUseProgram(Graphic::mPrograms[mDepthProgramID]->mNumber);
    glBindFramebuffer(GL_FRAMEBUFFER, mFrameBufferObject);
    glViewport(0, 0, DEPTH_TEXTURE_SIZE, DEPTH_TEXTURE_SIZE);
    glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
    glClear(GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    this->SendMatData();

    for (auto d : mLevel->mCamera->mDrawlist)
    {
        if (d.second->GetID() == mID)
        {
            continue;
        }

        d.second->DrawShadow();
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Light::DrawShadow()
{
}

void Light::Draw()
{
    if (mModel != nullptr)
    {
        unsigned int preID = mProgramID;
        mProgramID = 16;

        Object::Draw();

        mProgramID = preID;

        return;
    }

    Object::Draw();
}

void Light::Close()
{
    mDepthTexture->Close();
    delete mDepthTexture;

    glDeleteFramebuffers(1, &mFrameBufferObject);

	this->Shutdown();
}

Light::~Light()
{
}

void PointLight::Init()
{
    mDepthTexture->Initialize(TEXTURE_CONFIG_FLAG::DEPTH | TEXTURE_CONFIG_FLAG::CUBEMAP, TEXTURE_FORMAT::FORMAT_DEPTH);

    mDepthProgramID = GL::PROGRAM::DEPTHCUBE;

    mDepthProj = glm::perspective(glm::radians(90.0f), 1.0f, mNear_plane, mFar_plane);
}

void PointLight::Shutdown()
{
    mDepthTransforms.clear();
}

void PointLight::Update()
{
    mDepthTransforms.clear();
    mDepthTransforms.push_back(mDepthProj * glm::lookAt(mPosition, mPosition + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
    mDepthTransforms.push_back(mDepthProj * glm::lookAt(mPosition, mPosition + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
    mDepthTransforms.push_back(mDepthProj * glm::lookAt(mPosition, mPosition + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)));
    mDepthTransforms.push_back(mDepthProj * glm::lookAt(mPosition, mPosition + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)));
    mDepthTransforms.push_back(mDepthProj * glm::lookAt(mPosition, mPosition + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
    mDepthTransforms.push_back(mDepthProj * glm::lookAt(mPosition, mPosition + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));

    Object::Update();
}

void PointLight::SendMatData()
{
    int number;
    glGetIntegerv(GL_CURRENT_PROGRAM, &number);

    int lightindex = 0;

    for (unsigned int i = 0; i < 6; ++i)
    {
        std::string index = "lightMatrices[" + std::to_string(i) + "]";
        unsigned int loc = glGetUniformLocation(number, index.c_str());
        glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(mDepthTransforms.at(i)));
    }

    unsigned int transformLoc = glGetUniformLocation(number, "far_plane");
    glUniform1f(transformLoc, mFar_plane);

    transformLoc = glGetUniformLocation(number, "lightPos");
    glUniform3fv(transformLoc, 1, &mPosition.x);
}

void PointLight::SendLightData(unsigned int index)
{
    int number;
    glGetIntegerv(GL_CURRENT_PROGRAM, &number);

    std::string name = "depthCubeMap[" + std::to_string(index) + "]";

    unsigned int loc = glGetUniformLocation(number, name.c_str());
    glUniform1i(loc, index + 6);

    mDepthTexture->Bind(index + 6);
}

PointLight::~PointLight()
{
}

void DirLight::Init()
{
    mDepthTexture->Initialize(TEXTURE_CONFIG_FLAG::DEPTH, TEXTURE_FORMAT::FORMAT_DEPTH);

    mDepthProgramID = GL::PROGRAM::SHADOWMAP;
}

void DirLight::Shutdown()
{
}

void DirLight::Update()
{
    glm::mat4 lightProjection, lightView;
    float near_plane = 0.1f, far_plane = 100.0f;
    float ortho_plane = 15;

    lightView = glm::lookAt(mPosition, mDirection + mPosition, glm::vec3(1.0, 0.0, 0.0));
    lightProjection = glm::ortho(-ortho_plane, ortho_plane, -ortho_plane, ortho_plane, near_plane, far_plane);
    mLightMat = lightProjection * lightView;

    glm::mat4 mat = glm::mat4(1.0f);
    mat = glm::rotate(mat, glm::radians(mRot.x), glm::vec3(1.0f, 0.0f, 0.0f));
    mat = glm::rotate(mat, glm::radians(mRot.y), glm::vec3(0.0f, 1.0f, 0.0f));
    mat = glm::rotate(mat, glm::radians(mRot.z), glm::vec3(0.0f, 0.0f, 1.0f));

    mDirection = mat * glm::vec4(0.0f, -1.0f, 0.0f, 1.0f);

    Object::Update();
}

void DirLight::SendMatData()
{
    glUniformMatrix4fv(102, 1, GL_FALSE, glm::value_ptr(mLightMat));
}

void DirLight::SendLightData(unsigned int index)
{
    int number;
    glGetIntegerv(GL_CURRENT_PROGRAM, &number);

    unsigned int loc = glGetUniformLocation(number, "shadowTex");
    glUniform1i(loc, index);

    mDepthTexture->Bind(index);
}

DirLight::~DirLight()
{
}
