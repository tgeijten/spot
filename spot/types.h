#pragma once

#if defined(_MSC_VER)
#	ifdef SPOT_EXPORTS
#		define SPOT_API __declspec(dllexport)
#	else
#		define SPOT_API __declspec(dllimport)
#	endif
#else
#	define SPOT_API
#endif

#include "flut/system/types.hpp"
#include "flut/math/optional.hpp"

namespace spot
{
	using namespace flut;

	using fitness_t = double;
	using fitness_vec_t = vector< fitness_t >;

	using par_value = double;
	using optional_par_value = optional< par_value >;
	using par_vec = vector< par_value >;
}
