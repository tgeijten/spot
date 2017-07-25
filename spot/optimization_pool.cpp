#include "optimization_pool.h"
#include "flut/math/linear_regression.hpp"
#include "flut/system/log.hpp"

namespace spot
{
	optimization_pool::optimization_pool( size_t promise_window ) : 
	promise_window_( promise_window )
	{}

	void optimization_pool::push_back( u_ptr< optimizer > opt )
	{
		optimizers_.push_back( std::move( opt ) );
		fitness_trackers_.push_back( fitness_tracker( promise_window_ ) );
	}

	void optimization_pool::run( int max_steps )
	{
		for ( int i = 0; i < max_steps && !test_interrupt_flag(); ++i )
			step();
	}

	void optimization_pool::step()
	{
		flut_assert( optimizers_.size() > 0 && fitness_trackers_.size() > 0 );

		while ( step_queue_.size() < 1 )
		{
			// choose best active optimizer
			auto best_indices = sort_indices( fitness_trackers_ );
			for ( auto it = best_indices.begin(); it != best_indices.end() && optimizers_[ *it ]->is_active(); ++it )
			{
				step_queue_.push_back( *it );
				if ( ( it < best_indices.end() - 1 ) && fitness_trackers_[ *it ] < fitness_trackers_[ *( it + 1 ) ] )
					break; // stop if the next one is worse
			}
		}

		// process a single optimizer from the step queue
		auto idx = step_queue_.front();
		optimizers_[ idx ]->step();
		fitness_trackers_[ idx ].update( *optimizers_[ idx ] );
		step_queue_.pop_front();
	}

	void optimization_pool::interrupt() const
	{
		for ( auto& o : optimizers_ )
			o->interrupt();
		interruptible::interrupt();
	}

	void optimization_pool::fitness_tracker::update( const optimizer& opt )
	{
		if ( history_.full() )
			history_.pop_front();
		history_.push_back( float( opt.current_step_best() ) );

		if ( history_.full() )
		{
			float window = static_cast< float >( history_.size() );
			float start = static_cast< float >( opt.current_step() ) - window;

			regression_ = flut::linear_regression( start, 1.0f, history_ );

			// compute progress
			auto scale = regression_( start + 0.5f * window );
			progress_ = opt.info().minimize() ? -regression_.slope() / scale : regression_.slope() / scale;

			// compute promise
			auto steps = intersect_y( regression_, static_cast< float >( opt.info().target_fitness() ) ) - opt.current_step();
			promise_ = steps > 0 ? 1.0f / steps : 0.0f;
		}
	}
}
