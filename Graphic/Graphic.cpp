#include "Graphic.hpp"
#include "Utility.hpp"
#include "DataManager.hpp"
#include "Texture.hpp"
#include "Engine.hpp"
#include "LevelBase.hpp"
#include "Object.hpp"
#include "Light.hpp"
#include "Model.hpp"

#include <GLFW/glfw3.h>

#include <iostream>

std::unordered_map<unsigned int, Program*> Graphic::mPrograms;
std::unordered_map<unsigned int, Shader*> Graphic::mVertexShader;
std::unordered_map<unsigned int, Shader*> Graphic::mFragmentShader;
std::unordered_map<unsigned int, Shader*> Graphic::mGeometryShader;
std::unordered_map<unsigned int, Texture*> Graphic::mTextures;

unsigned int Graphic::mVertexArrayObject[GL::VAO::MAX];
unsigned int Graphic::mVertexBufferObject[GL::VBO::MAX];
unsigned int Graphic::mElementBufferObject[GL::EBO::MAX];
unsigned int Graphic::mUniformBufferObject[GL::UBO::MAX];
unsigned int Graphic::mFrameBufferObject[GL::FBO::MAX];

Texture* Graphic::mScreenTexture;
unsigned int Graphic::mRenderBufferObject;
unsigned int Graphic::mMultisampleTex[5];
unsigned int Graphic::mNoiseTex;

Texture* Graphic::mBlurTexture[3];
Texture* Graphic::mGBufferTex[5];
Texture* Graphic::mSSAOTex[2];

std::vector<Texture*> Graphic::mWindowSizeTexture;

Program::Program(std::string name) : mName(name)
{
    mNumber = glCreateProgram();
}

void Program::Initialize(unsigned int vertex, unsigned int fragment, unsigned int geometry)
{
    mVertexID = vertex;
    mFragmentID = fragment;
    mGeometryID = geometry;
    std::string err;

    Shader* Vertex = Graphic::mVertexShader[vertex];
    Shader* Fragment = Graphic::mFragmentShader[fragment];
    Shader* Geometry = nullptr;
    if(geometry != -1) Geometry = Graphic::mGeometryShader[geometry];

    Vertex->ResetSource();

    int vertexShader = Vertex->Compile(err);

    if (!err.empty())
    {
        std::cout << "Error occured : Failed to compile vertex shader!" << err << std::endl;
    }

    Fragment->ResetSource();

    int fragmentShader = Fragment->Compile(err);

    if (!err.empty())
    {
        std::cout << "Error occured : Failed to compile fragment shader!" << err << std::endl;
    }

    if (Geometry != nullptr)
    {
        Geometry->ResetSource();
        int geometryShader = Geometry->Compile(err);

        if (!err.empty())
        {
            std::cout << "Error occured : Failed to compile geometry shader!" << err << std::endl;
        }

        LinkProgram(vertexShader, fragmentShader, geometryShader);
    }
    else
    {
        LinkProgram(vertexShader, fragmentShader);
    }

    if (mName.empty())
    {
        mName = "Program" + std::to_string(mNumber);
    }
}

void Program::Reset(unsigned int vert, unsigned int frag)
{
    glDeleteProgram(mNumber);

    mNumber = glCreateProgram();

    LinkProgram(vert, frag);

    glUseProgram(mNumber);
}

void Program::Close()
{
    glDeleteProgram(mNumber);
}

void Program::LinkProgram(unsigned int Vertex, unsigned int Fragment, unsigned int Geometry)
{
    glAttachShader(mNumber, Vertex);
    glAttachShader(mNumber, Fragment);
    if(Geometry != -1) glAttachShader(mNumber, Geometry);
    glLinkProgram(mNumber);
    GLint isLinked = 0;
    glGetProgramiv(mNumber, GL_LINK_STATUS, &isLinked);
    if (isLinked == GL_FALSE)
    {
        GLint maxLength = 0;
        glGetProgramiv(mNumber, GL_INFO_LOG_LENGTH, &maxLength);

        char* errorLog = new char[maxLength];
        glGetProgramInfoLog(mNumber, maxLength, &maxLength, errorLog);
        std::cout << "Error occured : Failed to link program!" << errorLog <<std::endl;
        delete[] errorLog;

        return;
    }

    glDeleteShader(Vertex);
    glDeleteShader(Fragment);
    if (Geometry != -1) glDeleteShader(Geometry);
}

//this function is deprecated do not use it
int Program::CompileShader(const char* shadersource, GLint shadertype)
{
    unsigned int Shaderid;
    Shaderid = glCreateShader(shadertype);

    glShaderSource(Shaderid, 1, &shadersource, NULL);
    glCompileShader(Shaderid);
    GLint isCompiled = 0;
    glGetShaderiv(Shaderid, GL_COMPILE_STATUS, &isCompiled);
    if (isCompiled == GL_FALSE)
    {
        GLint maxLength = 0;
        glGetShaderiv(Shaderid, GL_INFO_LOG_LENGTH, &maxLength);

        char* errorLog = new char[maxLength];
        glGetShaderInfoLog(Shaderid, maxLength, &maxLength, errorLog);
        std::cout << "Failed to compile shader : " << errorLog << std::endl;
        delete[] errorLog;

        return -1;
    }

    return Shaderid;
}

void Graphic::SendUBO()
{
    Camera* cam = Engine::mCurrentLevel->mCamera;
    glBindBuffer(GL_UNIFORM_BUFFER, mUniformBufferObject[GL::UBO::VIEW_PROJECTION]);
    glm::mat4 view = glm::mat4(1.0f);
    view = glm::lookAt(cam->mPosition, cam->mPosition + cam->mFront, cam->mUp);
    glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(view));
}

void Graphic::Initialize()
{
    glEnable(GL_DEPTH_TEST);
    //glEnable(GL_STENCIL_TEST);

    glDepthFunc(GL_LESS);
    //glStencilMask(0X00);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);

    glEnable(GL_FRAMEBUFFER_SRGB);
    glEnable(GL_MULTISAMPLE);

    //glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CW);

    glGenBuffers(GL::VBO::MAX, mVertexBufferObject);
    glGenBuffers(GL::EBO::MAX, mElementBufferObject);
    glGenVertexArrays(GL::VAO::MAX, mVertexArrayObject);
    glGenBuffers(GL::UBO::MAX, mUniformBufferObject);
    glGenFramebuffers(GL::FBO::MAX, mFrameBufferObject);

    for (auto pro : mPrograms)
    {
        unsigned int uniformIDX = glGetUniformBlockIndex(pro.second->mNumber, "VPmat");

        if (uniformIDX == -1)
        {
            continue;
        }

        glUniformBlockBinding(pro.second->mNumber, uniformIDX, 0);
    }

    glBindBuffer(GL_UNIFORM_BUFFER, mUniformBufferObject[GL::UBO::VIEW_PROJECTION]);
    glm::mat4 projection;
    projection = glm::perspective(glm::radians(90.0f), Window::GetAspectRatio(), PROJECTION_NEAR_FACE, PROJECTION_FAR_FACE);
    glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), nullptr, GL_STATIC_DRAW);
    glBindBufferRange(GL_UNIFORM_BUFFER, 0, mUniformBufferObject[GL::UBO::VIEW_PROJECTION], 0, 2 * sizeof(glm::mat4));
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(projection));
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    glGenRenderbuffers(1, &mRenderBufferObject);
    glBindFramebuffer(GL_FRAMEBUFFER, mFrameBufferObject[GL::FBO::MSAA]);
    glGenTextures(5, mMultisampleTex);

    for (int i = 1; i < 5; ++i)
    {
        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, mMultisampleTex[i]);
        glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA32F, Window::Windowsize[0], Window::Windowsize[1], GL_TRUE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D_MULTISAMPLE, mMultisampleTex[i], 0);
    }

    {
        unsigned int attachments[5] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4 };
        glDrawBuffers(5, attachments);
    }

    glBindRenderbuffer(GL_RENDERBUFFER, mRenderBufferObject);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH24_STENCIL8, Window::Windowsize[0], Window::Windowsize[1]);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, mRenderBufferObject);


    glBindFramebuffer(GL_FRAMEBUFFER, mFrameBufferObject[GL::FBO::POST_PROCESS]);
    mScreenTexture = new Texture();
    mScreenTexture->Initialize(TEXTURE_CONFIG_FLAG::WINDOWSIZE | TEXTURE_CONFIG_FLAG::FRAMEBUFFER, TEXTURE_FORMAT::FORMAT_FLOAT);

    mBlurTexture[0] = new Texture();
    mBlurTexture[0]->Initialize(TEXTURE_CONFIG_FLAG::WINDOWSIZE | TEXTURE_CONFIG_FLAG::FRAMEBUFFER, TEXTURE_FORMAT::FORMAT_FLOAT_3, 1);

    {
        unsigned int attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
        glDrawBuffers(2, attachments);
    }

    for (int i = 0; i < 2; ++i)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, mFrameBufferObject[GL::FBO::BLUR_HORIZONTAL + i]);

        mBlurTexture[i + 1] = new Texture();
        mBlurTexture[i + 1]->Initialize(TEXTURE_CONFIG_FLAG::WINDOWSIZE | TEXTURE_CONFIG_FLAG::FRAMEBUFFER | TEXTURE_CONFIG_FLAG::CLAMP, TEXTURE_FORMAT::FORMAT_FLOAT);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, mFrameBufferObject[GL::FBO::GEOM_BUFFER]);
    for (int i = 0; i < 5; ++i)
    {
        mGBufferTex[i] = new Texture();
        if (i == 1)
        {
            mGBufferTex[i]->Initialize(TEXTURE_CONFIG_FLAG::WINDOWSIZE | TEXTURE_CONFIG_FLAG::FRAMEBUFFER | TEXTURE_CONFIG_FLAG::CLAMP, TEXTURE_FORMAT::FORMAT_FLOAT_3, i);
            continue;
        }
        mGBufferTex[i]->Initialize(TEXTURE_CONFIG_FLAG::WINDOWSIZE | TEXTURE_CONFIG_FLAG::FRAMEBUFFER, TEXTURE_FORMAT::FORMAT_FLOAT, i);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, mFrameBufferObject[GL::FBO::SSAO]);
    mSSAOTex[0] = new Texture();
    mSSAOTex[0]->Initialize(TEXTURE_CONFIG_FLAG::WINDOWSIZE | TEXTURE_CONFIG_FLAG::FRAMEBUFFER, TEXTURE_FORMAT::FORMAT_RED);

    glBindFramebuffer(GL_FRAMEBUFFER, mFrameBufferObject[GL::FBO::SSAO_BLUR]);
    mSSAOTex[1] = new Texture();
    mSSAOTex[1]->Initialize(TEXTURE_CONFIG_FLAG::WINDOWSIZE | TEXTURE_CONFIG_FLAG::FRAMEBUFFER, TEXTURE_FORMAT::FORMAT_RED);

    std::vector<glm::vec3> noisePixel;
    for (unsigned int i = 0; i < 16; ++i)
    {
        glm::vec3 noise = glm::vec3(Utility::GetRand(-1.0f, 1.0f), Utility::GetRand(-1.0f, 1.0f), 0.0f);
        noise = glm::normalize(noise);
        noisePixel.push_back(noise);
    }

    glGenTextures(1, &mNoiseTex);
    glBindTexture(GL_TEXTURE_2D, mNoiseTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 4, 4, 0, GL_RGBA, GL_FLOAT, noisePixel.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    float RectVertices[] = {
       -0.5f,-0.5f,   0.0f, 0.0f,
       -0.5f, 0.5f,   0.0f, 1.0f,
        0.5f, 0.5f,   1.0f, 1.0f,
        0.5f,-0.5f,   1.0f, 0.0f
    };

    unsigned int RectIndices[] = {
        0, 1, 3,
        1, 2, 3
    };

    float CubeVertices[] = {
        -0.5f,-0.5f,-0.5f,   0.0f, 0.0f,-1.0f,   0.0f, 0.0f,
         0.5f,-0.5f,-0.5f,   0.0f, 0.0f,-1.0f,   10.0f, 0.0f,
         0.5f, 0.5f,-0.5f,   0.0f, 0.0f,-1.0f,   10.0f, 10.0f,
        -0.5f, 0.5f,-0.5f,   0.0f, 0.0f,-1.0f,   0.0f, 10.0f,
                                                
        -0.5f,-0.5f, 0.5f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,
         0.5f,-0.5f, 0.5f,   0.0f, 0.0f, 1.0f,   10.0f, 0.0f,
         0.5f, 0.5f, 0.5f,   0.0f, 0.0f, 1.0f,   10.0f, 10.0f,
        -0.5f, 0.5f, 0.5f,   0.0f, 0.0f, 1.0f,   0.0f, 10.0f,
                                                
        -0.5f, 0.5f, 0.5f,  -1.0f, 0.0f, 0.0f,   10.0f, 0.0f,
        -0.5f, 0.5f,-0.5f,  -1.0f, 0.0f, 0.0f,   10.0f, 10.0f,
        -0.5f,-0.5f,-0.5f,  -1.0f, 0.0f, 0.0f,   0.0f, 10.0f,
        -0.5f,-0.5f, 0.5f,  -1.0f, 0.0f, 0.0f,   0.0f, 0.0f,
                                                
         0.5f, 0.5f, 0.5f,   1.0f, 0.0f, 0.0f,   10.0f, 0.0f,
         0.5f, 0.5f,-0.5f,   1.0f, 0.0f, 0.0f,   10.0f, 10.0f,
         0.5f,-0.5f,-0.5f,   1.0f, 0.0f, 0.0f,   0.0f, 10.0f,
         0.5f,-0.5f, 0.5f,   1.0f, 0.0f, 0.0f,   0.0f, 0.0f,
                                                
        -0.5f,-0.5f,-0.5f,   0.0f,-1.0f, 0.0f,   0.0f, 10.0f,
         0.5f,-0.5f,-0.5f,   0.0f,-1.0f, 0.0f,   10.0f, 10.0f,
         0.5f,-0.5f, 0.5f,   0.0f,-1.0f, 0.0f,   10.0f, 0.0f,
        -0.5f,-0.5f, 0.5f,   0.0f,-1.0f, 0.0f,   0.0f, 0.0f,
                                                
        -0.5f, 0.5f,-0.5f,   0.0f, 1.0f, 0.0f,   0.0f, 10.0f,
         0.5f, 0.5f,-0.5f,   0.0f, 1.0f, 0.0f,   10.0f, 10.0f,
         0.5f, 0.5f, 0.5f,   0.0f, 1.0f, 0.0f,   10.0f, 0.0f,
        -0.5f, 0.5f, 0.5f,   0.0f, 1.0f, 0.0f,   0.0f, 0.0f
    };

    float InverseCubeVertices[] = {
        -0.5f,-0.5f,-0.5f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,
         0.5f,-0.5f,-0.5f,   0.0f, 0.0f, 1.0f,   1.0f, 0.0f,
         0.5f, 0.5f,-0.5f,   0.0f, 0.0f, 1.0f,   1.0f, 1.0f,
        -0.5f, 0.5f,-0.5f,   0.0f, 0.0f, 1.0f,   0.0f, 1.0f,

        -0.5f,-0.5f, 0.5f,   0.0f, 0.0f,-1.0f,   0.0f, 0.0f,
         0.5f,-0.5f, 0.5f,   0.0f, 0.0f,-1.0f,   1.0f, 0.0f,
         0.5f, 0.5f, 0.5f,   0.0f, 0.0f,-1.0f,   1.0f, 1.0f,
        -0.5f, 0.5f, 0.5f,   0.0f, 0.0f,-1.0f,   0.0f, 1.0f,

        -0.5f, 0.5f, 0.5f,   1.0f, 0.0f, 0.0f,   1.0f, 0.0f,
        -0.5f, 0.5f,-0.5f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f,
        -0.5f,-0.5f,-0.5f,   1.0f, 0.0f, 0.0f,   0.0f, 1.0f,
        -0.5f,-0.5f, 0.5f,   1.0f, 0.0f, 0.0f,   0.0f, 0.0f,

         0.5f, 0.5f, 0.5f,  -1.0f, 0.0f, 0.0f,   1.0f, 0.0f,
         0.5f, 0.5f,-0.5f,  -1.0f, 0.0f, 0.0f,   1.0f, 1.0f,
         0.5f,-0.5f,-0.5f,  -1.0f, 0.0f, 0.0f,   0.0f, 1.0f,
         0.5f,-0.5f, 0.5f,  -1.0f, 0.0f, 0.0f,   0.0f, 0.0f,

        -0.5f,-0.5f,-0.5f,   0.0f, 1.0f, 0.0f,   0.0f, 1.0f,
         0.5f,-0.5f,-0.5f,   0.0f, 1.0f, 0.0f,   1.0f, 1.0f,
         0.5f,-0.5f, 0.5f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f,
        -0.5f,-0.5f, 0.5f,   0.0f, 1.0f, 0.0f,   0.0f, 0.0f,

        -0.5f, 0.5f,-0.5f,   0.0f,-1.0f, 0.0f,   0.0f, 1.0f,
         0.5f, 0.5f,-0.5f,   0.0f,-1.0f, 0.0f,   1.0f, 1.0f,
         0.5f, 0.5f, 0.5f,   0.0f,-1.0f, 0.0f,   1.0f, 0.0f,
        -0.5f, 0.5f, 0.5f,   0.0f,-1.0f, 0.0f,   0.0f, 0.0f
    };

    unsigned int CubeIndices[] = {
        0,  1,  3,
        1,  2,  3,

        4,  7,  5,
        5,  7,  6,

        8, 11,  9,
        9, 11, 10,

       12, 13, 15,
       13, 14, 15,

       16, 19, 17,
       17, 19, 18,

       20, 21, 23,
       21, 22, 23
    };

    float WallVertices[] = {
        -0.5f, 0.5f, 0.0f,   0.0f, 0.0f, -1.0f,   0.0f, 1.0f,   1.0f, 0.0f, 0.0f,   0.0f, 1.0f, 0.0f,
        -0.5f,-0.5f, 0.0f,   0.0f, 0.0f, -1.0f,   0.0f, 0.0f,   1.0f, 0.0f, 0.0f,   0.0f, 1.0f, 0.0f,
         0.5f,-0.5f, 0.0f,   0.0f, 0.0f, -1.0f,   1.0f, 0.0f,   1.0f, 0.0f, 0.0f,   0.0f, 1.0f, 0.0f,

        -0.5f, 0.5f, 0.0f,   0.0f, 0.0f, -1.0f,   0.0f, 1.0f,   1.0f, 0.0f, 0.0f,   0.0f, 1.0f, 0.0f,
         0.5f,-0.5f, 0.0f,   0.0f, 0.0f, -1.0f,   1.0f, 0.0f,   1.0f, 0.0f, 0.0f,   0.0f, 1.0f, 0.0f,
         0.5f, 0.5f, 0.0f,   0.0f, 0.0f, -1.0f,   1.0f, 1.0f,   1.0f, 0.0f, 0.0f,   0.0f, 1.0f, 0.0f
    };


    glBindVertexArray(mVertexArrayObject[GL::VAO::WALL_RECT]);
    glBindBuffer(GL_ARRAY_BUFFER, mVertexBufferObject[GL::VBO::WALL_RECT]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(WallVertices), &WallVertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(8 * sizeof(float)));
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(11 * sizeof(float)));

    glBindVertexArray(mVertexArrayObject[GL::VAO::SOLID_RECT]);

    glBindBuffer(GL_ARRAY_BUFFER, mVertexBufferObject[GL::VBO::SOLID_RECT]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(CubeVertices), CubeVertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mElementBufferObject[GL::EBO::SOLID_RECT]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(CubeIndices), CubeIndices, GL_STATIC_DRAW);


    glBindVertexArray(mVertexArrayObject[GL::VAO::MATERIAL_RECT]);

    glBindBuffer(GL_ARRAY_BUFFER, mVertexBufferObject[GL::VBO::SOLID_RECT]);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mElementBufferObject[GL::EBO::SOLID_RECT]);

    glBindVertexArray(mVertexArrayObject[GL::VAO::TEXTURE_RECT]);

    glBindBuffer(GL_ARRAY_BUFFER, mVertexBufferObject[GL::VBO::TEXTURE_RECT]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(CubeVertices), CubeVertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mElementBufferObject[GL::EBO::SOLID_RECT]);

    glBindVertexArray(mVertexArrayObject[GL::VAO::INVERSE_TEXTURE_RECT]);

    glBindBuffer(GL_ARRAY_BUFFER, mVertexBufferObject[GL::VBO::INVERSE_TEXTURE_RECT]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(InverseCubeVertices), InverseCubeVertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mElementBufferObject[GL::EBO::SOLID_RECT]);


    glBindVertexArray(mVertexArrayObject[GL::VAO::TWO_RECT]);

    glBindBuffer(GL_ARRAY_BUFFER, mVertexBufferObject[GL::VBO::TWO_RECT]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(RectVertices), RectVertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mElementBufferObject[GL::EBO::TWO_RECT]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(RectIndices), RectIndices, GL_STATIC_DRAW);
}

void Graphic::Close()
{
    for (auto program : mPrograms)
    {
        program.second->Close();

        delete program.second;
    }
    mPrograms.clear();

    for (auto tex : mTextures)
    {
        tex.second->Close();

        delete tex.second;
    }
    mTextures.clear();

    for (auto vert : mVertexShader)
    {
        vert.second->Close();

        delete vert.second;
    }
    mVertexShader.clear();

    for (auto frag : mFragmentShader)
    {
        frag.second->Close();

        delete frag.second;
    }
    mFragmentShader.clear();

    for (auto geom : mGeometryShader)
    {
        geom.second->Close();

        delete geom.second;
    }
    mGeometryShader.clear();

    mScreenTexture->Close();
    delete mScreenTexture;

    glDeleteBuffers(GL::VBO::MAX, mVertexBufferObject);
    glDeleteBuffers(GL::EBO::MAX, mElementBufferObject);
    glDeleteBuffers(GL::UBO::MAX, mUniformBufferObject);
    glDeleteFramebuffers(GL::FBO::MAX, mFrameBufferObject);

    glDeleteBuffers(1, &mRenderBufferObject);
    glDeleteTextures(5, mMultisampleTex);
    glDeleteTextures(1, &mNoiseTex);
    glDeleteVertexArrays(GL::VAO::MAX, mVertexArrayObject);

    for (int i = 0; i < 3; ++i)
    {
        mBlurTexture[i]->Close();

        delete mBlurTexture[i];
    }

    for (int i = 0; i < 5; ++i)
    {
        mGBufferTex[i]->Close();

        delete mGBufferTex[i];
    }

    for (int i = 0; i < 2; ++i)
    {
        mSSAOTex[i]->Close();

        delete mSSAOTex[i];
    }
}

void Graphic::Draw()
{
    glEnable(GL_DEPTH_TEST);

    SendUBO();

    glCullFace(GL_FRONT);
    for (auto l : Engine::mCurrentLevel->mLight)
    {
        l->DrawOnTex();
    }
    glCullFace(GL_BACK);

    unsigned int number;

    glViewport(0, 0, Window::Windowsize[0], Window::Windowsize[1]);
    glBindFramebuffer(GL_FRAMEBUFFER, mFrameBufferObject[GL::FBO::MSAA]);
    glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    for (auto d : Engine::mCurrentLevel->mCamera->mDrawlist)
    {
        d.second->Draw();
    }

    glBindFramebuffer(GL_READ_FRAMEBUFFER, mFrameBufferObject[GL::FBO::MSAA]);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, mFrameBufferObject[GL::FBO::GEOM_BUFFER]);

    for (int i = 1; i < 5; ++i)
    {
        glReadBuffer(GL_COLOR_ATTACHMENT0 + i);
        glDrawBuffer(GL_COLOR_ATTACHMENT0 + i);
        glBlitFramebuffer(0, 0, Window::Windowsize[0], Window::Windowsize[1], 0, 0, Window::Windowsize[0], Window::Windowsize[1], GL_COLOR_BUFFER_BIT, GL_NEAREST);
    }


    glBindFramebuffer(GL_FRAMEBUFFER, mFrameBufferObject[GL::FBO::SSAO]);

    number = mPrograms[GL::PROGRAM::SSAO]->mNumber;
    glUseProgram(number);
    glBindVertexArray(mVertexArrayObject[GL::VAO::TWO_RECT]);
    
    for (int i = 0; i < 2; ++i)
    {
        mGBufferTex[i + 1]->Bind(i);
    }
    glActiveTexture(GL_TEXTURE0 + 2);
    glBindTexture(GL_TEXTURE_2D, mNoiseTex);

    unsigned int loc = glGetUniformLocation(number, "posTex");
    glUniform1i(loc, 0);
    loc = glGetUniformLocation(number, "normalTex");
    glUniform1i(loc, 1);
    loc = glGetUniformLocation(number, "noiseTex");
    glUniform1i(loc, 2);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, mFrameBufferObject[GL::FBO::SSAO_BLUR]);

    number = mPrograms[GL::PROGRAM::SSAOBLUR]->mNumber;
    glUseProgram(number);
    glBindVertexArray(mVertexArrayObject[GL::VAO::TWO_RECT]);

    mSSAOTex[0]->Bind(0);

    loc = glGetUniformLocation(number, "aoTex");
    glUniform1i(loc, 0);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, mFrameBufferObject[GL::FBO::POST_PROCESS]);
    glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);

    number = mPrograms[GL::PROGRAM::LIGHT]->mNumber;
    glUseProgram(number);
    glBindVertexArray(mVertexArrayObject[GL::VAO::TWO_RECT]);

    for (int i = 0; i < 4; ++i)
    {
        mGBufferTex[i + 1]->Bind(i);
    }
    mSSAOTex[1]->Bind(4);

    loc = glGetUniformLocation(number, "posTex");
    glUniform1i(loc, 0);
    loc = glGetUniformLocation(number, "normalTex");
    glUniform1i(loc, 1);
    loc = glGetUniformLocation(number, "colTex");
    glUniform1i(loc, 2);
    loc = glGetUniformLocation(number, "specTex");
    glUniform1i(loc, 3);
    loc = glGetUniformLocation(number, "aoTex");
    glUniform1i(loc, 4);


    //cubemap texture occurs error when it did not has value even though it does not use(maybe glsl does not optimized code when it comes to cubemap)
    for (int iter = 0; iter < 5; ++iter)
    {
        std::string AA = "light[" + std::to_string(iter) + "].depthCubeMap";
        loc = glGetUniformLocation(number, AA.c_str());
        glUniform1i(loc, 6);
    }

    int iter = 0;
    for (auto& l : Engine::mCurrentLevel->mLight)
    {
        std::string arrayname = "light[" + std::to_string(iter) + "]";
        std::string addname = arrayname + ".Position";

        loc = glGetUniformLocation(number, addname.c_str());
        glUniform3fv(loc, 1, &l->mPosition.x);

        addname = arrayname + ".Color";
        loc = glGetUniformLocation(number, addname.c_str());
        glUniform3fv(loc, 1, &l->mMaterialSpecularVector.x);

        addname = arrayname + ".Linear";
        loc = glGetUniformLocation(number, addname.c_str());
        glUniform1f(loc, l->mLinear);

        addname = arrayname + ".Quadratic";
        loc = glGetUniformLocation(number, addname.c_str());
        glUniform1f(loc, l->mQuadratic);

        addname = arrayname + ".depthCubeMap";

        loc = glGetUniformLocation(number, addname.c_str());
        glUniform1i(loc, 6 + iter);

        l->mDepthTexture->Bind(iter + 6);
        ++iter;
    }

    loc = glGetUniformLocation(number, "light_num");
    glUniform1i(loc, iter);

    loc = glGetUniformLocation(number, "viewPos");
    glUniform3fv(loc, 1, glm::value_ptr(Engine::mCurrentLevel->mCamera->mPosition));

    {
        auto cam = Engine::mCurrentLevel->mCamera;
        loc = glGetUniformLocation(number, "spotlight");
        glUniform1i(loc, cam->mIsFlashlight);

        loc = glGetUniformLocation(number, "flash.linear");
        glUniform1f(loc, 0.09f);

        loc = glGetUniformLocation(number, "flash.quadratic");
        glUniform1f(loc, 0.032f);

        loc = glGetUniformLocation(number, "viewdir");
        glUniform3fv(loc, 1, glm::value_ptr(cam->mFront));

        loc = glGetUniformLocation(number, "flash.Color");
        glUniform3fv(loc, 1, glm::value_ptr(cam->mSpecularColor));

        loc = glGetUniformLocation(number, "flash.cutOff");
        glUniform1f(loc, glm::cos(glm::radians(20.f)));

        loc = glGetUniformLocation(number, "flash.outercutOff");
        glUniform1f(loc, glm::cos(glm::radians(35.f)));
    }

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    glUseProgram(mPrograms[GL::PROGRAM::TEXTUREBLUR]->mNumber);

    for (int i = 0; i < 2; ++i)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, mFrameBufferObject[GL::FBO::BLUR_HORIZONTAL + i]);
     
        glUseProgram(mPrograms[GL::PROGRAM::TEXTUREBLUR]->mNumber);
        glBindVertexArray(mVertexArrayObject[GL::VAO::TWO_RECT]);

        unsigned int loc = glGetUniformLocation(mPrograms[GL::PROGRAM::TEXTUREBLUR]->mNumber, "horizontal");
        glUniform1i(loc, i);

        loc = glGetUniformLocation(mPrograms[GL::PROGRAM::TEXTUREBLUR]->mNumber, "texID");
        glUniform1i(loc, 0);

        mBlurTexture[i]->Bind(0);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);

    number = mPrograms[GL::PROGRAM::TEXTURE2D]->mNumber;
    glUseProgram(number);
    glBindVertexArray(mVertexArrayObject[GL::VAO::TWO_RECT]);

    mScreenTexture->Bind(0);
    loc = glGetUniformLocation(number, "texID");
    glUniform1i(loc, 0);

    mBlurTexture[2]->Bind(1);
    loc = glGetUniformLocation(number, "blurTex");
    glUniform1i(loc, 1);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    Engine::mCurrentLevel->mCamera->mDrawlist.clear();
}

void Graphic::ResizeEvent(unsigned int w, unsigned int h)
{
    for (auto texture : mWindowSizeTexture)
    {
        texture->ChangeSize(w, h);
    }

    glm::mat4 projection;
    projection = glm::perspective(glm::radians(90.0f), Window::GetAspectRatio(), PROJECTION_NEAR_FACE, PROJECTION_FAR_FACE);

    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(projection));
}

Shader::Shader(SHADER_TYPE type, std::string name, unsigned int id) : mType(type), mName(name), mID(id) {}

std::string Shader::GetPath() const
{
    if (mType == SHADER_TYPE::FRAGMENT_SHADER)
    {
        return "Data/ShaderSource/" + mName + ".frag";
    }
    else if (mType == SHADER_TYPE::GEOMETRY_SHADER)
    {
        return "Data/ShaderSource/" + mName + ".geom";
    }

    return "Data/ShaderSource/" + mName + ".vert";
}

void Shader::SaveFile() const
{
    Utility::SaveDataToFile(GetPath(), mSource);
}

void Shader::ResetSource()
{
    mSource.clear();

    Utility::LoadDataFromFile(GetPath(), mSource);
}

const unsigned int Shader::Compile(std::string& err) const
{
    unsigned int Shaderid;
    if (mType == SHADER_TYPE::FRAGMENT_SHADER)
    {
        Shaderid = glCreateShader(GL_FRAGMENT_SHADER);
    }
    else if(mType == SHADER_TYPE::VERTEX_SHADER)
    {
        Shaderid = glCreateShader(GL_VERTEX_SHADER);
    }
    else
    {
        Shaderid = glCreateShader(GL_GEOMETRY_SHADER);
    }

    const char* tempstr = mSource.c_str();

    glShaderSource(Shaderid, 1, &tempstr, NULL);
    glCompileShader(Shaderid);
    GLint isCompiled = 0;
    glGetShaderiv(Shaderid, GL_COMPILE_STATUS, &isCompiled);
    if (isCompiled == GL_FALSE)
    {
        GLint maxLength = 0;
        glGetShaderiv(Shaderid, GL_INFO_LOG_LENGTH, &maxLength);

        char* errorLog = new char[maxLength];
        glGetShaderInfoLog(Shaderid, maxLength, &maxLength, errorLog);
        err = errorLog;
        delete[] errorLog;

        return -1;
    }

    err.clear();

    return Shaderid;
}

void Shader::Close()
{

}

float Window::GetAspectRatio()
{
    return static_cast<float>(Windowsize[0]) / static_cast<float>(Windowsize[1]);
}

void Camera::Update(int vert, int hor)
{
    mPosition += mSpeed * vert * mFront;
    mPosition += glm::normalize(glm::cross(mFront, mUp)) * (hor * mSpeed);
}

void Camera::Look(float x, float y, bool move)
{
    static float Yaw = -90.0f;
    static float Pitch = 0.0f;

    static float prex = x;
    static float prey = y;

    if (!move)
    {
        prex = x;
        prey = y;

        return;
    }

    Yaw -= (prex - x) * 0.08f;
    Pitch -= (prey - y) * 0.08f;

    prex = x;
    prey = y;

    if (Pitch > 89.0f)
    {
        Pitch = 89.0f;
    }
    if (Pitch < -89.0f)
    {
        Pitch = -89.0f;
    }

    glm::vec3 front;
    front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    front.y = sin(glm::radians(Pitch));
    front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    mFront = glm::normalize(front);
}
