#include "stop_condition.h"
#include "optimizer.h"
#include "cma_optimizer.h"

namespace spot
{
	bool abort_condition::test( const optimizer& opt )
	{
		return opt.test_interrupt_flag();
	}

	bool flat_fitness_condition::test( const optimizer& opt )
	{
		return opt.current_step() > 0 && xo::equals( opt.current_step_best(), opt.current_step_median(), epsilon_ );
	}

	bool max_steps_condition::test( const optimizer& opt )
	{
		return opt.current_step() >= max_steps_;
	}

	bool min_progress_condition::test( const optimizer& opt )
	{
		if ( opt.current_step() >= progress_window_ )
			return opt.progress() < min_progress_;
		else return false;
	}

	bool similarity_condition::test( const optimizer& opt )
	{
		if ( opt.current_step() >= min_steps_ )
		{
			similarities.resize( similarity_points.size() );
			auto& cma = dynamic_cast< const cma_optimizer& >( opt );
			for ( index_t i = 0; i < similarity_points.size(); ++i )
			{
				std::vector< double > std;
				std.reserve( opt.info().dim() );
				for ( auto& p : cma.info() ) std.push_back( p.std );
				//auto std = cma.info().begin();
				auto point = cma.current_step_best_point().values();
				similarities[ i ] = normalized_distance( point, similarity_points[ i ], std );
				if ( similarities[ i ] < min_distance_ )
				{
					similar_idx = i;
					return true;
				}
			}
		}
		return false;
	}

}
