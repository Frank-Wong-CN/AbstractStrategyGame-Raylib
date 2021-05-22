#pragma once

#include <string>

using std::string;

template<typename T = int> struct MessageIDDefinitions;
using mid = MessageIDDefinitions<>;

template<typename T>
struct MessageIDDefinitions
{
public:
	static string keyPressed;
};
