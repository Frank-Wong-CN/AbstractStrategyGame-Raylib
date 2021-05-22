#pragma once

#include <raylib.h>
#include <string>

using std::string;

template<typename T> class GameManager;

template<typename T = int> class ConfigManager;
using conf = ConfigManager<>;

template<typename T>
class ConfigManager
{
friend class GameManager<int>;
private:
	static int screenWidth;
	static int screenHeight;
	static int targetFPS;
	static string gameTitle;
	
public:
	static void SetScreenSize(int width, int height);
	static Vector2 GetScreenSize();
	static int GetScreenWidth();
	static int GetScreenHeight();
	static void SetTargetFPS(int fps);
	static int GetTargetFPS();
	static void SetGameTitle(string title);
	static string GetGameTitle();
};
