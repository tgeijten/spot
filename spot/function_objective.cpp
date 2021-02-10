#include "function_objective.h"

namespace spot
{
	function_objective::function_objective( size_t d, objective_function_t func, const par_vec& mean, const par_vec& stdev, const par_vec& lower, const par_vec& upper ) :
		func_( func )
	{
		for ( size_t i = 0; i < d; ++i )
			info_.add( par_info( xo::stringf( "%d", i ), mean[ i ], stdev[ i ], lower[ i ], upper[ i ] ) );
	}

	function_objective::function_objective( size_t d, objective_function_t func, par_t mean, par_t stdev, par_t lower, par_t upper ) :
		func_( func )
	{
		for ( size_t i = 0; i < d; ++i )
			info_.add( par_info( xo::stringf( "%d", i ), mean, stdev, lower, upper ) );
	}
}
