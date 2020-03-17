#pragma once

#define SPOT_DISABLE_EVALUATOR

#ifndef SPOT_DISABLE_EVALUATOR
#	define SPOT_EVALUATOR_ENABLED 1
#else
#	define SPOT_EVALUATOR_ENABLED 0
#endif

#if defined( _MSC_VER )
#	ifdef SPOT_EXPORTS
#		define SPOT_API __declspec(dllexport)
#	else
#		define SPOT_API __declspec(dllimport)
#	endif
#else
#	define SPOT_API
#endif
