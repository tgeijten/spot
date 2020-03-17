#pragma once

#include "reporter.h"

namespace spot
{
	struct SPOT_API console_reporter : public reporter
	{
		console_reporter( int individual_precision = 0, int summary_precision = 2 );

		virtual void on_post_evaluate_point( const optimizer& opt, const search_point& point, fitness_t fitness ) override;
		virtual void on_post_evaluate_population( const optimizer& opt, const search_point_vec& pop, const fitness_vec& fitnesses, bool new_best ) override;

		virtual void on_stop( const optimizer& opt, const stop_condition& s ) override;
		virtual void on_pre_step( const optimizer& opt ) override;
		virtual void on_start( const optimizer& opt ) override;

	private:
		int individual_precision_;
		int summary_precision_;
	};
}
