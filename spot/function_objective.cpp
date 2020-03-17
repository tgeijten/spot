#include "function_objective.h"

namespace spot
{
	function_objective::function_objective( size_t d, objective_function_t func, const par_vec& start, const par_vec& start_std, const par_vec& lower, const par_vec& upper ) :
		func_( func )
	{
		for ( size_t i = 0; i < d; ++i )
			info_.add( par_info( xo::stringf( "%d", i ), start[ i ], start_std[ i ], lower[ i ], upper[ i ] ) );
	}

	function_objective::function_objective( size_t d, objective_function_t func, par_t start, par_t start_std, par_t lower, par_t upper ) :
		func_( func )
	{
		for ( size_t i = 0; i < d; ++i )
			info_.add( par_info( xo::stringf( "%d", i ), start, start_std, lower, upper ) );
	}
}
