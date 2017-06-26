#pragma once

#include "reporter.h"

namespace spot
{
	class SPOT_API file_reporter : public reporter
	{
	public:
		file_reporter( const path& root_folder );
		virtual ~file_reporter() {}

		virtual void start( const optimizer& opt ) override;
		virtual void finish( const optimizer& opt ) override;
		virtual void evaluate( const optimizer& opt, const search_point& point, fitness_t fitness_t ) override {}
		virtual void evaluate( const optimizer& opt, const search_point_vec& pop, const fitness_vec_t& fitnesses, index_t best_idx, bool new_best ) override;

	private:
		path root_;
		int max_steps_without_file_output = 500;
		int last_output_step;
	};
}
