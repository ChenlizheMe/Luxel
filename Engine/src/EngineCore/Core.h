#pragma once

#ifdef LUXEL_PLATFORM_WINDOWS
	#ifdef LUXEL_BUILD_DLL
		#define LUXEL_API __declspec(dllexport)
	#else
		#define LUXEL_API __declspec(dllimport)
	#endif
#endif