#pragma once

#include "spot/spot_types.h"
#include "xo/utility/result.h"
#include "search_point.h"

namespace spot
{
	class SPOT_API evaluator
	{
	public:
		evaluator() = default;
		virtual ~evaluator() = default;
		virtual xo::result<fitness_vec> evaluate( const objective& o, const search_point_vec& point_vec, priority_t prio = 0 ) const;

	protected:
		xo::result<fitness_t> evaluate_noexcept( const objective& o, const search_point& point ) const noexcept;
	};

	SPOT_API evaluator& global_evaluator();
}
