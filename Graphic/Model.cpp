#include "Model.hpp"
#include "Texture.hpp"
#include "Graphic.hpp"
#include "Utility.hpp"

#include <glad/glad.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/pbrmaterial.h>

#include <iostream>

bool Model::Load(std::string path)
{
    Assimp::Importer importer;

    auto extension = path.find(".");
    std::string name = path.substr(0, extension) + "/";

    std::string typestring = path.substr(extension + 1, std::string::npos);

    MODEL_TYPE type;

    if (typestring == "gltf")
    {
        type = MODEL_TYPE::MODEL_GLTF;
    }
    if (typestring == "obj")
    {
        type = MODEL_TYPE::MODEL_OBJ;
    }

    std::string directory_path = "Data/asset/Objects/" + name;
    std::string file_path = directory_path + path;


    const aiScene* scene = importer.ReadFile(file_path,
        aiProcess_CalcTangentSpace | aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_SortByPType);

    if (!scene)
    {
        std::cout << "failed to load object file : " << importer.GetErrorString() << std::endl;

        return false;
    }

    std::vector<aiString> AISTRING;
    mNumMesh = scene->mNumMeshes;

    mVAOs = new unsigned int[mNumMesh];
    mVBOs = new unsigned int[mNumMesh + 1];
    mEBOs = new unsigned int[mNumMesh];

    glGenBuffers(mNumMesh + 1, mVBOs);
    glGenBuffers(mNumMesh, mEBOs);
    glGenVertexArrays(mNumMesh, mVAOs);

    if (mIsInstance)
    {
        static glm::vec3* offset;
        delete[] offset;
        offset = new glm::vec3[mInstanceNum];

        for (int i = 0; i < mInstanceNum; ++i)
        {
            offset[i].x = Utility::GetRand(0.0f, 1.0f);
            offset[i].y = Utility::GetRand(0.0f, 1.0f);
            offset[i].z = Utility::GetRand(0.0f, 1.0f);
            //float temp = i * 360.0f / 100.0;
            //temp = glm::radians(temp);
            //offset[i] = 10.0f * glm::vec3(std::sin(temp), std::sin(temp), std::cos(temp));
        }

        glBindBuffer(GL_ARRAY_BUFFER, mVBOs[mNumMesh]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * mInstanceNum, &offset[0], GL_STATIC_DRAW);
    }

    for (unsigned int k = 0; k < mNumMesh; ++k)
    {
        auto mesh = scene->mMeshes[k];

        std::vector<float> vertices;

        std::vector<unsigned int> index;

        for (unsigned int i = 0; i < mesh->mNumVertices; ++i)
        {
            auto vert = mesh->mVertices[i];
            vertices.push_back(vert.x);
            vertices.push_back(vert.y);
            vertices.push_back(vert.z);

            auto normal = mesh->mNormals[i];
            vertices.push_back(normal.x);
            vertices.push_back(normal.y);
            vertices.push_back(normal.z);

            auto tex = mesh->mTextureCoords[0][i];
            vertices.push_back(tex.x);
            vertices.push_back(1.0 - tex.y);
        }

        for (unsigned int i = 0; i < mesh->mNumFaces; ++i)
        {
            auto face = mesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; ++j)
            {
                index.push_back(face.mIndices[j]);
            }
        }

        auto mat = scene->mMaterials[mesh->mMaterialIndex];

        for (unsigned int i = 0; i < mat->GetTextureCount(aiTextureType_DIFFUSE); ++i)
        {
            aiString str;
            mat->GetTexture(aiTextureType_DIFFUSE, i, &str);
            if (std::find(AISTRING.begin(), AISTRING.end(), str) == AISTRING.end())
            {
                mDiffuse = new Texture();
                mDiffuse->LoadTexture(directory_path + str.C_Str());
                AISTRING.push_back(str);
            }
        }

        aiTextureType specular;

        if (type == MODEL_GLTF)
        {
            specular = aiTextureType_UNKNOWN;
        }
        else if (type == MODEL_OBJ)
        {
            specular = aiTextureType_SPECULAR;
        }

        for (unsigned int i = 0; i < mat->GetTextureCount(specular); ++i)
        {
            aiString str;
            mat->GetTexture(specular, i, &str);
            if (std::find(AISTRING.begin(), AISTRING.end(), str) == AISTRING.end())
            {
                mSpecular = new Texture();
                mSpecular->LoadTexture(directory_path + str.C_Str());
                AISTRING.push_back(str);
            }
        }

        mSizes.push_back(static_cast<unsigned int>(index.size()));

        glBindVertexArray(mVAOs[k]);

        glBindBuffer(GL_ARRAY_BUFFER, mVBOs[k]);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(sizeof(float) * 3));

        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(sizeof(float) * 6));

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mEBOs[k]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, index.size() * sizeof(unsigned int), index.data(), GL_STATIC_DRAW);


        glEnableVertexAttribArray(3);
        glBindBuffer(GL_ARRAY_BUFFER, mVBOs[mNumMesh]);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
        glVertexAttribDivisor(3, 1);

    }

    //mInnertime = 0.0f;

    return true;
}

void Model::Draw()
{
	unsigned int size = static_cast<unsigned int>(mSizes.size());

    mDiffuse->Bind(0);
    mSpecular->Bind(1);

    mInnertime += 0.1f;

    int num = Graphic::mPrograms[18]->mNumber;

    unsigned int loc = glGetUniformLocation(num, "time");
    glUniform1f(loc, mInnertime);

	for (unsigned int vao = 0; vao < size; ++vao)
	{
		glBindVertexArray(mVAOs[vao]);

        if(mIsInstance) glDrawElementsInstanced(GL_TRIANGLES, mSizes[vao], GL_UNSIGNED_INT, 0, mInstanceNum);
		else glDrawElements(GL_TRIANGLES, mSizes[vao], GL_UNSIGNED_INT, 0);
	}
}

void Model::Close()
{
    mDiffuse->Close();
    mSpecular->Close();

    delete mDiffuse;
    delete mSpecular;

    glDeleteBuffers(mNumMesh, mVBOs);
    glDeleteBuffers(mNumMesh, mEBOs);
    glDeleteVertexArrays(mNumMesh, mVBOs);

    delete[] mVBOs;
    delete[] mEBOs;
    delete[] mVAOs;
}
