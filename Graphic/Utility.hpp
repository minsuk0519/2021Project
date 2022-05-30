#include <IMGUI/imgui.h>
#include <IMGUI/imgui_internal.h>
#include <rapidjson/document.h>
#include <rapidjson/filereadstream.h>
#include <rapidjson/filewritestream.h>
#include <rapidjson/prettywriter.h>

#include <string>
#include <fstream>
#include <iostream>
#include <random>
#include <sstream>
#include <chrono>

namespace Utility
{
    //imGUI
    static int MyResizeCallback(ImGuiInputTextCallbackData* data)
    {
        if (data->EventFlag != ImGuiInputTextFlags_CallbackResize)
        {
            return 0;
        }
        std::string* my_str = (std::string*)data->UserData;
        my_str->resize(data->BufSize);
        data->Buf = (my_str->data());
        return 0;
    }
    static bool MyInputTextMultiline(const char* label, std::string* my_str, const ImVec2& size)
    {
        return ImGui::InputTextMultiline(label, my_str->data(), my_str->size(), size, ImGuiInputTextFlags_CallbackResize | ImGuiInputTextFlags_AllowTabInput, Utility::MyResizeCallback, (void*)(my_str));
    }
    static int InputTextCallback(ImGuiInputTextCallbackData* data)
    {
        if (data->EventFlag == ImGuiInputTextFlags_CallbackResize)
        {
            std::string* str = (std::string*)data->UserData;
            IM_ASSERT(data->Buf == str->c_str());
            str->resize(data->BufTextLen);
            data->Buf = (char*)str->c_str();
        }

        return 0;
    }
    static bool MyInputTextString(const char* label, std::string* str, ImGuiInputTextFlags flags = ImGuiInputTextFlags_None)
    {
        flags |= ImGuiInputTextFlags_CallbackResize;
        return ImGui::InputText(label, (char*)str->c_str(), str->capacity() + 1, flags, InputTextCallback, (void*)str);
    }

    //RapidJson
    static rapidjson::Value SetVec3(glm::vec3 v, rapidjson::MemoryPoolAllocator<>& allocator)
    {
        rapidjson::Value arr("", allocator);
        arr.SetArray();
        arr.PushBack(v.x, allocator);
        arr.PushBack(v.y, allocator);
        arr.PushBack(v.z, allocator);
        return arr;
    }
    static glm::vec3 GetVec3(const rapidjson::Value& value, std::string idx)
    {
        glm::vec3 result;

        if (const rapidjson::Value& arr = value[idx]; arr.IsArray())
        {
            if (arr.Size() != 3)
            {
                std::cout << "Error occured : wrong GetVec3() function parameter!" << std::endl;

                return glm::vec3(0.0f);
            }
            result.x = static_cast<float>(arr[0].GetDouble());
            result.y = static_cast<float>(arr[1].GetDouble());
            result.z = static_cast<float>(arr[2].GetDouble());
        }

        return result;
    }


    //file I/O
    static bool LoadDataFromFile(const std::string& Filepath, std::string& data)
    {
        std::ifstream vertexfile(Filepath);

        if (vertexfile.is_open())
        {
            while (!vertexfile.eof())
            {
                std::string temp;
                std::getline(vertexfile, temp, '\0');
                data += temp;
            }
            data += '\0';
            return true;
        }

        return false;
    }

    static bool SaveDataToFile(const std::string& Filepath, const std::string& data)
    {
        std::ofstream shaderfile(Filepath);
        if (shaderfile.is_open())
        {
            shaderfile << data.c_str();
            shaderfile.close();

            return true;
        }

        return false;
    }

    //etc..
    static float GetRand(float start, float end)
    {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<float> dist(start, end);

        return dist(gen);
    }

    template <typename T>
    static std::string to_string_precision(const T value, int precision = 5)
    {
        std::ostringstream out;
        out.precision(precision);
        out << std::fixed << value;
        return out.str();
    }

    static float GetTick()
    {
        static std::chrono::high_resolution_clock::time_point prevtime;
        static std::chrono::high_resolution_clock::time_point lasttime;
        lasttime = std::chrono::high_resolution_clock::now();
        float result = std::chrono::duration<float>(lasttime - prevtime).count();
        prevtime = std::chrono::high_resolution_clock::now();

        return result;
    }
};