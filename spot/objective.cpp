#include "objective.h"

#include "flut/math/math.hpp"

namespace spot
{
	function_objective::function_objective( size_t d, objective_function_t func, const par_vec& start, const par_vec& start_std, const par_vec& lower, const par_vec& upper ) :
	func_( func )
	{
		for ( size_t i = 0; i < d; ++i )
			info_.add( stringf( "%d", i ), start[ i ], start_std[ i ], lower[ i ], upper[ i ] );
	}

	function_objective::function_objective( size_t d, objective_function_t func, par_value start, par_value start_std, par_value lower, par_value upper )
	{
		for ( size_t i = 0; i < d; ++i )
			info_.add( stringf( "%d", i ), start, start_std, lower, upper );
	}
}
