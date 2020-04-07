#pragma once

#include "spot_types.h"
#include "evaluator.h"
#include <future>
#include <mutex>
#include <deque>

namespace spot
{
	class SPOT_API pooled_evaluator : public evaluator
	{
	public:
		pooled_evaluator( int max_threads = 0, xo::thread_priority thread_prio = xo::thread_priority::low );
		virtual ~pooled_evaluator();

		virtual vector< result<fitness_t> > evaluate( const objective& o, const search_point_vec& point_vec, priority_t prio = 0 ) override;

		void set_max_threads( int max_threads, xo::thread_priority prio );

	protected:
		void start_threads();
		void stop_threads();

		void thread_func();

		std::vector< std::thread > threads_;
		std::atomic_bool stop_signal_;

		int max_threads_;
		xo::thread_priority thread_prio_;

		using eval_task = std::packaged_task< xo::result<fitness_t>() >;
		std::mutex queue_mutex_;
		std::condition_variable queue_cv_;
		std::deque< eval_task > queue_;
	};
}
