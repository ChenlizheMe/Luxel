#include "pch.h"

#include "log.h"

namespace Luxel
{
	int LogSystem::currentLogLevel = 0;

	LogSystem::LogSystem()
	{

	}
	LogSystem::~LogSystem()
	{

	}
	void LogSystem::SetLogLevel(int level)
	{
		currentLogLevel = level;
		std::cout << "Set Log System Level to: " << currentLogLevel << '\n';
	}
}