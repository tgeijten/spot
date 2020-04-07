#include "async_evaluator.h"

#include "objective.h"

namespace spot
{
	async_evaluator::async_evaluator( int max_threads, xo::thread_priority thread_prio ) :
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

	vector< result<fitness_t> > async_evaluator::evaluate( const objective& o, const search_point_vec& point_vec, priority_t prio )
	{
		vector< result<fitness_t> > results( point_vec.size() );
		vector< pair< std::future< xo::result< fitness_t > >, index_t > > threads;

		auto thread_count = max_threads_ > 0 ? max_threads_ : std::thread::hardware_concurrency() + max_threads_;
		for ( index_t eval_idx = 0; eval_idx < point_vec.size(); ++eval_idx )
		{
			// wait for threads to finish
			while ( threads.size() >= thread_count )
			{
				for ( auto it = threads.begin(); it != threads.end(); )
				{
					if ( it->first.wait_for( std::chrono::milliseconds( 1 ) ) == std::future_status::ready )
					{
						// a thread is finished, set the result
						results[ it->second ] = it->first.get();

						// remove the thread to make room for a new one
						it = threads.erase( it );
					}
					else ++it;
				}
			}

			// add new thread
			threads.push_back( std::make_pair( evaluate_async( o, point_vec[ eval_idx ] ), eval_idx ) );
		}

		// wait for remaining threads
		for ( auto it = threads.begin(); it != threads.end() ; ++it )
			results[ it->second ] = it->first.get();

		return results;
	}

	void async_evaluator::set_max_threads( int max_threads, xo::thread_priority prio )
	{
		max_threads_ = max_threads;
		thread_prio_ = prio;
	}
}
