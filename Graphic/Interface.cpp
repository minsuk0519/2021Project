#include "Interface.hpp"
#include "Graphic.hpp"
#include "Utility.hpp"
#include "Object.hpp"
#include "Texture.hpp"
#include "LevelBase.hpp"
#include "Engine.hpp"
#include "Wall.hpp"
#include "Light.hpp"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <fstream>

Object* Interface::mDetail = nullptr;

float Interface::AMBIENT = 1.0f;
float Interface::DIFFUSE = 1.0f;

void Interface::Initialize(GLFWwindow* window)
{
    const char* glsl_version = "#version 130";

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);
}

void Interface::Update()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    if (!Engine::mOpenGUI)
    {
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        return;
    }

    ImGui::ShowDemoWindow();

    {
        ImGui::Begin("Program Setting");

        if (ImGui::CollapsingHeader("Window Setting"))
        {
            ImGui::DragInt2("Resolution", Window::Windowsize);
            if (ImGui::Button("Apply"))
            {
                glfwSetWindowSize(Window::Windowptr, Window::Windowsize[0], Window::Windowsize[1]);
                glfwGetFramebufferSize(Window::Windowptr, Window::Windowsize, (Window::Windowsize + 1));
            }
        }
        
        if (ImGui::CollapsingHeader("Shader"))
        {
            ShaderSetting();
        }

        if (ImGui::CollapsingHeader("Object"))
        {
            static unsigned int current_id = 1;

            if (ImGui::BeginListBox("ObjectList"))
            {
                auto objlist = Engine::mCurrentLevel->mObjects;
                for (auto obj : objlist)
                {
                    const bool is_selected = (current_id == obj->mID);
                    std::string uniqueName = obj->mName + "##" + std::to_string(obj->mID);
                    if (ImGui::Selectable(uniqueName.c_str(), is_selected))
                    {
                        current_id = obj->mID;
                    }
                    if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
                    {
                        mDetail = obj;
                    }
                    if (is_selected)
                    {
                        ImGui::SetItemDefaultFocus();
                    }
                }
                ImGui::EndListBox();
            }
        }

        if (ImGui::CollapsingHeader("Texture"))
        {
            static unsigned int current_id = 0;

            if (ImGui::BeginListBox("Texture"))
            {
                for (auto tex : Graphic::mTextures)
                {
                    const bool is_selected = (current_id == tex.first);
                    if (ImGui::Selectable(tex.second->mName.c_str()))
                    {
                        current_id = tex.first;
                    }
                    if (is_selected)
                    {
                        ImGui::SetItemDefaultFocus();
                    }
                }
                ImGui::EndListBox();
            }

            ImGui::Image(reinterpret_cast<void*>(Graphic::mTextures[current_id]->mID), ImVec2(100.0f, 100.0f));
        }

        ImGui::End();
    }

    {
        ImGui::Begin("Detail");

        ImGui::DragFloat("Ambient", &AMBIENT, 0.1f, 0.0f, 10.0f);
        ImGui::DragFloat("Diffuse", &DIFFUSE, 0.1f, 0.0f, 10.0f);

        {
            int type = -1;
            if (ImGui::Button("Create Object"))
            {
                ImGui::OpenPopup("New Object");
            }
            if (ImGui::BeginPopupModal("New Object", NULL, ImGuiWindowFlags_AlwaysAutoResize))
            {
                if (ImGui::Button("Object"))
                {
                    type = 0;
                    ImGui::CloseCurrentPopup();
                }
                if (ImGui::Button("Wall"))
                {
                    type = 1;
                    ImGui::CloseCurrentPopup();
                }

                ImGui::EndPopup();
            }
            Object* newobject = nullptr;
            if (type != -1)
            {
                if (type == 0)
                {
                    newobject = new Object();
                    newobject->mProgramID = 17;
                }
                if (type == 1)
                {
                    newobject = new Wall();
                    newobject->mProgramID = 11;
                }

                newobject->mShinessFactor = 0.25;
                newobject->mMaterialAmbientVector = glm::vec3(1, 1, 1);
                newobject->mMaterialDiffuseVector = glm::vec3(1, 1, 1);
                newobject->mMaterialSpecularVector = glm::vec3(1, 1, 1);

                newobject->mVAO = 1;
                newobject->mFaceSize = 36;

                Engine::mCurrentLevel->AddObject(newobject);

                mDetail = newobject;
            }
        }

        if (mDetail)
        {
            ObjectsSetting(mDetail);
        }

        ImGui::End();
    }


    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Interface::Close()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();

    ImGui::DestroyContext();

}

void Interface::ShaderSetting()
{
    static unsigned int selectedprogram = 0;
    bool changed = false;

    if (ImGui::BeginCombo("Program", Graphic::mPrograms[selectedprogram]->mName.c_str(), ImGuiComboFlags_None))
    {
        for (unsigned int iter = 0; iter < Graphic::mPrograms.size(); ++iter)
        {
            Program* p = Graphic::mPrograms[iter];
            const bool isSelected = (iter == selectedprogram);
            if (ImGui::Selectable(Graphic::mPrograms[iter]->mName.c_str(), isSelected))
            {
                if (selectedprogram != iter)
                {
                    changed = true;
                    selectedprogram = iter;
                }
            }
            if (isSelected)
            {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }

    ImGui::SameLine();
    static unsigned int newprogramID = -1;
    if (ImGui::Button("New"))
    {
        Program* program = new Program("New Shader");
        program->Initialize(0, 0);
        program->mID = newprogramID;
        newprogramID = static_cast<unsigned int>(Graphic::mPrograms.size());
        Graphic::mPrograms[newprogramID] = program;

        ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
        ImGui::OpenPopup("New Program");
    }
    if (ImGui::BeginPopupModal("New Program", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        Utility::MyInputTextString("Name", &Graphic::mPrograms[newprogramID]->mName);

        if (ImGui::Button("Apply"))
        {
            newprogramID = -1;

            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    Program* program = Graphic::mPrograms.at(selectedprogram);

    unsigned int shaderProgram = program->mNumber;
    unsigned int vertexID = program->mVertexID;
    unsigned int fragmentID = program->mFragmentID;
    if (ImGui::BeginTabBar("##Shader"))
    {
        ShaderSourceSetting(program, changed, "Vertex Shader", SHADER_TYPE::VERTEX_SHADER);
        ShaderSourceSetting(program, changed, "Fragmnet Shader", SHADER_TYPE::FRAGMENT_SHADER);

        ImGui::EndTabBar();
    }
}

void Interface::ShaderSourceSetting(Program* program, bool changed, const std::string& title, SHADER_TYPE type)
{
    unsigned int shaderProgram = program->mNumber;
    unsigned int vertexID = program->mVertexID;
    unsigned int fragmentID = program->mFragmentID;

    Shader* vertex = Graphic::mVertexShader[vertexID];
    Shader* fragment = Graphic::mFragmentShader[fragmentID];

    static std::string err;
    Shader* targetshader = (type == SHADER_TYPE::VERTEX_SHADER) ? vertex : fragment;
    auto& ShaderContainer = (type == SHADER_TYPE::VERTEX_SHADER) ? Graphic::mVertexShader : Graphic::mFragmentShader;
    if (ImGui::BeginTabItem(title.c_str()))
    {
        static unsigned int selectedvertex = vertexID;
        static unsigned int selectedfragment = fragmentID;

        if (changed)
        {
            selectedvertex = vertexID;
            selectedfragment = fragmentID;
        }

        unsigned int& selectedID = (type == SHADER_TYPE::VERTEX_SHADER) ? selectedvertex : selectedfragment;

        if (ImGui::BeginCombo("Shader", ShaderContainer[selectedID]->mName.c_str(), ImGuiComboFlags_None))
        {
            unsigned int sz = static_cast<unsigned int>(ShaderContainer.size());
            for (unsigned int iter = 0; iter < sz; ++iter)
            {
                const bool isSelected = (iter == selectedID);
                Shader* shader = ShaderContainer[iter];
                if (ImGui::Selectable(shader->mName.c_str(), isSelected))
                {
                    selectedID = iter;
                }
                if (isSelected)
                {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }

        ImGui::SameLine();

        static unsigned int newID = -1;
        if (ImGui::Button("New"))
        {
            newID = static_cast<unsigned int>(ShaderContainer.size());
            Shader* newshaderptr = new Shader(type, "New Shader", newID);
            ShaderContainer[newID] = newshaderptr;

            ImVec2 center = ImGui::GetMainViewport()->GetCenter();
            ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
            ImGui::OpenPopup("New Shader Source");

            selectedID = newID;
        }
        if (ImGui::BeginPopupModal("New Shader Source", NULL, ImGuiWindowFlags_AlwaysAutoResize))
        {
            Utility::MyInputTextString("Name", &ShaderContainer[newID]->mName);

            if (ImGui::Button("Apply"))
            {
                ImGui::CloseCurrentPopup();
                newID = -1;
            }

            ImGui::EndPopup();
        }

        if (ImGui::Button("Select"))
        {
            if(type == SHADER_TYPE::VERTEX_SHADER) program->mVertexID = selectedID;
            else program->mFragmentID = selectedID;
            unsigned int vert = vertex->Compile(err);
            unsigned int frag = fragment->Compile(err);
            program->Reset(vert, frag);
        }

        ImGui::Text("Shader Source");
        ImGui::SameLine();
        if (ImGui::Button("Save"))
        {
            targetshader->SaveFile();
        }
        ImGui::SameLine();
        if (ImGui::Button("Load"))
        {
            targetshader->ResetSource();
        }
        ImGui::SameLine();
        if (ImGui::Button("Compile"))
        {
            if (type == SHADER_TYPE::VERTEX_SHADER)
            {
                unsigned int vert = vertex->Compile(err);
                if (err.empty())
                {
                    unsigned int frag = fragment->Compile(err);
                    program->Reset(vert, frag);
                }
            }
            else
            {
                unsigned int frag = fragment->Compile(err);
                if (err.empty())
                {
                    unsigned int vert = vertex->Compile(err);
                    program->Reset(vert, frag);
                }
            }
        }
        std::string Unique = "##" + title;
        Utility::MyInputTextMultiline(Unique.c_str(), &targetshader->mSource, ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 16));
        ImGui::TextColored(ImVec4(1.f, 0.f, 0.f, 1.f), err.c_str());

        ImGui::EndTabItem();
    }
}

void Interface::ObjectsSetting(Object* obj)
{
    Utility::MyInputTextString("Name", &obj->mName);

    ImGui::DragFloat3("Position", &obj->mPosition.x, 0.3f);
    ImGui::DragFloat3("Rotation", &obj->mRot.x, 1.0f, -180.0f, 180.0f);
    ImGui::DragFloat3("Size", &obj->mScale.x, 0.1f, 0.0f, 10000.0f);

    if (ImGui::CollapsingHeader("Graphic"))
    {
        if (ImGui::CollapsingHeader("Material"))
        {
            ImGui::Text("Texture");
            
            unsigned int unique_id = 0;
            for (auto& t : obj->mTextures)
            {
                std::string title = "Texture##Object" + std::to_string(unique_id++);
                if (ImGui::BeginListBox(title.c_str()))
                {
                    for (auto texture : Graphic::mTextures)
                    {
                        const bool isselected = (texture.first == t);
                        if (ImGui::Selectable(texture.second->mName.c_str(), isselected))
                        {
                            t = texture.first;
                        }
                        if (isselected)
                        {
                            ImGui::SetItemDefaultFocus();
                        }
                    }

                    ImGui::EndListBox();
                }
                ImGui::Spacing();
            }

            if (ImGui::Button("New Texture")) obj->mTextures.push_back(0);

            ImGui::DragFloat3("Ambient##Material", &obj->mMaterialAmbientVector.x, 0.01f, 0.0f, 1.0f);
            ImGui::DragFloat3("Specular##Material", &obj->mMaterialSpecularVector.x, 0.01f, 0.0f, 1.0f);
            ImGui::DragFloat3("Diffuse##Material", &obj->mMaterialDiffuseVector.x, 0.01f, 0.0f, 1.0f);
            ImGui::DragFloat("Shininess##Material", &obj->mShinessFactor, 0.01f, 0.0f, 1.0f);
        }
    }

    if (Light* lit = dynamic_cast<Light*>(obj); lit != nullptr)
    {
        if (ImGui::CollapsingHeader("Light"))
        {
            ImGui::DragFloat("Linear##Light", &lit->mLinear, 0.001f, 0.0f, 1.0f);
            ImGui::DragFloat("Quadratic##Light", &lit->mQuadratic, 0.001f, 0.0f, 1.0f);
        }
    }

    if (ImGui::Button("Copy"))
    {
        Object* newobject = obj->CopyObject();
        Engine::mCurrentLevel->AddObject(newobject);
    }
    if (ImGui::Button("Json"))
    {
        std::string A = R"(,{
                "Name": ")";
        A += obj->mName;
        A += R"(",
                "Position" : [)";
        A += Utility::to_string_precision(obj->mPosition.x) + ", " + Utility::to_string_precision(obj->mPosition.y) + ", " + Utility::to_string_precision(obj->mPosition.z);
        A += R"(],
                "Rotation" : [)";
        A += Utility::to_string_precision(obj->mRot.x) + ", " + Utility::to_string_precision(obj->mRot.y) + ", " + Utility::to_string_precision(obj->mRot.z);
        A += R"(],
                "Scale" : [)";
        A += Utility::to_string_precision(obj->mScale.x) + ", " + Utility::to_string_precision(obj->mScale.y) + ", " + Utility::to_string_precision(obj->mScale.z);
        A += R"(],
                "Shiness" : )";
        A += Utility::to_string_precision(obj->mShinessFactor) + R"(,
                "Ambient" : [)";
        A += Utility::to_string_precision(obj->mMaterialAmbientVector.x) + ", " + Utility::to_string_precision(obj->mMaterialAmbientVector.y) + ", " + Utility::to_string_precision(obj->mMaterialAmbientVector.z);
        A += R"(],
                "Diffuse" : [)";
        A += Utility::to_string_precision(obj->mMaterialDiffuseVector.x) + ", " + Utility::to_string_precision(obj->mMaterialDiffuseVector.y) + ", " + Utility::to_string_precision(obj->mMaterialDiffuseVector.z);
        A += R"(],
                "Specular" : [)";
        A += Utility::to_string_precision(obj->mMaterialSpecularVector.x) + ", " + Utility::to_string_precision(obj->mMaterialSpecularVector.y) + ", " + Utility::to_string_precision(obj->mMaterialSpecularVector.z);
        A += R"(],
                "FaceSize" : )";
        A += Utility::to_string_precision(obj->mFaceSize) + R"(,
                "ProgramID" : )" + Utility::to_string_precision(obj->mProgramID) + R"(,
                "VAOID" : )" + Utility::to_string_precision(obj->mVAO);
        A += R"(,
                "Texture" : [)";
        for (auto t = obj->mTextures.begin(); t != obj->mTextures.end(); ++t)
        {
            A += std::to_string(*t);
            if (t == obj->mTextures.end() - 1)
            {
                break;
            }
            A += ", ";
        }
        A += R"(],
                "Type" : )";
        if (dynamic_cast<Wall*>(obj) != nullptr)
        {
            A += "2";
        }
        A += R"(
            })";
        glfwSetClipboardString(Window::Windowptr, A.c_str());
    }
}