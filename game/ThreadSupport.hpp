#pragma once

class ThreadFunction
{
private:
	std::function<void()> threadFunction;
public:
	ThreadFunction(std::function<void()> func) : threadFunction(func) {}
	
	void operator()()
	{
		threadFunction();
	}
};