#pragma once

#include "reporter.h"
#include "xo/container/container_tools.h"
#include "optimizer.h"

namespace spot
{
	class SPOT_API console_reporter : public reporter
	{
	public:
		console_reporter( int individual_precision = 0, int summary_precision = 2 );
		virtual ~console_reporter() {}
		virtual void evaluate_point_finish( const optimizer& opt, const search_point& point, fitness_t fitness ) override;
		virtual void evaluate_population_finish( const optimizer& opt, const search_point_vec& pop, const fitness_vec_t& fitnesses, index_t best_idx, bool new_best ) override;

		virtual void finish( const optimizer& opt ) override;
		virtual void next_step( const optimizer& opt, size_t gen ) override;
		virtual void start( const optimizer& opt ) override;

	private:
		int individual_precision_;
		int summary_precision_;
	};
}
