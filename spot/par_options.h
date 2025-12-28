#pragma once

#include "spot_types.h"
#include "xo/container/flat_map.h"
#include "xo/string/pattern_matcher.h"

namespace spot
{
	const par_t default_lower_boundaray = par_t( -1e12 );
	const par_t default_upper_boundaray = par_t( 1e12 );

	struct SPOT_API par_options
	{
		par_t auto_std_factor = par_t( 0.1 );
		par_t auto_std_offset = par_t( 0.0 );
		par_t auto_std_minimum = par_t( 0.01 );
		par_t lower_boundaray = default_lower_boundaray;
		par_t upper_boundaray = default_upper_boundaray;
		xo::flat_map<string, string> aliases_;
	};

	// #todo: use this struct for objective_info::import_mean_std
	struct SPOT_API par_import_settings
	{
		bool import_std = true;
		bool use_best_as_mean = false;
		bool locked = false;
		par_t std_factor = 1.0;
		par_t std_offset = 0.0;
		par_t value_factor = 1.0;
		par_t value_offset = 0.0;
		xo::pattern_matcher include = {};
		xo::pattern_matcher exclude = {};
	};
}
