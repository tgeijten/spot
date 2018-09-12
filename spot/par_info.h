#pragma once

#include "types.h"
#include "xo/container/prop_node.h"

#if defined(_MSC_VER)
#	pragma warning( push )
#	pragma warning( disable: 4251 )
#endif

namespace spot
{
	const par_value default_std_factor = 0.1;
	const par_value default_std_minimum = 0.01;
	const par_value default_upper_boundaray = 1e12;
	const par_value default_lower_boundaray = -1e12;

	struct SPOT_API par_info
	{
		par_info() : name(), mean(), std(), min( default_lower_boundaray ), max( default_upper_boundaray ) {}
		par_info( string name, par_value mean, par_value std, par_value min = default_lower_boundaray, par_value max = default_upper_boundaray );
		par_info( string name, const prop_node& pn );

		bool is_valid() const { return std > 0; }

		string name;
		par_value mean;
		par_value std;
		par_value min;
		par_value max;
	};
}

#if defined(_MSC_VER)
#	pragma warning( pop )
#endif
