#include "evaluator.h"

#include "objective.h"
#include <thread>
#include "xo/system/system_tools.h"
#include "async_evaluator.h"

namespace spot
{
	xo::result<fitness_t> evaluator::evaluate_noexcept( const objective& o, const search_point& point ) const noexcept
	{
		try
		{
			return o.evaluate( point );
		}
		catch ( std::exception& e )
		{
			return string( e.what() );
		}
		catch ( ... )
		{
			return "Error evaluating objective";
		}
	}

	xo::result<fitness_vec> evaluator::evaluate( const objective& o, const search_point_vec& point_vec, priority_t prio ) const
	{
		// single threaded evaluation
		fitness_vec results;
		results.reserve( point_vec.size() );
		for ( const auto& sp : point_vec )
		{
			if ( auto r = evaluate_noexcept( o, sp ) )
				results.push_back( r.value() );
			else return r.error();
		}

		return std::move( results );
	}

	evaluator& global_evaluator()
	{
		static async_evaluator g_async_eval( std::thread::hardware_concurrency(), xo::thread_priority::low );

		return g_async_eval;
	}
}
