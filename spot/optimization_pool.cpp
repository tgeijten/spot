#include "optimization_pool.h"
#include "flut/math/linear_regression.hpp"
#include "flut/system/log.hpp"

namespace spot
{
	optimization_pool::optimization_pool( size_t promise_window, size_t min_window_size ) : 
	max_window_size_( promise_window ),
	min_window_size_( min_window_size > 1 ? min_window_size : promise_window )
	{}

	void optimization_pool::push_back( u_ptr< optimizer > opt )
	{
		opt->enable_fitness_tracking( max_window_size_ );
		optimizers_.push_back( std::move( opt ) );
	}

	void optimization_pool::run( int max_steps )
	{
		bool keep_going = true;
		for ( int i = 0; i < max_steps && keep_going; ++i )
			keep_going = !step() && !test_interrupt_flag();
	}

	bool optimization_pool::step()
	{
		flut_assert( optimizers_.size() > 0 );
		if ( step_queue_.size() < 1 )
		{
			// choose best active optimizer
			vector< float > promises;
			for ( auto& o : optimizers_ )
				promises.push_back( o->current_step() < min_window_size_ ? 1.0f : o->predicted_fitness( 1000 ) );

			auto best_indices = sort_indices( promises );
			std::reverse( best_indices.begin(), best_indices.end() );
			for ( auto it = best_indices.begin(); it != best_indices.end() && !optimizers_[ *it ]->test_stop_conditions(); ++it )
			{
				step_queue_.push_back( *it );
				if ( ( it < best_indices.end() - 1 ) && promises[ *it ] != promises[ *( it + 1 ) ] )
					break; // stop if the next one is worse
			}

			//string str = "Promises:";
			//for ( int i = 0; i < optimizers_.size(); ++i )
			//	str += stringf( "\t%d=%.5f", best_indices[ i ], promises[ best_indices[ i ] ] );
			//log::info( str );
		}

		// process a single optimizer from the step queue
		if ( !step_queue_.empty() )
		{
			auto idx = step_queue_.front();
			optimizers_[ idx ]->step();
			step_queue_.pop_front();
			return false;
		}
		else return true;
	}

	void optimization_pool::interrupt() const
	{
		for ( auto& o : optimizers_ )
			o->interrupt();
		interruptible::interrupt();
	}
}
