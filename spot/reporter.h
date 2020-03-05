#pragma once

#include "types.h"
#include "search_point.h"

namespace spot
{
	class optimizer;
	struct stop_condition;

	struct SPOT_API reporter
	{
		reporter() {}
		virtual ~reporter() {}

		virtual void on_start( const optimizer& opt ) {}
		virtual void on_stop( const optimizer& opt, const stop_condition& s ) {}
		virtual void on_post_evaluate_point( const optimizer& opt, const search_point& point, fitness_t fitness_t ) {}
		virtual void on_pre_evaluate_population( const optimizer& opt, const search_point_vec& pop ) {}
		virtual void on_post_evaluate_population( const optimizer& opt, const search_point_vec& pop, const fitness_vec& fitnesses, bool new_best ) {}
		virtual void on_new_best( const optimizer& opt, const search_point& point, fitness_t fitness_t ) {}
		virtual void on_pre_step( const optimizer& opt ) {}
		virtual void on_post_step( const optimizer& opt ) {}
	};
}
