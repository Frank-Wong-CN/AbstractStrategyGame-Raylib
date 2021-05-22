#include "conf.hpp"

template<> int conf::screenWidth = 800;
template<> int conf::screenHeight = 600;
template<> int conf::targetFPS = 144;
template<> string conf::gameTitle = "Hello World!";

template<> void conf::SetScreenSize(int width, int height)
{
	conf::screenWidth = width;
	conf::screenHeight = height;
	::SetWindowSize(width, height);
}

template<> Vector2 conf::GetScreenSize()
{ return (Vector2) { (float)conf::screenWidth, (float)conf::screenHeight }; }

template<> int conf::GetScreenWidth()
{ return conf::screenWidth; }

template<> int conf::GetScreenHeight()
{ return conf::screenHeight; }

template<> void conf::SetTargetFPS(int fps)
{
	conf::targetFPS = fps;
	::SetTargetFPS(fps);
}

template<> int conf::GetTargetFPS()
{ return conf::targetFPS; }

template<> void conf::SetGameTitle(string title)
{
	conf::gameTitle = title;
	::SetWindowTitle(title.c_str());
}

template<> string conf::GetGameTitle()
{ return conf::gameTitle; }

