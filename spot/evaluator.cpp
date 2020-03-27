#include "evaluator.h"

#include "objective.h"
#include "xo/system/system_tools.h"
#include "spot/search_point.h"
#include "async_evaluator.h"

namespace spot
{
	result<fitness_t> evaluator::evaluate_noexcept( const objective& o, const search_point& point ) noexcept
	{
		try
		{
			return o.evaluate( point );
		}
		catch ( std::exception& e )
		{
			return xo::error_message( e.what() );
		}
		catch ( ... )
		{
			return xo::error_message( "Error evaluating objective" );
		}
	}

	vector< result<fitness_t> > evaluator::evaluate( const objective& o, const search_point_vec& point_vec, priority_t prio )
	{
		// single threaded evaluation
		vector< result<fitness_t> > results;
		results.reserve( point_vec.size() );
		for ( const auto& sp : point_vec )
			results.push_back( evaluate_noexcept( o, sp ) );
	
		return results;
	}

	evaluator& global_evaluator()
	{
		static async_evaluator g_async_eval( std::thread::hardware_concurrency(), xo::thread_priority::low );

		return g_async_eval;
	}
}
