#include "file_reporter.h"

#include <fstream>

#include "xo/system/system_tools.h"
#include "xo/filesystem/filesystem.h"
#include "xo/container/container_tools.h"

#include "optimizer.h"
#include "xo/system/log.h"
#include "xo/container/container_algorithms.h"
#include "xo/numerical/math.h"

namespace spot
{
	file_reporter::file_reporter( const path& root_folder, double min_improvement, size_t max_steps ) :
		root_( root_folder ),
		min_improvement_for_file_output_( min_improvement ),
		max_steps_without_file_output_( max_steps ),
		last_output_step( -1 )
	{}

	void file_reporter::on_start( const optimizer& opt )
	{
		xo::create_directories( root_ );

		// setup history.txt
		if ( output_fitness_history_ )
		{
			history_ = std::ofstream( ( root_ / "history.txt" ).str() );
			history_ << "generation\tbest_fitness\tmedian_fitness";

			if ( opt.fitness_tracking_window_size() > 0 )
				history_ << "\tpredicted_fitness\tfitness_progress";

			// setup par_history.txt
			if ( output_par_history_ )
			{
				for ( auto& pi : opt.obj().info() )
					history_ << '\t' << pi.name;
				for ( auto& l : opt.optimizer_state_labels() )
					history_ << '\t' << l;
			}
			history_ << std::endl;
		}
	}

	void file_reporter::on_stop( const optimizer& opt, const stop_condition& s )
	{
		if ( output_fitness_history_ )
			history_.flush();
	}

	void file_reporter::on_pre_evaluate_population( const optimizer& opt, const search_point_vec& pop )
	{
		if ( output_individual_search_points )
		{
			for ( index_t i = 0; i < pop.size(); ++i )
			{
				path p = root_ / xo::stringf( "%04d_individual%02d.par", opt.current_step(), i );
				std::ofstream( p.str() ) << pop[ i ];
			}
		}
	}

	void file_reporter::on_post_evaluate_population( const optimizer& opt, const search_point_vec& pop, const fitness_vec& fitnesses, bool new_best )
	{
		if ( opt.current_step() - last_output_step >= max_steps_without_file_output_ )
			write_par_file( opt, false );
	}

	void file_reporter::on_new_best( const optimizer& opt, const search_point& sp, fitness_t best )
	{
		write_par_file( opt, true );
	}

	void file_reporter::on_post_step( const optimizer& opt )
	{
		if ( output_fitness_history_ )
		{
			// update history
			auto cur_trend = opt.fitness_trend();
			history_ << opt.current_step() << "\t" << opt.current_step_best_fitness() << "\t" << xo::median( opt.current_step_fitnesses() );

			if ( opt.fitness_tracking_window_size() > 0 )
				history_ << "\t" << opt.predicted_fitness( opt.fitness_tracking_window_size() ) << "\t" << opt.progress();

			// setup par_history.txt
			if ( output_par_history_ )
			{
				history_ << opt.current_step();
				for ( auto&& v : opt.current_step_best_point().values() )
					history_ << '\t' << v;
				for ( auto&& v : opt.optimizer_state_values() )
					history_ << '\t' << v;
			}

			history_ << '\n';
			if ( opt.current_step() % 10 == 9 ) // flush every 10 entries
				history_.flush();
		}
	}

	void file_reporter::write_par_file( const optimizer& opt, bool try_cleanup )
	{
		const fitness_t replim = 999999.999;
		auto best = opt.current_step_best_fitness();
		objective_info updated_info = opt.make_updated_objective_info();
		auto sp = search_point( updated_info, opt.current_step_best_point().values() );
		auto avg = xo::median( opt.current_step_fitnesses() );
		path filename = root_ / xo::stringf( "%04d_%.3f_%.3f.par", opt.current_step(), xo::clamped( avg, -replim, replim ), xo::clamped( best, -replim, replim ) );
		std::ofstream str( filename.str() );
		str << sp;

		if ( try_cleanup )
		{
			recent_files.push_back( std::make_pair( filename, best ) );
			xo_assert( recent_files.size() <= 3 );
			if ( recent_files.size() == 3 )
			{
				// see if we should delete the second last file
				auto& f1 = recent_files[ 0 ].second;
				auto& f2 = recent_files[ 1 ].second;
				auto& f3 = recent_files[ 2 ].second;

				double imp1 = ( f2 - f1 ) / abs( f1 );
				double imp2 = ( f3 - f2 ) / abs( f2 );
				if ( opt.info().minimize() )
					imp1 = -imp1, imp2 = -imp2;

				if ( imp1 < min_improvement_for_file_output_ && imp2 < min_improvement_for_file_output_ )
				{
					//xo::log::info( "Cleaning up file ", recent_files[ 1 ].first );
					xo::remove( recent_files[ 1 ].first );
					recent_files[ 1 ] = recent_files[ 2 ];
					recent_files.pop_back();
				}
				else recent_files.pop_front();
			}
		}

		last_output_step = opt.current_step();
	}
}
