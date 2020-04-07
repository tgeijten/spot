#pragma once

#include "spot/spot_types.h"
#include "spot/search_point.h"
#include "evaluator.h"
#include <future>

namespace spot
{
	class SPOT_API async_evaluator : public evaluator
	{
	public:
		async_evaluator( int max_threads, xo::thread_priority thread_prio = xo::thread_priority::low );

		virtual vector<result<fitness_t>> evaluate( const objective& o, const search_point_vec& point_vec, priority_t prio = 0 ) override;

		void set_max_threads( int max_threads, xo::thread_priority prio );

	protected:
		std::future< xo::result< fitness_t > > evaluate_async( const objective& o, const search_point& point ) const;
		void set_result( xo::result< fitness_t > result, fitness_t* value, xo::error_message* error ) const;

		int max_threads_;
		xo::thread_priority thread_prio_;
	};
}
