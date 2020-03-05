#pragma once

#include "types.h"
#include "xo/container/prop_node.h"

namespace spot
{
	const par_t default_std_factor = 0.1;
	const par_t default_std_minimum = 0.01;
	const par_t default_upper_boundaray = 1e12;
	const par_t default_lower_boundaray = -1e12;

	struct SPOT_API par_info
	{
		par_info() : name(), mean(), std(), min( default_lower_boundaray ), max( default_upper_boundaray ) {}
		par_info( string name, par_t mean, par_t std, par_t min = default_lower_boundaray, par_t max = default_upper_boundaray );
		par_info( string name, const prop_node& pn );

		bool is_constant() const { return std == 0; }
		bool is_valid() const { return mean == mean && std >= 0 && min <= max; }

		string name;
		par_t mean;
		par_t std;
		par_t min;
		par_t max;
	};
}
