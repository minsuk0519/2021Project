#include "Object.hpp"
#include "Graphic.hpp"
#include "Texture.hpp"
#include "LevelBase.hpp"
#include "Model.hpp"
#include "Engine.hpp"
#include "Light.hpp"

Object::Object(std::string name, unsigned int id, LevelBase* level) : mName(name), mID(id), mLevel(level) {}

void Object::Initialize()
{
}

void Object::Update()
{
    while (mRot.x > 180.0f) mRot.x -= 360.0f;
    while (mRot.x < -180.0f) mRot.x += 360.0f;
    while (mRot.y > 180.0f) mRot.y -= 360.0f;
    while (mRot.y < -180.0f) mRot.y += 360.0f;
    while (mRot.z > 180.0f) mRot.z -= 360.0f;
    while (mRot.z < -180.0f) mRot.z += 360.0f;

    mTransform = glm::mat4(1.0f);

    mTransform = glm::translate(mTransform, mPosition);

    mTransform = glm::rotate(mTransform, glm::radians(mRot.x), glm::vec3(1.0f, 0.0f, 0.0f));
    mTransform = glm::rotate(mTransform, glm::radians(mRot.y), glm::vec3(0.0f, 1.0f, 0.0f));
    mTransform = glm::rotate(mTransform, glm::radians(mRot.z), glm::vec3(0.0f, 0.0f, 1.0f));

    mTransform = glm::scale(mTransform, mScale);

    if (mName == "Box")
    {
        mRot.x += mPosition.x;
        mRot.y += mPosition.y;
        mRot.z += mPosition.z;
    }

    if (mName == "Rot")
    {
        mRot.y += 1.0;
    }

    float dis = glm::length(mPosition - mLevel->mCamera->mPosition);
    mLevel->mCamera->mDrawlist.insert(std::pair<float, Object*>(0, this));
}

void Object::Close()
{
}

void Object::Draw()
{
    Camera* cam = mLevel->mCamera;

    unsigned int programnumber = Graphic::mPrograms.at(mProgramID)->mNumber;


    glUseProgram(programnumber);

    unsigned int transformLoc;
    glUniformMatrix4fv(100, 1, GL_FALSE, glm::value_ptr(mTransform));

    transformLoc = glGetUniformLocation(programnumber, "objectcolor");
    glUniform3fv(transformLoc, 1, glm::value_ptr(mMaterialSpecularVector));

    if (mName != "Light")
        glUniform3fv(101, 1, glm::value_ptr(cam->mPosition));

    transformLoc = glGetUniformLocation(programnumber, "material.shininess");
    glUniform1f(transformLoc, mShinessFactor);

    //unsigned int iter = 0;
    //for(auto lit : mLevel->mLight)
    //{
    //    std::string arrname = "light[" + std::to_string(iter) + "].";
    //    transformLoc = glGetUniformLocation(programnumber, (arrname + "position").c_str());
    //    glUniform3fv(transformLoc, 1, glm::value_ptr(lit->mPosition));

    //    transformLoc = glGetUniformLocation(programnumber, (arrname + "ambient").c_str());
    //    glUniform3fv(transformLoc, 1, glm::value_ptr(lit->mMaterialAmbientVector));

    //    transformLoc = glGetUniformLocation(programnumber, (arrname + "diffuse").c_str());
    //    glUniform3fv(transformLoc, 1, glm::value_ptr(lit->mMaterialDiffuseVector));

    //    transformLoc = glGetUniformLocation(programnumber, (arrname + "specular").c_str());
    //    glUniform3fv(transformLoc, 1, glm::value_ptr(lit->mMaterialSpecularVector));

    //    transformLoc = glGetUniformLocation(programnumber, (arrname + "constant").c_str());
    //    glUniform1f(transformLoc, 1.0f);

    //    transformLoc = glGetUniformLocation(programnumber, (arrname + "linear").c_str());
    //    glUniform1f(transformLoc, 0.07f);

    //    transformLoc = glGetUniformLocation(programnumber, (arrname + "quadratic").c_str());
    //    glUniform1f(transformLoc, 0.017f);

    //    ++iter;
    //}

    //transformLoc = glGetUniformLocation(programnumber, "lightnum");
    //glUniform1i(transformLoc, iter);

    transformLoc = glGetUniformLocation(programnumber, "material.ambient");
    glUniform3fv(transformLoc, 1, glm::value_ptr(mMaterialAmbientVector));

    transformLoc = glGetUniformLocation(programnumber, "material.diffuse");
    glUniform3fv(transformLoc, 1, glm::value_ptr(mMaterialDiffuseVector));

    transformLoc = glGetUniformLocation(programnumber, "material.specular");
    glUniform3fv(transformLoc, 1, glm::value_ptr(mMaterialSpecularVector));

    transformLoc = glGetUniformLocation(programnumber, "material.texture");
    glUniform1i(transformLoc, 0);

    transformLoc = glGetUniformLocation(programnumber, "material.texture2");
    glUniform1i(transformLoc, 1);

    if (cam->mIsFlashlight)
    {
        transformLoc = glGetUniformLocation(programnumber, "flash.direction");
        glUniform3fv(transformLoc, 1, glm::value_ptr(cam->mFront));

        transformLoc = glGetUniformLocation(programnumber, "flash.light.position");
        glUniform3fv(transformLoc, 1, glm::value_ptr(cam->mPosition));

        transformLoc = glGetUniformLocation(programnumber, "flash.cutOff");
        glUniform1f(transformLoc, glm::cos(glm::radians(25.f)));

        transformLoc = glGetUniformLocation(programnumber, "flash.outercutOff");
        glUniform1f(transformLoc, glm::cos(glm::radians(35.f)));

        transformLoc = glGetUniformLocation(programnumber, "flash.light.constant");
        glUniform1f(transformLoc, 1.0f);

        transformLoc = glGetUniformLocation(programnumber, "flash.light.linear");
        glUniform1f(transformLoc, 0.09f);

        transformLoc = glGetUniformLocation(programnumber, "flash.light.quadratic");
        glUniform1f(transformLoc, 0.032f);

        transformLoc = glGetUniformLocation(programnumber, "flash.light.ambient");
        glUniform3fv(transformLoc, 1, glm::value_ptr(cam->mAmbientColor));

        transformLoc = glGetUniformLocation(programnumber, "flash.light.diffuse");
        glUniform3fv(transformLoc, 1, glm::value_ptr(cam->mDiffuseColor));

        transformLoc = glGetUniformLocation(programnumber, "flash.light.specular");
        glUniform3fv(transformLoc, 1, glm::value_ptr(cam->mSpecularColor));
    }

    transformLoc = glGetUniformLocation(programnumber, "isflash");
    glUniform1i(transformLoc, cam->mIsFlashlight);

    transformLoc = glGetUniformLocation(programnumber, "sundir");
    glUniform3fv(transformLoc, 1, glm::value_ptr(dynamic_cast<DirLight*>(mLevel->mSun)->mDirection));

    unsigned int slot = 0;

    if (mModel != nullptr)
    {
        mModel->Draw();

        return;
    }

    for (auto t : mTextures)
    {
        Graphic::mTextures[t]->Bind(slot++);

    }

    glBindVertexArray(Graphic::mVertexArrayObject[mVAO]);

    glDrawElements(GL_TRIANGLES, mFaceSize, GL_UNSIGNED_INT, 0);
}

void Object::DrawStencil()
{
    mTransform = glm::mat4(1.0f);

    mTransform = glm::translate(mTransform, mPosition);

    glm::vec3 AAaa = glm::vec3(glm::radians(mRot.x), glm::radians(mRot.y), glm::radians(mRot.z));

    mTransform = glm::rotate(mTransform, glm::radians(mRot.x), glm::vec3(1.0f, 0.0f, 0.0f));
    mTransform = glm::rotate(mTransform, glm::radians(mRot.y), glm::vec3(0.0f, 1.0f, 0.0f));
    mTransform = glm::rotate(mTransform, glm::radians(mRot.z), glm::vec3(0.0f, 0.0f, 1.0f));

    glm::vec3 tempscale = mScale * 1.1f;

    mTransform = glm::scale(mTransform, tempscale);

    glm::mat4 view = glm::mat4(1.0f);
    Camera* cam = mLevel->mCamera;
    view = glm::lookAt(cam->mPosition, cam->mPosition + cam->mFront, cam->mUp);

    glm::mat4 projection;
    projection = glm::perspective(glm::radians(90.0f), Window::GetAspectRatio(), 0.1f, 100.0f);

    unsigned int programnumber = Graphic::mPrograms.at(0)->mNumber;

    glUseProgram(programnumber);

    unsigned int loc = glGetUniformLocation(programnumber, "objectcolor");
    glUniform3fv(loc, 1, glm::value_ptr(glm::vec3(1.0f, 0.0f, 0.0f)));

    glUniformMatrix4fv(100, 1, GL_FALSE, glm::value_ptr(mTransform));

    glBindVertexArray(mVAO);

    glDrawElements(GL_TRIANGLES, mFaceSize, GL_UNSIGNED_INT, 0);
}

void Object::DrawShadow()
{
    glUniformMatrix4fv(100, 1, GL_FALSE, glm::value_ptr(mTransform));

    if (mModel != nullptr)
    {
        if (mModel->mIsInstance) return;

        mModel->Draw();

        return;
    }
    
    glBindVertexArray(Graphic::mVertexArrayObject[mVAO]);

    glDrawElements(GL_TRIANGLES, mFaceSize, GL_UNSIGNED_INT, 0);
}

void Object::SetColor(glm::vec3 col)
{
    mMaterialSpecularVector = col;
    mMaterialDiffuseVector = col * 0.5f;
    mMaterialAmbientVector = col * 0.2f;
}

Object* Object::CopyObject() const
{
    Object* newobject = new Object();

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

    return newobject;
}

Object::~Object()
{
}

unsigned int Object::GetID() const
{
    return mID;
}
