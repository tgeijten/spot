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
#include <string>
#include <vector>

namespace spot
{
	class par_io;

	using std::string;

	using xo::prop_node;
	using xo::path;
	using xo::index_t;
	using xo::no_index;

	using fitness_t = double;
	using fitness_vec_t = std::vector< fitness_t >;

	using par_value = double;
	using par_vec = std::vector< par_value >;
}
