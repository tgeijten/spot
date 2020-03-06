#pragma once

#include "spot/types.h"
#include "spot/search_point.h"
#include "evaluator.h"

namespace spot
{
	class SPOT_API async_evaluator : public evaluator
	{
	public:
		async_evaluator( size_t max_threads, xo::thread_priority thread_prio );

		virtual xo::result<fitness_vec> evaluate( const objective& o, const search_point_vec& point_vec, priority_t prio = 0 ) const override;

	protected:
		std::future< xo::result< fitness_t > > evaluate_async( const objective& o, const search_point& point ) const;
		void set_result( xo::result< fitness_t > result, fitness_t* value, xo::error_message* error ) const;

		size_t max_threads_;
		xo::thread_priority thread_prio_;
	};
}
