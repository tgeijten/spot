#pragma once

#include "spot/spot_types.h"
#include "spot/search_point.h"
#include "evaluator.h"
#include <future>

namespace spot
{
	class SPOT_API batch_evaluator : public evaluator
	{
	public:
		batch_evaluator( xo::thread_priority thread_prio = xo::thread_priority::low );
		virtual vector<result<fitness_t>> evaluate( const objective& o, const search_point_vec& point_vec, const xo::stop_token& st, priority_t prio = 0 ) override;
		void set_thread_priority( xo::thread_priority prio );

	protected:
		xo::thread_priority thread_prio_;
	};
}
