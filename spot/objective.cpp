#include "objective.h"

#include <future>

#include "xo/system/system_tools.h"
#include "xo/system/log.h"

namespace spot
{
#if !SPOT_EVALUATOR_ENABLED
	fitness_t objective::evaluate_in_thread_noexcept( const search_point& point, xo::thread_priority prio ) const
	{
		xo::set_thread_priority( prio );
		return evaluate_noexcept( point );
	}

	fitness_t objective::evaluate_noexcept( const search_point& point ) const noexcept
	{
		try
		{
			return evaluate( point );
		}
		catch ( std::exception& e )
		{
			xo::log::error( "Exception caught during object evaluation, returning worst fitness: ", e.what() );
			return info_.worst_fitness();
		}
		catch ( ... )
		{
			xo::log::error( "Unknown exception caught during object evaluation, returning worst fitness" );
			return info_.worst_fitness();
		}
	}

	std::future< fitness_t > objective::evaluate_async( const search_point& point, xo::thread_priority prio ) const
	{
		return std::async( std::launch::async, &objective::evaluate_in_thread_noexcept, this, point, prio );
	}

	vector< result<fitness_t> > objective::evaluate_async( const search_point_vec& pop, size_t max_threads, xo::thread_priority prio ) const
	{
		vector< result<fitness_t> > results( pop.size(), info().worst_fitness() );
		vector< pair< std::future< fitness_t >, index_t > > threads;

		for ( index_t eval_idx = 0; eval_idx < pop.size(); ++eval_idx )
		{
			// wait for threads to finish
			while ( threads.size() >= max_threads )
			{
				for ( auto it = threads.begin(); it != threads.end(); )
				{
					if ( it->first.wait_for( std::chrono::milliseconds( 1 ) ) == std::future_status::ready )
					{
						// a thread is finished, add it to the results and make room for a new thread
						results[ it->second ] = it->first.get();
						it = threads.erase( it );
					}
					else ++it;
				}
			}

			// add new thread
			threads.push_back( std::make_pair( evaluate_async( pop[ eval_idx ], prio ), eval_idx ) );
		}

		// wait for remaining threads
		for ( auto& f : threads )
			results[ f.second ] = f.first.valid() ? f.first.get() : info().worst_fitness();

		return results;
	}
#endif
}
