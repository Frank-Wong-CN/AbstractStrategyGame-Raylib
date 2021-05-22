#pragma once

#include <raylib.h>
#include <string>
#include <vector>

#include "messenger.hpp"

using std::string;
using std::vector;
using Messenger::MessageDispatcher;
using Messenger::MessageBroadcaster;
using Messenger::Message;
using Messenger::NO_NEED_AT_ALL;
using Messenger::AT_LEAST_ONE;
using Messenger::ALL_REQUIRED;
using Messenger::ALL_PRESENCE;
using Messenger::ALL_PRESENCE_SPAM;

class Object;

template<typename T = int> class GameManager;
using gm = GameManager<>;

template<typename T>
class GameManager
{	
friend int main(int argc, char *argv[]);
private:
	static vector<string> args;
	static vector<Camera2D> cameras;
	static int currentCamera;
	static vector<Object*> instances;
	static MessageDispatcher disp;
	static MessageBroadcaster brod;
	static int currentID;
	
	static void Main(int argc, char *argv[]);
	static void InitGame(int argc, char *argv[]);
	static void PreStep();
	static void Step();
	static void PostStep();
	static void Draw();
	static void Cleanup();

public:
	static Vector2 mouse;
	
	static vector<string> &GetCmdLines();
	static Camera2D &GetCurrentCamera();
	static void SwitchCamera(int n);
	static void CreateNewCamera(Vector2 target = {0});
	static int GetCurrentCameraNumber();
	static void RemoveCamera(int n);
	
	static void AddInstance(Object *o);
	static void RemoveInstance(int id);
};
