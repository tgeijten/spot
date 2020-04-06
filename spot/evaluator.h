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
		virtual vector< result<fitness_t> > evaluate( const objective& o, const search_point_vec& point_vec, priority_t prio = 0 ) = 0;

	protected:
		static result<fitness_t> evaluate_noexcept( const objective& o, const search_point& point ) noexcept;
	};

	class SPOT_API sequential_evaluator : public evaluator
	{
	public:
		virtual vector< result<fitness_t> > evaluate( const objective& o, const search_point_vec& point_vec, priority_t prio = 0 );
	};
}
