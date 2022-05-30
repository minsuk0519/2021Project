#pragma once
#include "LevelBase.hpp"

class Level0 : public LevelBase
{
public:
private:
	virtual void Initialize() override;
	virtual void Run() override;
	virtual void ShutDown() override;

	Object* hello = nullptr;
};

