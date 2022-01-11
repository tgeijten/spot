#include "stop_condition.h"
#include "optimizer.h"
#include "cma_optimizer.h"
#include "math_tools.h"
#include "xo/container/container_algorithms.h"

namespace spot
{
	bool abort_condition::test( const optimizer& opt )
	{
		return opt.stop_requested();
	}

	bool flat_fitness_condition::test( const optimizer& opt )
	{
		if ( opt.current_step() > 0 )
		{
			auto highest = *xo::max_element( opt.current_step_fitnesses() );
			auto lowest = *xo::min_element( opt.current_step_fitnesses() );
			return highest - lowest < epsilon_;
		}
		else return false;
	}

	bool target_fitness_condition::test( const optimizer& opt )
	{
		return opt.is_better( opt.best_fitness(), target_ );
	}

	bool max_steps_condition::test( const optimizer& opt )
	{
		return opt.current_step() >= max_steps_;
	}

	bool min_progress_condition::test( const optimizer& opt )
	{
		if ( opt.current_step() >= min_samples_ )
			return opt.progress() < min_progress_;
		else return false;
	}

	xo::string predicted_fitness_condition::what() const
	{
		return xo::stringf( "Predicted fitness %f is worse than %f", prediction_, fitness_ );
	}

	bool predicted_fitness_condition::test( const optimizer& opt )
	{
		if ( opt.current_step() >= min_samples_ )
		{
			prediction_ = opt.predicted_fitness( look_ahead_ );
			return !opt.info().is_better( prediction_, fitness_ );
		}
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
				par_vec std;
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
