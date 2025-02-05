#pragma once

#pragma warning(push)
#pragma warning(disable : 4251)

#ifdef LUXEL_PLATFORM_WINDOWS
	#ifdef LUXEL_BUILD_DLL
		#define LUXEL_API __declspec(dllexport)
	#else
		#define LUXEL_API __declspec(dllimport)
	#endif
#endif

#define ui32 uint32_t
#define ui16 uint16_t

#define Debug(...) Luxel::LogSystem::DEBUG(__VA_ARGS__)
#define Info(...) Luxel::LogSystem::INFO(__VA_ARGS__)
#define Warning(...) Luxel::LogSystem::WARNING(__VA_ARGS__)
#define Error(...) Luxel::LogSystem::ERR(__VA_ARGS__)
#define Fatal(...) Luxel::LogSystem::FATAL(__VA_ARGS__)
#define LogLevel(x) Luxel::LogSystem::SetLogLevel(x)