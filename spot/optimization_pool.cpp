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
		// log promises
		string str = "promises: ";
		for ( auto& p : fitness_trackers_ )
			str += stringf( "\t%.4f/%.4f", p.promise_, p.progress_ );
		log::info( str );

		// choose best optimizer and update promise
		// TODO: give equal promises equal chances (especially when promise = 0)
		index_t idx = max_element( fitness_trackers_ ) - fitness_trackers_.begin();
		if ( optimizers_[ idx ]->is_active() )
		{
			optimizers_[ idx ]->step();
			fitness_trackers_[ idx ].update( *optimizers_[ idx ] );
		}
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
