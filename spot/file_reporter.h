#pragma once

#include "xo/container/circular_deque.h"
#include "reporter.h"
#include <fstream>

namespace spot
{
	struct SPOT_API file_reporter : public reporter
	{
	public:
		file_reporter( const path& root_folder, double min_improvement = 0.05, size_t max_steps = 1000 );
		virtual void on_start( const optimizer& opt ) override;
		virtual void on_stop( const optimizer& opt, const stop_condition& s ) override;
		virtual void on_pre_evaluate_population( const optimizer& opt, const search_point_vec& pop ) override;
		virtual void on_post_evaluate_population( const optimizer& opt, const search_point_vec& pop, const fitness_vec& fitnesses, bool new_best ) override;
		virtual void on_new_best( const optimizer& opt, const search_point& point, fitness_t fitness_t ) override;
		virtual void on_post_step( const optimizer& opt ) override;

		double min_improvement_for_file_output_ = 0.05;
		size_t max_steps_without_file_output_ = 1000;
		bool output_temp_files = false;
		bool output_fitness_history_ = true;
		bool output_par_history_ = false;

	private:
		void write_par_file( const optimizer& opt, bool try_cleanup );

		path root_;
		index_t last_output_step;
		xo::circular_deque< pair< path, fitness_t > > recent_files;
		std::ofstream fitness_history_;
		std::ofstream par_history_;
	};
}
