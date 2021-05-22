#include "gm.hpp"
#include "conf.hpp"
#include "msgid.hpp"
#include "util.hpp"

#include "object.hpp"

void Start(int argc, char *argv[]);
void End();

template<> vector<string> gm::args {};
template<> vector<Camera2D> gm::cameras {};
template<> int gm::currentCamera = 0;
template<> Vector2 gm::mouse = { 0 };
template<> vector<Object*> gm::instances {};
template<> MessageDispatcher gm::disp {};
template<> MessageBroadcaster gm::brod {};
template<> int gm::currentID = 0;

template<> void gm::InitGame(int argc, char *argv[])
{
	for (int i = 0; i < argc; i++)
		gm::args.emplace_back(argv[i]);
	
	gm::cameras.emplace_back((Camera2D){ 0 });
	Camera2D &camera = gm::cameras[currentCamera];
	
	camera.target = (Vector2){ 0, 0 };
	camera.offset = (Vector2){ conf::screenWidth / 2.0f, conf::screenHeight / 2.0f };
	camera.rotation = 0.0f;
	camera.zoom = 1.0f;
	
	Start(argc, argv);
	
	InitWindow(conf::screenWidth, conf::screenHeight, conf::gameTitle.c_str());
	SetTargetFPS(conf::targetFPS * 2);

	for (auto &o : gm::instances)
		if (o->Create)
			o->Create();
}

template<> void gm::PreStep()
{
	/* Event Loop Here */
	gm::disp.DispatchNext();
	gm::brod.DispatchNext();
	
	/* Custom PreStep Here */
	for (auto &o : gm::instances)
		if (o->PreStep)
			o->PreStep();
}

template<> void gm::PostStep()
{
	/* Custom PostStep Here */
	for (auto &o : gm::instances)
		if (o->PostStep)
			o->PostStep();
}

template<> void gm::Step()
{
	gm::mouse = ::GetScreenToWorld2D(::GetMousePosition(), gm::cameras[gm::currentCamera]);
	
	/* Alarm Countdown Here */
	for (auto &o : gm::instances)
		o->DoAlarm();

	/* Collisions Here */
	for (auto &o : gm::instances)
		for (auto &c : o->Collisions)
		{
			if (!c.first || !c.second)
				continue;

			Object &obj = *o;
			Object &col = *(c.first);
			bool objCircle = false;
			bool colCircle = false;
			Rectangle o1, o2;
			
			if (obj.col.shape == CIRCLE)
				objCircle = true;
			o1 = (Rectangle){ obj.x + obj.col.offset.x, obj.y + obj.col.offset.y, obj.col.size.x, obj.col.size.y };
			
			if (col.col.shape == CIRCLE)
				colCircle = true;
			o2 = (Rectangle){ col.x + col.col.offset.x, col.y + col.col.offset.y, col.col.size.x, col.col.size.y };
			
			if (objCircle && colCircle)
				if (!CheckCollisionCircles((Vector2){ o1.x, o1.y }, o1.width, (Vector2){ o2.x, o2.y }, o2.width))
					continue;
			if (objCircle && !colCircle)
				if (!CheckCollisionCircleRec((Vector2){ o1.x, o1.y }, o1.width, o2))
					continue;
			if (!objCircle && colCircle)
				if (!CheckCollisionCircleRec((Vector2){ o2.x, o2.y }, o2.width, o1))
					continue;
			if (!objCircle && !colCircle)
				if (!CheckCollisionRecs(o1, o2))
					continue;
			(c.second)();
		}	

	/* Custom Steps Here */
	for (auto &o : gm::instances)
		if (o->Step)
			o->Step();
	
	/* Key Checks Here */
	int key = ::GetKeyPressed();
	while (key > 0)
	{
		Message *temp = new Message(mid::keyPressed, (void *)key, NO_NEED_AT_ALL, false);
		gm::brod.Acquire(temp);
		key = ::GetKeyPressed();
	}
	
	/* Triggers Here */
}

template<> void gm::Draw()
{
	::BeginDrawing();
		::ClearBackground(RAYWHITE);
		
		::BeginMode2D(gm::cameras[gm::currentCamera]);
			/* Draw Scene */
			for (auto &o : gm::instances)
				if (o->Draw)
					o->Draw();
		::EndMode2D();
		
		/* Draw GUI */
		for (auto &o : gm::instances)
			if (o->DrawGUI)
				o->DrawGUI();
	::EndDrawing();
}

template<> void gm::Cleanup()
{
	::CloseWindow();
	End();
	
	/* Cleanup Other Stuff */
	for (auto &o : gm::instances)
		if (o->Destroy)
			o->Destroy();
}

template<> void gm::Main(int argc, char *argv[])
{
	gm::InitGame(argc, argv);
	
	while (!::WindowShouldClose())
	{
		gm::PreStep();
		gm::Step();
		gm::PostStep();
		gm::Draw();
	}
	
	gm::Cleanup();
}

template<> vector<string> &gm::GetCmdLines()
{ return gm::args; }

template<> Camera2D &gm::GetCurrentCamera()
{ return gm::cameras[gm::currentCamera]; }

template<> void gm::SwitchCamera(int n)
{ gm::currentCamera = n; }

template<> void gm::CreateNewCamera(Vector2 target)
{
	gm::cameras.emplace_back((Camera2D) {
		(Vector2) { conf::screenWidth / 2.0f, conf::screenHeight / 2.0f },
		(Vector2) target,
		0.0f,
		1.0f
	});
}

template<> int gm::GetCurrentCameraNumber()
{ return gm::currentCamera; }

template<> void gm::RemoveCamera(int n)
{ if (cameras.size() > n) cameras.erase(cameras.begin() + n); }

template<> void gm::AddInstance(Object *o)
{
	int *idptr = (int *)((float*)&(o->y) + 1);
	*idptr = gm::currentID++;
	gm::instances.push_back(o);
}

template<> void gm::RemoveInstance(int id)
{
	gm::instances.erase(std::remove_if(gm::instances.begin(), gm::instances.end(), [=](Object *obj){
		return obj->id == id;
	}));
}
