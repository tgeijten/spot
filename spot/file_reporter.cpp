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
		}
	}
}
