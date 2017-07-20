#include "multi_cma_optimizer.h"
#include <memory>
#include "flut/system/log_sink.hpp"

namespace spot
{
	multi_cma_optimizer::multi_cma_optimizer( const objective& obj, size_t max_solutions, size_t max_searches, double min_distance, int seed ) :
	optimizer( obj ),
	max_solutions_( max_solutions ),
	max_searches_( max_searches ),
	seed_( seed ),
	search_count_( 0 )
	{
		similarity_stop_ = std::make_shared< similarity_condition >( min_distance );
		stop_conditions_.clear();
		stop_conditions_.push_back( std::make_shared< multi_stop_condition >() );
	}

	void multi_cma_optimizer::internal_step()
	{
		if ( optimizers_.empty() || !optimizers_.back()->is_active() )
		{
			// add a new optimizer
			optimizers_.emplace_back( std::make_unique< cma_optimizer >( objective_, 0, seed_ == 0 ? 0 : seed_ + (int)search_count_ ) );
			optimizers_.back()->add_stop_condition( std::make_unique< min_progress_condition >( 100, 1e-6 ) );
			optimizers_.back()->add_stop_condition( std::make_unique< max_steps_condition >( 1000 ) );
			optimizers_.back()->add_stop_condition( similarity_stop_ );
			++search_count_;
		}

		// perform a step with the currently active optimizer
		auto* sc = optimizers_.back()->step();
		auto point = optimizers_.back()->current_step_best_point();
		if ( sc )
		{
			log::info( stringf( "%2d %3d %4d %.3f [%.2f, %.2f]: ", optimizers_.size(), search_count_, optimizers_.back()->current_step(), optimizers_.back()->current_step_best(), point[ 0 ], point[ 1 ] ) + sc->what() );
			if ( sc != similarity_stop_.get() )
			{
				if ( !optimizers_.empty() )
					similarity_stop_->similarity_points.push_back( optimizers_.back()->current_step_best_point().values() );
			}
			else optimizers_.pop_back();

		}
	}
}
