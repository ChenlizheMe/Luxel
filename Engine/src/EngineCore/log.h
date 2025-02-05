#pragma once

#include "pch.h"

#include "Core.h"

namespace Luxel
{

	class LUXEL_API LogSystem
	{
	public:

		LogSystem(const LogSystem&) = delete;
		LogSystem(LogSystem&&) = delete;
		LogSystem operator=(const LogSystem&) = delete;

		static void SetLogLevel(int level);

		template<typename... Args>
		static void Output(const std::string color, const Args&... message)
		{
			std::stringstream ss;

			ss << color;
			((ss << message << " "), ...);
			ss << "\033[0m";

			std::cout << ss.str() << "\n";
		}

		template<typename... Args>
		static void DEBUG(const Args&... message)
		{
			if (currentLogLevel <= 0)
				Output("\033[34m", "[DEBUG]:", message...);
		}

		template<typename... Args>
		static void INFO(const Args&... message)
		{
			if (currentLogLevel <= 1)
				Output("\033[0m", "[INFO]:", message...);
		}

		template<typename... Args>
		static void WARNING(const Args&... message)
		{
			if (currentLogLevel <= 2)
				Output("\033[33m", "[WARNING]:", message...);
		}

		template<typename... Args>
		static void ERR(const Args&... message)
		{
			if (currentLogLevel <= 3)
				Output("\033[31m", "[ERROR]:", message...);
		}

		template<typename... Args>
		static void FATAL(const Args&... message)
		{
			if (currentLogLevel <= 4)
				Output("\033[35m", "[FATAL]:", message...);
		}

	private:
		static int currentLogLevel;

		LogSystem();
		~LogSystem();
	};
}
