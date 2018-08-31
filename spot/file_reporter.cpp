#include "file_reporter.h"

#include <fstream>

#include "xo/system/system_tools.h"
#include "xo/filesystem/filesystem.h"
#include "xo/container/container_tools.h"

#include "optimizer.h"

namespace spot
{
	file_reporter::file_reporter( const path& root_folder ) : root_( root_folder ), last_output_step( -1 )
	{}

	void file_reporter::on_start( const optimizer& opt )
	{
		xo::create_directories( root_ );

		// setup history.txt
		history_ = std::ofstream( ( root_ / "history.txt" ).string() );
		history_ << "Step\tBest\tMedian\tPredicted\tProgress" << std::endl;
	}

	void file_reporter::on_stop( const optimizer& opt, const stop_condition& s )
	{}

	void file_reporter::on_pre_evaluate_population( const optimizer& opt, const search_point_vec& pop )
	{
		if ( output_temp_files )
		{
			// create temp files for debugging purposes
			for ( index_t i = 0; i < pop.size(); ++i )
			{
				path p = root_ / stringf( "%04d_%02d.tmp", opt.current_step(), i );
				std::ofstream( p.str() ) << pop[ i ];
			}
		}
	}

	void file_reporter::on_post_evaluate_population( const optimizer& opt, const search_point_vec& pop, const fitness_vec_t& fitnesses, bool new_best )
	{
		if ( output_temp_files )
		{
			// remove temp files
			for ( index_t i = 0; i < pop.size(); ++i )
			{
				path p = root_ / stringf( "%04d_%02d.tmp", opt.current_step(), i );
				remove( p );
			}
		}

		if ( opt.current_step() - last_output_step >= max_steps_without_file_output )
			write_par_file( opt, false );

		// update history
		auto cur_trend = opt.fitness_trend();
		history_ << opt.current_step()
			<< "\t" << opt.current_step_best_fitness()
			<< "\t" << xo::median( opt.current_step_fitnesses() );

		if ( opt.fitness_tracking_window_size() > 0 )
			history_ << "\t" << opt.predicted_fitness( opt.fitness_tracking_window_size() )
			<< "\t" << opt.progress();

		history_ << "\n";
		if ( opt.current_step() % 10 == 9 ) // flush every 10 entries
			history_.flush();
	}

	void file_reporter::on_new_best( const optimizer& opt, const search_point& sp, fitness_t best )
	{
		write_par_file( opt, true );
	}

	void file_reporter::write_par_file( const optimizer& opt, bool try_cleanup )
	{
		auto best = opt.current_step_best_fitness();
		objective_info updated_info = opt.make_updated_objective_info();
		auto sp = search_point( updated_info, opt.current_step_best_point().values() );
		auto avg = xo::median( opt.current_step_fitnesses() );
		path filename = root_ / xo::stringf( "%04d_%.3f_%.3f.par", opt.current_step(), avg, best );
		std::ofstream str( filename.str() );
		str << sp;

		if ( try_cleanup )
		{
			recent_files.push_back( std::make_pair( filename, best ) );
			if ( recent_files.size() >= 3 )
			{
				// see if we should delete the second last file
				auto testIt = recent_files.end() - 2;
				double imp1 = testIt->second / ( testIt - 1 )->second;
				double imp2 = ( testIt + 1 )->second / testIt->second;
				if ( opt.info().minimize() ) { imp1 = 1.0 / imp1; imp2 = 1.0 / imp2; }

				if ( imp1 < min_improvement_factor_for_file_output && imp2 < min_improvement_factor_for_file_output )
				{
					xo::remove( testIt->first );
					*testIt = *recent_files.begin();
				}
				recent_files.pop_front();
			}
		}

		last_output_step = opt.current_step();
	}
}
