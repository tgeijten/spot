#include "pooled_evaluator.h"

namespace spot
{
	pooled_evaluator::pooled_evaluator( size_t max_threads, xo::thread_priority thread_prio ) :
		max_threads_( max_threads ),
		thread_prio_( thread_prio )
	{}

	vector< result<fitness_t> > pooled_evaluator::evaluate( const objective& o, const search_point_vec& point_vec, priority_t prio )
	{
		// prepare vector of tasks and futures
		vector< std::future< result<fitness_t> > > futures;
		futures.reserve( point_vec.size() );
		vector< eval_task > tasks;
		tasks.reserve( point_vec.size() );
		for ( const auto& point : point_vec )
		{
			tasks.emplace_back( [&]() { return evaluate_noexcept( o, point ); } );
			futures.emplace_back( tasks.back().get_future() );
		}

		{
			// add tasks to end of queue
			std::scoped_lock lock( queue_mutex_ );
			std::move( tasks.begin(), tasks.end(), std::back_inserter( queue_ ) );
		}

		// worker threads are notified after the lock is released
		queue_cv_.notify_all();
		tasks.clear(); // these tasks are moved-out and cleared for clarity

		vector< result<fitness_t> > results;
		results.reserve( point_vec.size() );
		for ( auto& f : futures )
			results.push_back( f.get() );
		return results;
	}
}
