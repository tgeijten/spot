#pragma once

#include "search_point.h"

#include "flut/system/platform.hpp"

namespace spot
{
	class optimizer;

	class SPOT_API reporter
	{
	public:
		reporter() {}
		virtual ~reporter() {}

		virtual void start_cb( const optimizer& opt ) {}
		virtual void finish_cb( const optimizer& opt ) {}
		virtual void evaluate_cb( const search_point& point, fitness_t fitness_t ) {}
		virtual void evaluate_cb( const search_point_vec& pop, const fitness_vec_t& fitnesses ) {}
		virtual void next_generation_cb( size_t gen ) {}
		virtual void new_best_cb( const search_point& ps, fitness_t fitness ) {}
	};
}
