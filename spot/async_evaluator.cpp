#include "async_evaluator.h"

#include "objective.h"

namespace spot
{
	async_evaluator::async_evaluator( size_t max_threads, xo::thread_priority thread_prio ) :
		evaluator(),
		max_threads_( max_threads ),
		thread_prio_( thread_prio )
	{}

	std::future< xo::result< fitness_t > > async_evaluator::evaluate_async( const objective& o, const search_point& point ) const
	{
		return std::async( std::launch::async,
			[&]() {
				xo::set_thread_priority( thread_prio_ );
				return evaluate_noexcept( o, point );
			} );
	}

	void async_evaluator::set_result( xo::result<fitness_t> result, fitness_t* value, xo::error_message* error ) const
	{
		if ( result )
			*value = result.value();
		else *error = result.error();
	}

	xo::result<fitness_vec> async_evaluator::evaluate( const objective& o, const search_point_vec& point_vec, priority_t prio ) const
	{
		fitness_vec results( point_vec.size(), o.info().worst_fitness() );
		xo::error_message error;
		vector< pair< std::future< xo::result< fitness_t > >, index_t > > threads;

		for ( index_t eval_idx = 0; eval_idx < point_vec.size() && error.good(); ++eval_idx )
		{
			// wait for threads to finish
			while ( threads.size() >= max_threads_ && error.good() )
			{
				for ( auto it = threads.begin(); it != threads.end() && error.good(); )
				{
					if ( it->first.wait_for( std::chrono::milliseconds( 1 ) ) == std::future_status::ready )
					{
						// a thread is finished, add it to the results or process error
						set_result( it->first.get(), &results[ it->second ], &error );

						// remove the thread to make room for a new one
						it = threads.erase( it );
					}
					else ++it;
				}
			}

			// add new thread
			if ( error.good() )
				threads.push_back( std::make_pair( evaluate_async( o, point_vec[ eval_idx ] ), eval_idx ) );
		}

		// wait for remaining threads
		for ( auto it = threads.begin(); it != threads.end() && error.good(); ++it )
			set_result( it->first.get(), &results[ it->second ], &error );

		if ( error.good() )
			return std::move( results );
		else return std::move( error );
	}
}
