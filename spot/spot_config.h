#pragma once

//#define SPOT_PRECISION_SINGLE

#if defined( SPOT_PRECISION_SINGLE )
#	define SPOT_DEFAULT_PRECISION_TYPE float
#else
#	define SPOT_DEFAULT_PRECISION_TYPE double
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
