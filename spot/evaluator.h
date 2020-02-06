#pragma once

#include <future>

#include "spot/types.h"
#include "spot/search_point.h"

namespace spot
{
	class SPOT_API evaluator
	{
	public:
		evaluator() {}
		virtual ~evaluator() {}

		virtual std::future<fitness_t> evaluate( const objective& o, const search_point& p );
		virtual vector<std::future<fitness_t>> evaluate( const objective& o, const search_point_vec& point_vec );
	};
}
