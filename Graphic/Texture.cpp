#include "Texture.hpp"
#include "Graphic.hpp"
#include "Engine.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>
#include <glad/glad.h>

#include <iostream>

Texture::Texture() {}

Texture::Texture(std::string filename) : mName(filename)
{
	std::string path = "Data/asset/Texture/" + filename;

	LoadTexture(path);
}

void Texture::LoadTexture(const std::string& path)
{
	int nrChannels;


	mData = stbi_load(path.c_str(), &mWidth, &mHeight, &nrChannels, 0);
	if (mData)
	{
		if (nrChannels == 1) mFormat = GL_RED;
		else if (nrChannels == 3)
		{
			mFormat = GL_RGB;
			this->Initialize(TEXTURE_CONFIG_FLAG::DEFAULT, TEXTURE_FORMAT::FORMAT_RGB);
		}
		else
		{
			mFormat = GL_RGBA;
			this->Initialize(TEXTURE_CONFIG_FLAG::DEFAULT, TEXTURE_FORMAT::FORMAT_RGBA);
		}


		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "eu-ak" << std::endl;
	}

	stbi_image_free(mData);

	//this->SaveTexture(filename);
}

void Texture::LoadCubeMap(const std::string& filepath)
{
	glGenTextures(1, &mID);

	mTextureType = GL_TEXTURE_CUBE_MAP;


	glBindTexture(GL_TEXTURE_CUBE_MAP, mID);
	int nrChannels;

	for (int i = 0; i < 6; ++i)
	{

		std::string path = "Data/asset/CubeMap/" + std::to_string(i + 1) + filepath;

		mData = stbi_load(path.c_str(), &mWidth, &mHeight, &nrChannels, 0);

		if (mData)
		{
			int loc = GL_TEXTURE_CUBE_MAP_POSITIVE_X + i;

			glTexImage2D(loc, 0, GL_RGB, mWidth, mHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, mData);

		}
		else
		{
			std::cout << "failed to load cubemap file!" << std::endl;
		}

		stbi_image_free(mData);
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	mFlag = TEXTURE_CONFIG_FLAG::CUBEMAP;
	mFormat = TEXTURE_FORMAT::FORMAT_RGB;
}

void Texture::Initialize(int flag, TEXTURE_FORMAT format, unsigned int COLOR_SLOT)
{
	mFlag = flag;
	mFormat = format;

	if (format == TEXTURE_FORMAT::FORMAT_RGB)
	{
		mFormat = GL_RGB;
	}
	else if (format == TEXTURE_FORMAT::FORMAT_RGBA)
	{
		mFormat = GL_RGBA;
	}
	else if (format == TEXTURE_FORMAT::FORMAT_RED)
	{
		mFormat = GL_RED;
	}

	glGenTextures(1, &mID);

	if (mFlag & TEXTURE_CONFIG_FLAG::CUBEMAP) mTextureType = GL_TEXTURE_CUBE_MAP;
	else mTextureType = GL_TEXTURE_2D;
	glBindTexture(mTextureType, mID);

	if (mFlag & TEXTURE_CONFIG_FLAG::WINDOWSIZE)
	{
		mWidth = Window::Windowsize[0];
		mHeight = Window::Windowsize[1];

		Graphic::mWindowSizeTexture.push_back(this);
	}

	if (mFlag & TEXTURE_CONFIG_FLAG::DEPTH)
	{
		mFormat = GL_DEPTH_COMPONENT;

		mDataType = GL_FLOAT;
		mWidth = DEPTH_TEXTURE_SIZE;
		mHeight = DEPTH_TEXTURE_SIZE;
	}
	else
	{
		mDataType = GL_UNSIGNED_BYTE;
	}

	if ((mFlag & TEXTURE_CONFIG_FLAG::CUBEMAP) && (mFlag & TEXTURE_CONFIG_FLAG::DEPTH))
	{
		for (unsigned int i = 0; i < 6; ++i)
		{

			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, mFormat, mWidth, mHeight, 0, mFormat, mDataType, mData);
		}

		glTexParameteri(mTextureType, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(mTextureType, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(mTextureType, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(mTextureType, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(mTextureType, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

		glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, mID, 0);
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);

		return;
	}

	if (format == TEXTURE_FORMAT::FORMAT_FLOAT)
	{
		mFormat = GL_RGBA32F;
		mDataType = GL_FLOAT;
		glTexImage2D(mTextureType, 0, mFormat, mWidth, mHeight, 0, GL_RGBA, mDataType, mData);
	}
	else if (format == TEXTURE_FORMAT::FORMAT_FLOAT_3)
	{
		mFormat = GL_RGB32F;
		mDataType = GL_FLOAT;
		glTexImage2D(mTextureType, 0, mFormat, mWidth, mHeight, 0, GL_RGB, mDataType, mData);
	}
	else
		glTexImage2D(mTextureType, 0, mFormat, mWidth, mHeight, 0, mFormat, mDataType, mData);

	if (mFlag & TEXTURE_CONFIG_FLAG::DEPTH)
	{
		glTexParameteri(mTextureType, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(mTextureType, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(mTextureType, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(mTextureType, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

		float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };

		glTexParameterfv(mTextureType, GL_TEXTURE_BORDER_COLOR, borderColor);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, mID, 0);

		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
	}
	else if (mFlag & TEXTURE_CONFIG_FLAG::FRAMEBUFFER)
	{
		glTexParameteri(mTextureType, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(mTextureType, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + COLOR_SLOT, mTextureType, mID, 0);

		if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			std::cout << "Error occured : Cannot attach texture on framebuffer!" << std::endl;
		}
	}
	else
	{
		glTexParameteri(mTextureType, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(mTextureType, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(mTextureType, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(mTextureType, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}

	if (mFlag & TEXTURE_CONFIG_FLAG::CLAMP)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}
}

void Texture::Bind(unsigned int slot)
{
	glActiveTexture(GL_TEXTURE0 + slot);

	glBindTexture(mTextureType, mID);
}

void Texture::ChangeSize(int w, int h)
{
	glBindTexture(mTextureType, mID);

	mWidth = w;
	mHeight = h;

	glTexImage2D(mTextureType, 0, mFormat, w, h, 0, mFormat, mDataType, nullptr);
}

void Texture::Close()
{
	glDeleteTextures(1, &mID);
}

void Texture::SaveTexture(const std::string& filepath)
{
	Bind();
	
	unsigned char* last_row;

	if (mTextureType == GL_TEXTURE_CUBE_MAP)
	{
		for (int f = 0; f < 6; ++f)
		{
			Bind();

			unsigned char* tmp_data = new unsigned char[mWidth * mHeight * 3];

			float* data = new float[mWidth * mHeight * 6](0);

			glGetTexImage(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, mFormat, mDataType, data);

			int j = 0;
			for (int i = mWidth * mHeight - 1; i != 0; --i)
			{
				tmp_data[j * 3 + 0] = static_cast<unsigned char>(data[i] * 255.0f);
				tmp_data[j * 3 + 1] = static_cast<unsigned char>(data[i] * 255.0f);
				tmp_data[j * 3 + 2] = static_cast<unsigned char>(data[i] * 255.0f);

				++j;
			}

			delete[] data;
			last_row = tmp_data + 3 * (mWidth * (mHeight - 1));

			std::string newpath =  std::to_string(f) + filepath;

			stbi_write_png(newpath.c_str(), mWidth, mHeight, 3, last_row, -3 * mWidth);

			delete[] tmp_data;
		}
		return;
	}

	unsigned char* char_data = new unsigned char[mWidth * mHeight * 3];

	if (mFormat == GL_RGBA32F)
	{
		float* data = new float[mWidth * mHeight * 4](0);

		glGetTexImage(mTextureType, 0, GL_RGBA, mDataType, data);

		int j = 0;
		for (int i = 0; i < mWidth * mHeight * 4; i += 4)
		{
			char_data[j * 3 + 0] = static_cast<unsigned char>(data[i] * 255.0f);
			char_data[j * 3 + 1] = static_cast<unsigned char>(data[i + 1] * 255.0f);
			char_data[j * 3 + 2] = static_cast<unsigned char>(data[i + 2] * 255.0f);

			++j;
		}

		delete[] data;
		last_row = char_data + 3 * (mWidth * (mHeight - 1));

		stbi_write_png(filepath.c_str(), mWidth, mHeight, 3, last_row, -3 * mWidth);

		delete[] char_data;

		return;
	}

	if (mDataType == GL_FLOAT)
	{
		float* data = new float[mWidth * mHeight](0);
		glGetTexImage(mTextureType, 0, mFormat, mDataType, data);

		int j = 0;
		for (int i = mWidth * mHeight - 1; i != 0; --i)
		{
			char_data[j * 3 + 0] = static_cast<unsigned char>(data[i] * 255.0f);
			char_data[j * 3 + 1] = static_cast<unsigned char>(data[i] * 255.0f);
			char_data[j * 3 + 2] = static_cast<unsigned char>(data[i] * 255.0f);

			++j;
		}

		delete[] data;
		last_row = char_data + 3 * (mWidth * (mHeight - 1));
	}
	else
	{
		glGetTexImage(mTextureType, 0, mFormat, mDataType, char_data);

		last_row = char_data +  3 * (mWidth * (mHeight - 1));
	}


	stbi_write_png(filepath.c_str(), mWidth, mHeight, 3, last_row, -3 * mWidth);

	delete[] char_data;
}