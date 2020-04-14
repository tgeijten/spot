#include "pooled_evaluator.h"
#include "xo/system/log.h"
#include "objective.h"
#include <iostream>

namespace spot
{
	pooled_evaluator::pooled_evaluator( int max_threads, xo::thread_priority thread_prio ) :
		max_threads_( max_threads ),
		thread_prio_( thread_prio )
	{
		start_threads();
	}

	pooled_evaluator::~pooled_evaluator()
	{
		std::scoped_lock lock( queue_mutex_ );
		if ( !queue_.empty() )
			xo::log::error( "destroying pooled_evaluator with non-empty queue" );
		stop_threads();
	}

	vector< result<fitness_t> > pooled_evaluator::evaluate( const objective& o, const search_point_vec& point_vec, const xo::stop_token& st, priority_t prio )
	{
		// prepare vector of tasks and futures
		vector< std::future< result<fitness_t> > > futures;
		futures.reserve( point_vec.size() );
		vector< eval_task > tasks;
		tasks.reserve( point_vec.size() );
		for ( const auto& point : point_vec )
		{
			tasks.emplace_back( [&]() { return o.evaluate_noexcept( point, st ); } );
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

	void pooled_evaluator::set_max_threads( int thread_count, xo::thread_priority prio )
	{
		if ( max_threads_ != thread_count || thread_prio_ != prio )
		{
			max_threads_ = thread_count;
			thread_prio_ = prio;
			stop_threads();
			start_threads();
		}
	}

	void pooled_evaluator::start_threads()
	{
		if ( !threads_.empty() )
			stop_threads();
		stop_signal_ = false;
		auto thread_count = max_threads_ > 0 ? max_threads_ : std::thread::hardware_concurrency() + max_threads_;
		for ( index_t i = 0; i < thread_count; ++i )
			threads_.emplace_back( &pooled_evaluator::thread_func, this );
		xo::log::debug( "pooled_evaluator started threads: ", thread_count );
	}

	void pooled_evaluator::stop_threads()
	{
		stop_signal_ = true;
		queue_cv_.notify_all();
		while ( !threads_.empty() )
		{
			threads_.back().join();
			threads_.pop_back();
		}
	}

	void pooled_evaluator::thread_func()
	{
		xo::set_thread_priority( thread_prio_ );
		while ( !stop_signal_ )
		{
			eval_task task;
			{
				std::unique_lock lock( queue_mutex_ );
				while ( queue_.empty() )
				{
					queue_cv_.wait( lock );
					if ( stop_signal_ )
						return;
				}
				task = std::move( queue_.front() );
				queue_.pop_front();
			}
			task();
		}
	}
}
