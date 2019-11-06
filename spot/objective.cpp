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
			info_.add( par_info( xo::stringf( "%d", i ), start[ i ], start_std[ i ], lower[ i ], upper[ i ] ) );
	}

	function_objective::function_objective( size_t d, objective_function_t func, par_value start, par_value start_std, par_value lower, par_value upper ) :
	func_( func )
	{
		for ( size_t i = 0; i < d; ++i )
			info_.add( par_info( xo::stringf( "%d", i ), start, start_std, lower, upper ) );
	}

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

	fitness_vec_t objective::evaluate_async( const search_point_vec& pop, size_t max_threads, xo::thread_priority prio ) const
	{
		fitness_vec_t results( pop.size(), info().worst_fitness() );
		std::vector< std::pair< std::future< fitness_t >, index_t > > threads;

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
}
