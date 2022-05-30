#pragma once

#include <string>
#include <vector>

class LevelBase;

class DataManager
{
private:
	static void LoadGraphic();
	static void SaveGraphic();

	static void LoadLevel();
	static void SaveLevel();

public:
	static void SetLevel(LevelBase* level);
	static void LoadBaseData();

	static void SaveBaseData();
};