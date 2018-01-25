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

#include "xo/utility/types.h"
#include "xo/utility/optional.h"

#include <vector>

namespace spot
{
	using namespace xo;

	using fitness_t = double;
	using fitness_vec_t = std::vector< fitness_t >;

	using par_value = double;
	using optional_par_value = optional< par_value >;
	using par_vec = std::vector< par_value >;
}
