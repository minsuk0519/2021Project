#include "DataManager.hpp"
#include "Graphic.hpp"
#include "Texture.hpp"
#include "Utility.hpp"
#include "Engine.hpp"
#include "LevelBase.hpp"
#include "Object.hpp"
#include "Light.hpp"
#include "Wall.hpp"
#include "Model.hpp"

//
#include "Level0.hpp"

#include <rapidjson/document.h>
#include <rapidjson/filereadstream.h>
#include <rapidjson/filewritestream.h>
#include <rapidjson/prettywriter.h>

#include <string>

void DataManager::LoadGraphic()
{
	rapidjson::Document document;
	FILE* fp;
	fopen_s(&fp, "Data/graphic.json", "rb");
	if (fp == nullptr)
	{
		std::cout << "Error occured : Cannot find the file graphic.json!" << std::endl;

		return;
	}

	char readBuffer[4096];
	rapidjson::FileReadStream is(fp, readBuffer, sizeof(readBuffer));

	document.ParseStream(is);

	if (document.IsObject())
	{
		if (const rapidjson::Value& arr = document["Texture"]; arr.IsArray())
		{
			for (rapidjson::SizeType i = 0; i < arr.Size(); ++i)
			{
				unsigned int id = arr[i]["ID"].GetUint();
				std::string name = arr[i]["Name"].GetString();
				Texture* newTexture = new Texture(name);
				Graphic::mTextures[id] = newTexture;
			}
		}
		if (const rapidjson::Value& arr = document["Shader"]; arr.IsArray())
		{
			for (rapidjson::SizeType i = 0; i < arr.Size(); ++i)
			{
				unsigned int id = arr[i]["ID"].GetUint();
				std::string name = arr[i]["Name"].GetString();
				unsigned int type = arr[i]["Type"].GetUint();
				switch (type)
				{
				case VERTEX_SHADER:
					Graphic::mVertexShader[id] = new Shader((SHADER_TYPE)(type), name, id);
					break;
				case FRAGMENT_SHADER:
					Graphic::mFragmentShader[id] = new Shader((SHADER_TYPE)type, name, id);
					break;
				case GEOMETRY_SHADER:
					Graphic::mGeometryShader[id] = new Shader((SHADER_TYPE)type, name, id);
					break;
				default:
					std::cout << "Error occured : shader type is wrong!" << std::endl;
					return;
				}
			}
		}
		if (const rapidjson::Value& arr = document["Program"]; arr.IsArray())
		{
			for (rapidjson::SizeType i = 0; i < arr.Size(); ++i)
			{
				std::string name = arr[i]["Name"].GetString();
				Program* program = new Program(name);
				program->mID = arr[i]["ID"].GetUint();
				unsigned int vertex = arr[i]["VertexID"].GetUint();
				unsigned int fragment = arr[i]["FragmentID"].GetUint();
				if (arr[i].HasMember("GeometryID"))
				{
					unsigned int geometry = arr[i]["GeometryID"].GetUint();
					program->Initialize(vertex, fragment, geometry);
				}
				else
				{
					program->Initialize(vertex, fragment);
				}
				Graphic::mPrograms[program->mID] = program;
			}
		}
	}

	fclose(fp);
}

void DataManager::SaveGraphic()
{
	rapidjson::Document document;
	rapidjson::MemoryPoolAllocator allocator = document.GetAllocator();

	document.StartObject();
	document.SetObject();

	rapidjson::Value TextureArr("", allocator);
	TextureArr.SetArray();
	for (auto V : Graphic::mTextures)
	{
		rapidjson::Value tempVal;
		tempVal.SetObject();
		tempVal.AddMember("ID", V.first, allocator);
		tempVal.AddMember("Name", V.second->mName, allocator);
		TextureArr.PushBack(tempVal, allocator);
	}
	document.AddMember("Texture", TextureArr, allocator);

	rapidjson::Value ShaderArr("", allocator);
	ShaderArr.SetArray();
	for (auto V : Graphic::mVertexShader)
	{
		rapidjson::Value tempVal;
		tempVal.SetObject();
		tempVal.AddMember("ID", V.first, allocator);
		tempVal.AddMember("Name", V.second->mName, allocator);
		tempVal.AddMember("Type", V.second->mType, allocator);
		ShaderArr.PushBack(tempVal, allocator);
	}
	for (auto V : Graphic::mGeometryShader)
	{
		rapidjson::Value tempVal;
		tempVal.SetObject();
		tempVal.AddMember("ID", V.first, allocator);
		tempVal.AddMember("Name", V.second->mName, allocator);
		tempVal.AddMember("Type", V.second->mType, allocator);
		ShaderArr.PushBack(tempVal, allocator);
	}
	for (auto V : Graphic::mFragmentShader)
	{
		rapidjson::Value tempVal;
		tempVal.SetObject();
		tempVal.AddMember("ID", V.first, allocator);
		tempVal.AddMember("Name", V.second->mName, allocator);
		tempVal.AddMember("Type", V.second->mType, allocator);
		ShaderArr.PushBack(tempVal, allocator);
	}
	document.AddMember("Shader", ShaderArr, allocator);


	rapidjson::Value ProgramArr("", allocator);
	ProgramArr.SetArray();
	for (auto p : Graphic::mPrograms)
	{
		rapidjson::Value tempVal;
		tempVal.SetObject();
		tempVal.AddMember("Name", p.second->mName, allocator);
		tempVal.AddMember("ID", p.second->mID, allocator);
		tempVal.AddMember("VertexID", p.second->mVertexID, allocator);
		tempVal.AddMember("FragmentID", p.second->mFragmentID, allocator);
		if (unsigned int geom = p.second->mGeometryID; geom != -1)
		{
			tempVal.AddMember("GeometryID", geom, allocator);
		}
		ProgramArr.PushBack(tempVal, allocator);
	}
	document.AddMember("Program", ProgramArr, allocator);

	document.EndObject(0);


	FILE* fp = nullptr;
	fopen_s(&fp, "Data/graphic.json", "wb");
	if (fp == nullptr) return;

	char writeBuffer[4096];
	rapidjson::FileWriteStream os(fp, writeBuffer, sizeof(writeBuffer));

	rapidjson::PrettyWriter<rapidjson::FileWriteStream> writer(os);

	document.Accept(writer);

	fclose(fp);
}

void DataManager::LoadLevel()
{
	rapidjson::Document document;
	FILE* fp;
	fopen_s(&fp, "Data/level.json", "rb");
	if (fp == nullptr)
	{
		std::cout << "Error occured : Cannot find the file graphic.json!" << std::endl;

		return;
	}

	char readBuffer[4096];
	rapidjson::FileReadStream is(fp, readBuffer, sizeof(readBuffer));

	document.ParseStream(is);

	if (document.IsArray())
	{
		for (rapidjson::SizeType i = 0; i < document.Size(); ++i)
		{
			auto& level = document[i];
			if (const rapidjson::Value& info = level["Info"]; info.IsObject())
			{
				unsigned int id = info["ID"].GetUint();
				std::string name = info["Name"].GetString();
				LevelBase* newlevel = new Level0();
				newlevel->mName = name;
				newlevel->mID = id;

				if (Engine::mCurrentLevel == nullptr)
				{
					Engine::mCurrentLevel = newlevel;
					SetLevel(newlevel);
				}
				Engine::mLevels.push_back(newlevel);
				
			}
		}
	}

	fclose(fp);
}

void DataManager::SaveLevel()
{
	return;

	rapidjson::Document document;
	rapidjson::MemoryPoolAllocator allocator = document.GetAllocator();

	document.StartObject();
	document.SetObject();
	document.SetArray();

	for (auto level : Engine::mLevels)
	{
		rapidjson::Value levelVal;
		levelVal.SetObject();

		{
			rapidjson::Value tempVal;
			tempVal.SetObject();
			tempVal.AddMember("ID", 0, allocator);
			tempVal.AddMember("Name", "Level", allocator);
			levelVal.AddMember("Info", tempVal, allocator);
		}

		{
			rapidjson::Value tempVal;
			tempVal.SetObject();
			Camera* cam = level->mCamera;

			rapidjson::Value front = Utility::SetVec3(cam->mFront, allocator);
			tempVal.AddMember("Front", front, allocator);
			rapidjson::Value up = Utility::SetVec3(cam->mUp, allocator);
			tempVal.AddMember("Up", up, allocator);
			rapidjson::Value pos = Utility::SetVec3(cam->mPosition, allocator);
			tempVal.AddMember("Position", pos, allocator);
			rapidjson::Value ambient = Utility::SetVec3(cam->mAmbientColor, allocator);
			tempVal.AddMember("Ambient", ambient, allocator);
			rapidjson::Value diffuse = Utility::SetVec3(cam->mDiffuseColor, allocator);
			tempVal.AddMember("Diffuse", diffuse, allocator);
			rapidjson::Value specular = Utility::SetVec3(cam->mSpecularColor, allocator);
			tempVal.AddMember("Specular", specular, allocator);

			levelVal.AddMember("Camera", tempVal, allocator);
		}

		rapidjson::Value ObjectArr("", allocator);
		ObjectArr.SetArray();
		for (auto obj : level->mObjects)
		{
			rapidjson::Value tempVal;
			tempVal.SetObject();
			tempVal.AddMember("Name", obj->mName, allocator);
			tempVal.AddMember("Position", Utility::SetVec3(obj->mPosition, allocator), allocator);
			tempVal.AddMember("Rotation", Utility::SetVec3(obj->mRot, allocator), allocator);
			tempVal.AddMember("Scale", Utility::SetVec3(obj->mScale, allocator), allocator);
			tempVal.AddMember("Shiness", obj->mShinessFactor, allocator);
			tempVal.AddMember("Ambient", Utility::SetVec3(obj->mMaterialAmbientVector, allocator), allocator);
			tempVal.AddMember("Diffuse", Utility::SetVec3(obj->mMaterialDiffuseVector, allocator), allocator);
			tempVal.AddMember("Specular", Utility::SetVec3(obj->mMaterialSpecularVector, allocator), allocator);
			tempVal.AddMember("FaceSize", obj->mFaceSize, allocator);
			tempVal.AddMember("ProgramID", obj->mProgramID, allocator);
			tempVal.AddMember("VAOID", obj->mVAO, allocator);
			ObjectArr.PushBack(tempVal, allocator);
		}
		levelVal.AddMember("Objects", ObjectArr, allocator);

		document.PushBack(levelVal, allocator);
	}
	document.EndObject(0);


	FILE* fp = nullptr;
	fopen_s(&fp, "Data/level.json", "wb");
	if (fp == nullptr) return;

	char writeBuffer[4096];
	rapidjson::FileWriteStream os(fp, writeBuffer, sizeof(writeBuffer));

	rapidjson::PrettyWriter<rapidjson::FileWriteStream> writer(os);
	writer.SetFormatOptions(rapidjson::PrettyFormatOptions::kFormatSingleLineArray);
	writer.SetMaxDecimalPlaces(5);

	document.Accept(writer);

	fclose(fp);
}

void DataManager::SetLevel(LevelBase* levelptr)
{
	rapidjson::Document document;
	FILE* fp;
	std::string path = "Data/Level/" + levelptr->mName + ".json";
	fopen_s(&fp, path.c_str(), "rb");
	if (fp == nullptr)
	{
		std::cout << "Error occured : Cannot find the file " << path << "!" << std::endl;

		return;
	}

	char readBuffer[4096];
	rapidjson::FileReadStream is(fp, readBuffer, sizeof(readBuffer));

	document.ParseStream(is);

	if (document.IsArray())
	{
		for (rapidjson::SizeType i = 0; i < document.Size(); ++i)
		{
			auto& level = document[i];
			if (const rapidjson::Value& camVal = level["Camera"]; camVal.IsObject())
			{
				Camera* cam = new Camera();
				cam->mFront = Utility::GetVec3(camVal, "Front");
				cam->mUp = Utility::GetVec3(camVal, "Up");
				cam->mPosition = Utility::GetVec3(camVal, "Position");
				cam->mAmbientColor = Utility::GetVec3(camVal, "Ambient");
				cam->mDiffuseColor = Utility::GetVec3(camVal, "Diffuse");
				cam->mSpecularColor = Utility::GetVec3(camVal, "Specular");

				levelptr->mCamera = cam;
			}
			if (const rapidjson::Value& arr = level["Objects"]; arr.IsArray())
			{
				for (rapidjson::SizeType i = 0; i < arr.Size(); ++i)
				{
					auto& obj = arr[i];
					Object* newobject;
					if (obj.HasMember("Type"))
					{
						int lighttype = obj["Type"].GetInt();
						switch (lighttype)
						{
						case OBJ_TYPE::LIGHT_DIR:
							newobject = new DirLight();
							levelptr->mSun = dynamic_cast<DirLight*>(newobject);

							break;
						case OBJ_TYPE::LIGHT_POINT:
							newobject = new PointLight();
							levelptr->mLight.push_back(dynamic_cast<Light*>(newobject));

							if (obj.HasMember("Attenuation") && obj["Attenuation"].IsArray())
							{
								float linear = static_cast<float>(obj["Attenuation"][0].GetDouble());
								float quadratic = static_cast<float>(obj["Attenuation"][1].GetDouble());

								dynamic_cast<Light*>(newobject)->mLinear = linear;
								dynamic_cast<Light*>(newobject)->mQuadratic = quadratic;
							}

							break;
						case OBJ_TYPE::WALL:
							newobject = new Wall();
							break;
						default:
							std::cout << "Error occured : Object Type is wrong!" << std::endl;

							return;
						}
					}
					else
					{
						newobject = new Object();
					}

					newobject->mName = obj["Name"].GetString();
					newobject->mPosition = Utility::GetVec3(obj, "Position");
					newobject->mRot = Utility::GetVec3(obj, "Rotation");
					newobject->mScale = Utility::GetVec3(obj, "Scale");
					newobject->mShinessFactor = static_cast<float>(obj["Shiness"].GetDouble());
					newobject->mMaterialAmbientVector = Utility::GetVec3(obj, "Ambient");
					newobject->mMaterialDiffuseVector = Utility::GetVec3(obj, "Diffuse");
					newobject->mMaterialSpecularVector = Utility::GetVec3(obj, "Specular");
					newobject->mFaceSize = obj["FaceSize"].GetUint();
					newobject->mProgramID = obj["ProgramID"].GetUint();
					newobject->mVAO = obj["VAOID"].GetUint();

					if (obj.HasMember("Texture") && obj["Texture"].IsArray())
					{
						const rapidjson::Value& texarr = obj["Texture"];
						for (rapidjson::SizeType i = 0; i < texarr.Size(); ++i)
						{
							newobject->mTextures.push_back(texarr[i].GetUint());
						}
					}

					if (obj.HasMember("Model"))
					{
						std::string str = obj["Model"].GetString();
						newobject->mModel = new Model();
						if (obj.HasMember("Instance"))
						{
							newobject->mModel->mIsInstance = true;
							newobject->mModel->mInstanceNum = obj["Instance"].GetInt();
						}
						newobject->mModel->Load(str);
					}

					newobject->Initialize();

					levelptr->AddObject(newobject);
				}
			}

			levelptr->Init();
		}
	}

	fclose(fp);
}

void DataManager::LoadBaseData()
{
	LoadGraphic();
	LoadLevel();
}

void DataManager::SaveBaseData()
{
	SaveGraphic();
	SaveLevel();
}
