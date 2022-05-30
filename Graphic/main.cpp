#include "Engine.hpp"

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#define _CRTDBG_MAP_ALLOC
#define new new(_NORMAL_BLOCK, __FILE__, __LINE__)

#include <crtdbg.h>

int main(int, char**)
{
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    //_CrtSetBreakAlloc(2528);

    Engine::Initialize();

    while (Engine::ShouldUpdate()) Engine::Update();

    Engine::Close();

    return 0;
}
