#pragma once

#include "spot/types.h"
#include "spot/search_point.h"

namespace spot
{
	class SPOT_API evaluator
	{
	public:
		evaluator() = default;
		virtual ~evaluator() = default;
		virtual fitness_vec evaluate( const objective& o, const search_point_vec& point_vec, priority_t prio = 0 ) const;

	protected:
		fitness_t evaluate_noexcept( const objective& o, const search_point& point ) const noexcept;
		mutable string error_message_;
	};

	class SPOT_API async_evaluator : public evaluator
	{
	public:
		async_evaluator( size_t max_threads, xo::thread_priority thread_prio ) : evaluator(), max_threads_( max_threads ), thread_prio_( thread_prio ) {}
		virtual fitness_vec evaluate( const objective& o, const search_point_vec& point_vec, priority_t prio = 0 ) const override;

	protected:
		std::future< fitness_t > evaluate_async( const objective& o, const search_point& point ) const;
		size_t max_threads_;
		xo::thread_priority thread_prio_;
	};
}
