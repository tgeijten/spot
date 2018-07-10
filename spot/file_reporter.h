#pragma once

#include "xo/container/circular_deque.h"
#include "reporter.h"
#include <iosfwd>

#if defined(_MSC_VER)
#	pragma warning( push )
#	pragma warning( disable: 4251 )
#endif

namespace spot
{
	struct SPOT_API file_reporter : public reporter
	{
	public:
		file_reporter( const path& root_folder );
		virtual void on_start( const optimizer& opt ) override;
		virtual void on_finish( const optimizer& opt ) override;
		virtual void on_pre_evaluate_population( const optimizer& opt, const search_point_vec& pop ) override;
		virtual void on_post_evaluate_population( const optimizer& opt, const search_point_vec& pop, const fitness_vec_t& fitnesses, index_t best_idx, bool new_best ) override;

	private:
		path root_;
		size_t max_steps_without_file_output = 500;
		double min_improvement_factor_for_file_output = 1.05;
		index_t last_output_step;
		circular_deque< std::pair< path, fitness_t > > recent_files;
		std::ofstream history_;
	};
}

#if defined(_MSC_VER)
#	pragma warning( pop )
#endif
