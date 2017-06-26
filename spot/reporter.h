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

		virtual void start( const optimizer& opt ) {}
		virtual void finish( const optimizer& opt ) {}
		virtual void evaluate( const optimizer& opt, const search_point& point, fitness_t fitness_t ) {}
		virtual void evaluate( const optimizer& opt, const search_point_vec& pop, const fitness_vec_t& fitnesses, bool new_best ) {}
		virtual void next_step( const optimizer& opt, size_t gen ) {}
	};
}
