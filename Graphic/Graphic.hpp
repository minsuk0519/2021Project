#pragma once

#include <glad/GLad.h>
#include <glm/glm.hpp>

#include <string>
#include <vector>
#include <unordered_map>
#include <map>

class Object;
class Texture;

namespace GL
{
	namespace VAO
	{
		enum
		{
			START = __LINE__,
			SOLID_RECT = 0,
			TEXTURE_RECT = 1,
			MATERIAL_RECT = 2,
			MY_VAO = 3,
			TWO_RECT = 4,
			WALL_RECT = 5,
			INVERSE_TEXTURE_RECT = 6,
			MAX = __LINE__ - START - 1
		};
	}
	namespace VBO
	{
		enum 
		{
			START = __LINE__,
			SOLID_RECT = 0,
			TEXTURE_RECT = 1,
			MY_VBO = 2,
			TWO_RECT = 3,
			WALL_RECT = 4,
			INVERSE_TEXTURE_RECT = 5,
			MAX = __LINE__ - START - 1
		};
	}
	namespace EBO
	{
		enum
		{
			START = __LINE__,
			SOLID_RECT = 0,
			MY_EBO = 1,
			TWO_RECT = 2,
			MAX = __LINE__ - START - 1
		};
	}
	namespace UBO
	{
		enum
		{
			START = __LINE__,
			VIEW_PROJECTION = 0,
			MAX = __LINE__ - START - 1
		};
	}
	namespace FBO
	{
		enum
		{
			START = __LINE__,
			POST_PROCESS = 0,
			MSAA = 1,
			BLUR_HORIZONTAL = 2,
			BLUR_VERTICAL = 3,
			GEOM_BUFFER = 4,
			SSAO = 5,
			SSAO_BLUR = 6,
			MAX = __LINE__ - START - 1
		};
	}
	namespace PROGRAM
	{
		enum
		{
			START = __LINE__,
			STANDARD3D			= 0,
			TEXTURE				= 1,
			MATERIAL			= 2,
			TEXTURE2D			= 3,
			CUBEMAP				= 4,
			CUBEMAPREFLECT		= 5,
			INSTANCE			= 6,
			SHADOW				= 7,
			SHADOWMAP			= 8,
			DEPTHCUBE			= 9,
			TEXTURESHADOW		= 10,
			WALL				= 11,
			TEXTUREBLUR			= 12,
			LIGHT				= 13,
			SSAO				= 14,
			SSAOBLUR			= 15,
			MAX = __LINE__ - START - 1
		};
	}
}

constexpr float PROJECTION_NEAR_FACE = 0.1f;
constexpr float PROJECTION_FAR_FACE = 100.f;

struct Camera
{
public:
	glm::vec3 mPosition = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 mFront = glm::vec3(0.0f, 0.0f, 1.0f);
	glm::vec3 mUp = glm::vec3(0.0f, 1.0f, 0.0f);
	float mSpeed = 0.05f;
	void Update(int vert, int hor);
	void Look(float x, float y, bool move = true);

	bool mIsFlashlight = false;

	glm::vec3 mAmbientColor =	glm::vec3(0.2f, 0.2f, 0.2f);
	glm::vec3 mDiffuseColor =	glm::vec3(0.5f, 0.5f, 0.5f);
	glm::vec3 mSpecularColor =	glm::vec3(1.0f, 1.0f, 1.0f);

	std::multimap<float, Object*, std::greater<float>> mDrawlist;
};


class Program
{
private:
	friend class DataManager;
	friend class Graphic;
	friend class Interface;

	//engine id
	unsigned int mID = -1;
	std::string mName = "Program";
	unsigned int mVertexID = -1;
	unsigned int mFragmentID = -1;
	unsigned int mGeometryID = -1;

	void LinkProgram(unsigned int Vertex, unsigned int Fragment, unsigned int Geometry = -1);
	int CompileShader(const char* shadersource, GLint shadertype);
public:
	//opengl id
	unsigned int mNumber = -1;
	Program(std::string name = "");
	void Initialize(unsigned int vertex, unsigned int fragment, unsigned int geometry = -1);

	//put compiled glid
	void Reset(unsigned int vert, unsigned int frag);

	void Close();
};

enum SHADER_TYPE
{
	VERTEX_SHADER = 0,
	FRAGMENT_SHADER = 1,
	GEOMETRY_SHADER = 2
};

class Shader
{
private:
	friend class Interface;
	friend class DataManager;

	std::string mName = "Shader";

	//custom id(not opengl id)
	unsigned int mID;
	std::string mSource;
	SHADER_TYPE mType;
public:
	Shader(SHADER_TYPE type, std::string name, unsigned int id);
	std::string GetPath() const;
	void SaveFile() const;
	void ResetSource();
	const unsigned int Compile(std::string& err) const;

	void Close();
};

class Graphic
{
private:
	static void SendUBO();

	static unsigned int mMultisampleTex[5];
	static Texture* mBlurTexture[3];
	static Texture* mScreenTexture;
	static Texture* mGBufferTex[5];
	static Texture* mSSAOTex[2];
	static unsigned int mNoiseTex;
public:
	static std::unordered_map<unsigned int, Program*> mPrograms;
	static std::unordered_map<unsigned int, Shader*> mVertexShader;
	static std::unordered_map<unsigned int, Shader*> mFragmentShader;
	static std::unordered_map<unsigned int, Shader*> mGeometryShader;
	static std::unordered_map<unsigned int, Texture*> mTextures;

	static void Initialize();
	static void Close();

	static void Draw();

	static unsigned int mVertexBufferObject[GL::VBO::MAX];
	static unsigned int mVertexArrayObject[GL::VAO::MAX];
	static unsigned int mElementBufferObject[GL::EBO::MAX];
	static unsigned int mUniformBufferObject[GL::UBO::MAX];
	static unsigned int mFrameBufferObject[GL::FBO::MAX];
	static unsigned int mRenderBufferObject;

	static void ResizeEvent(unsigned int w, unsigned int h);

	static std::vector<Texture*> mWindowSizeTexture;
};