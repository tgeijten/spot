#pragma once

#include "spot_types.h"
#include "par_options.h"
#include "xo/container/prop_node.h"

namespace spot
{
	struct SPOT_API par_info
	{
		//par_info() : name(), mean(), std(), min( default_lower_boundaray ), max( default_upper_boundaray ) {}
		par_info( string name, par_t mean, par_t std, par_t min = default_lower_boundaray, par_t max = default_upper_boundaray );
		par_info( string name, const prop_node& pn, const par_options& opt );

		bool is_constant() const { return std == 0; }
		bool is_valid() const { return mean == mean && std >= 0 && min <= max; }

		string name;
		par_t mean;
		par_t std;
		par_t min;
		par_t max;
	};
}
