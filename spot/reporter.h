#pragma once

#include "search_point.h"

#include "xo/system/platform.h"

namespace spot
{
	class optimizer;

	class SPOT_API reporter
	{
	public:
		reporter() {}
		virtual ~reporter() {}

		virtual void start( const optimizer& opt ) {}
		virtual void finish( const optimizer& opt ) {}
		virtual void evaluate_point_start( const optimizer& opt, const search_point& point ) {}
		virtual void evaluate_point_finish( const optimizer& opt, const search_point& point, fitness_t fitness_t ) {}
		virtual void evaluate_population_start( const optimizer& opt, const search_point_vec& pop ) {}
		virtual void evaluate_population_finish( const optimizer& opt, const search_point_vec& pop, const fitness_vec_t& fitnesses, index_t best_idx, bool new_best ) {}
		virtual void new_best( const optimizer& opt, const search_point& point, fitness_t fitness_t ) {}
		virtual void next_step( const optimizer& opt, size_t gen ) {}
	};
}
