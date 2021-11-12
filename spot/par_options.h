#pragma once

#include "spot_types.h"

namespace spot
{
	const par_t default_lower_boundaray = -1e12;
	const par_t default_upper_boundaray = 1e12;

	struct SPOT_API par_options
	{
		par_t auto_std_factor = 0.1;
		par_t auto_std_minimum = 0.01;
		par_t lower_boundaray = default_lower_boundaray;
		par_t upper_boundaray = default_upper_boundaray;
	};

}
