#pragma once

#include "spot_types.h"
#include "xo/container/flat_map.h"

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
}
