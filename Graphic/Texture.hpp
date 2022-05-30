#pragma once

#include <string>

enum TEXTURE_CONFIG_FLAG : int
{
	DEFAULT		= 0b000000,
	DEPTH		= 0b000001,
	CUBEMAP		= 0b000010,
	WINDOWSIZE	= 0b000100,
	FRAMEBUFFER = 0b001000,
	ATTACH		= 0b010000,
	CLAMP		= 0b100000
};

enum TEXTURE_FORMAT
{
	FORMAT_DEPTH,
	FORMAT_RED,
	FORMAT_RGB,
	FORMAT_RGBA,
	FORMAT_FLOAT,
	FORMAT_FLOAT_3
};

constexpr unsigned int DEPTH_TEXTURE_SIZE = 2048;

class Texture
{
private:
	friend class DataManager;
	friend class Interface;
	friend class Graphic;

	int mFlag = TEXTURE_CONFIG_FLAG::DEFAULT;
	//2dtexture & cubemap
	int mTextureType;
	int mDataType;

	std::string mName;
	unsigned int mID = -1;
	int mWidth = -1;
	int mHeight = -1;
	unsigned char* mData = nullptr;

	//type is glenum
	int mFormat;
public:
	Texture();
	Texture(std::string path);

	//do not need to call initialize function
	void LoadTexture(const std::string& filepath);
	void LoadCubeMap(const std::string& filepath);

	//Set type TEXTURE_CONFIG_FLAG
	void Initialize(int flag = TEXTURE_CONFIG_FLAG::DEFAULT, TEXTURE_FORMAT format = FORMAT_RGB, unsigned int COLOR_SLOT = 0);
	void Bind(unsigned int slot = 0);

	void ChangeSize(int w, int h);

	void Close();

	void SaveTexture(const std::string& filepath);
};