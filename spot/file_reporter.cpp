#include "file_reporter.h"

#include "flut/system_tools.hpp"
#include "optimizer.h"
#include "flut/container_tools.hpp"
#include <fstream>

namespace spot
{
	file_reporter::file_reporter( const path& root_folder ) : root_( root_folder ), last_output_step( -1 )
	{

	}

	void file_reporter::start( const optimizer& opt )
	{
		root_ = flut::create_unique_folder( root_ );
	}

	void file_reporter::finish( const optimizer& opt )
	{

	}

	void file_reporter::evaluate( const optimizer& opt, const search_point_vec& pop, const fitness_vec_t& fitnesses, index_t best_idx, bool new_best )
	{
		if ( new_best || ( opt.current_step() - last_output_step > max_steps_without_file_output ) )
		{
			objective_info updated_info = opt.make_updated_objective_info();
			search_point sp( updated_info, pop[ best_idx ].values() );
			auto best = fitnesses[ best_idx ];
			auto avg = flut::median( fitnesses );
			path filename = root_ / flut::stringf( "%04d_%.3f_%.3f.par", opt.current_step(), avg, best );
			std::ofstream str( filename.str() );
			str << sp;
			last_output_step = opt.current_step();

			if ( new_best )
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
						flut::remove( testIt->first );
						*testIt = *recent_files.begin();
					}
					recent_files.pop_front();
				}
			}
		}
	}
}
