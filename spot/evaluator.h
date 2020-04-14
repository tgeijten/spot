#pragma once

#include "spot/spot_types.h"
#include "xo/utility/result.h"
#include "xo/utility/stop_token.h"
#include "search_point.h"

namespace spot
{
	class SPOT_API evaluator
	{
	public:
		evaluator() = default;
		virtual ~evaluator() = default;
		virtual vector< result<fitness_t> > evaluate( const objective& o, const search_point_vec& point_vec, const xo::stop_token& st, priority_t prio = 0 ) = 0;
	};

	class SPOT_API sequential_evaluator : public evaluator
	{
	public:
		virtual vector< result<fitness_t> > evaluate( const objective& o, const search_point_vec& point_vec, const xo::stop_token& st, priority_t prio = 0 ) override;
	};
}
