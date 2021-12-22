#pragma once

#include "spot_config.h"
#include "xo/xo_types.h"
#include "xo/string/string_type.h"
#include "xo/container/vector_type.h"
#include "xo/container/pair_type.h"
#include "xo/utility/pointer_types.h"
#include "xo/utility/result.h"

namespace spot
{
	class par_io;
	class search_point;
	class objective;
	class objective_info;
	class optimizer;
	struct stop_condition;

	using xo::string;
	using xo::vector;
	using xo::pair;
	using xo::prop_node;

	using xo::prop_node;
	using xo::path;
	using xo::index_t;
	using xo::no_index;

	using xo::u_ptr;
	using xo::s_ptr;

	using xo::result;

	using fitness_t = SPOT_DEFAULT_PRECISION_TYPE;
	using fitness_vec = vector<fitness_t>;

	using par_t = SPOT_DEFAULT_PRECISION_TYPE;
	using par_vec = vector<par_t>;

	using priority_t = SPOT_DEFAULT_PRECISION_TYPE;
}
