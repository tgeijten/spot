#include "evaluator.h"

#include "objective.h"

namespace spot
{
	fitness_t evaluator::evaluate_noexcept( const objective& o, const search_point& point ) const noexcept
	{
		try
		{
			return o.evaluate( point );
		}
		catch ( std::exception& e )
		{
			error_message_ = "Exception evaluating objective: " + string( e.what() );
			return o.info().worst_fitness();
		}
		catch ( ... )
		{
			error_message_ = "Unknown error evaluating objective";
			return o.info().worst_fitness();
		}
	}

	fitness_vec evaluator::evaluate( const objective& o, const search_point_vec& point_vec, priority_t prio ) const
	{
		// single threaded evaluation
		fitness_vec results;
		results.reserve( point_vec.size() );
		for ( const auto& sp : point_vec )
			results.push_back( o.evaluate( sp ) );
		return results;
	}

	std::future<fitness_t> async_evaluator::evaluate_async( const objective& o, const search_point& point ) const
	{
		return std::async( std::launch::async,
			[&]() {
				xo::set_thread_priority( thread_prio_ );
				return evaluate_noexcept( o, point );
			} );
	}

	fitness_vec async_evaluator::evaluate( const objective& o, const search_point_vec& point_vec, priority_t prio ) const
	{
		fitness_vec results( point_vec.size(), o.info().worst_fitness() );
		vector< pair< std::future< fitness_t >, index_t > > threads;

		for ( index_t eval_idx = 0; eval_idx < point_vec.size(); ++eval_idx )
		{
			// wait for threads to finish
			while ( threads.size() >= max_threads_ )
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
			threads.push_back( std::make_pair( evaluate_async( o, point_vec[ eval_idx ] ), eval_idx ) );
		}

		// wait for remaining threads
		for ( auto& f : threads )
			results[ f.second ] = f.first.valid() ? f.first.get() : o.info().worst_fitness();

		return results;
	}
}
