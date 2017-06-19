#include "objective.h"

#include "flut/math/math.hpp"

namespace spot
{
	function_objective::function_objective( size_t d, function_t func, bool minimize, const par_vec& start, const par_vec& start_std, const par_vec& upper, const par_vec& lower ) :
	func_( func )
	{
		info_.set_minimize( minimize );
		for ( size_t i = 0; i < d; ++i )
			info_.add( stringf( "%d", i ), start[ i ], start_std[ i ], i < lower.size() ? lower[ i ] : -1e18, i < lower.size() ? lower[ i ] : 1e18 );
	}
}
