#include "objective.h"

#include <future>

#include "xo/system/system_tools.h"
#include "xo/system/log.h"

namespace spot
{
	function_objective::function_objective( size_t d, objective_function_t func, const par_vec& start, const par_vec& start_std, const par_vec& lower, const par_vec& upper ) :
	func_( func )
	{
		for ( size_t i = 0; i < d; ++i )
			info_.add( stringf( "%d", i ), start[ i ], start_std[ i ], lower[ i ], upper[ i ] );
	}

	function_objective::function_objective( size_t d, objective_function_t func, par_value start, par_value start_std, par_value lower, par_value upper ) :
	func_( func )
	{
		for ( size_t i = 0; i < d; ++i )
			info_.add( stringf( "%d", i ), start, start_std, lower, upper );
	}

	double objective::evaluate_noexcept( const search_point& point, thread_priority prio ) const
	{
		try
		{
			set_thread_priority( prio );
			return evaluate( point );
		}
		catch ( std::exception& e )
		{
			log::error( "error evaluating objective: ", e.what() );
			return info_.worst_fitness();
		}
	}

	std::future< double > objective::evaluate_async( const search_point& point, thread_priority prio ) const
	{
		return std::async( std::launch::async, &objective::evaluate_noexcept, this, point, prio );
	}
}
