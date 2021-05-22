#pragma once

#include <functional>
#include <vector>
#include <map>

using Func = std::function<void()>;
using std::vector;
using std::map;

enum CollisionShape
{
	RECTANGLE,
	CIRCLE,
};

struct Collision
{
public:
	CollisionShape shape;
	Vector2 size; // Use x as radius if shape is circle	
	Vector2 offset;

	Collision(CollisionShape t, Vector2 s, Vector2 o = (Vector2){ 0.0f, 0.0f })
		: shape(t), size(s), offset(o) {}

	Collision(float radius, Vector2 o = (Vector2){ 0.0f, 0.0f })
		: shape(CIRCLE), size((Vector2){ radius, 0.0f }), offset(o) {}

	Collision(float width, float height, Vector2 o = (Vector2){ 0.0f, 0.0f })
		: shape(RECTANGLE), size((Vector2){ width, height }), offset(o) {}

	Collision(Vector2 s, Vector2 o = (Vector2){ 0.0f, 0.0f })
		: shape(RECTANGLE), size(s), offset(o) {}
};

class Object
{
public:
	float x;
	float y;
	const int id;
	Collision col;
	vector<Func> Alarms;
	vector<float> alarm;
	map<Object*, Func> Collisions; // Collision impl is flawed, do not use
	
	Func Create;
	Func PreStep;
	Func PostStep;
	Func Step;
	Func DrawGUI;
	Func Draw;
	Func Draw2;
	Func Destroy;
	
	Object() : x(0), y(0), id(-1), col(1.0f, 1.0f), Alarms(), alarm(), Collisions() {}
	void DoAlarm()
	{
		for (int i = 0; i < alarm.size(); i++)
			if (alarm[i] >= 0.0001f && (alarm[i] -= GetFrameTime(), alarm[i]) < 0.0001f)
				if (Alarms[i])
					Alarms[i]();
	}
};
