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

#include "xo/xo_types.h"
#include "xo/string/string_type.h"
#include "xo/container/vector_type.h"
#include "xo/utility/pointer_types.h"

namespace spot
{
	class par_io;
	class search_point;
	class objective;

	using xo::string;
	using xo::vector;
	using xo::prop_node;

	using xo::prop_node;
	using xo::path;
	using xo::index_t;
	using xo::no_index;

	using xo::u_ptr;
	using xo::s_ptr;

	using fitness_t = double;
	using fitness_vec_t = std::vector< fitness_t >;

	using par_value = double;
	using par_vec = std::vector< par_value >;
}
