#include "optimizer_pool.h"
#include "xo/numerical/regression.h"
#include "xo/system/log.h"
#include "stop_condition.h"

namespace spot
{
	struct pool_stop_condition : public stop_condition
	{
		virtual string what() const override { return "All optimizers have ended"; }
		virtual bool test( const optimizer& opt ) override {
			auto& pool = dynamic_cast<const optimizer_pool&>( opt );
			bool stop = true;
			for ( auto& o : pool.optimizers() )
				stop &= o->test_stop_conditions() != nullptr;
			return stop;
		}
	};

	optimizer_pool::optimizer_pool( const objective& o, size_t promise_window, size_t min_steps, size_t max_concurrent_optimizers ) :
	optimizer( o ),
	prediction_window_size_( promise_window ),
	prediction_window_min_size_( min_steps > 1 ? min_steps : promise_window ),
	max_concurrent_optimizations_( max_concurrent_optimizers )
	{
		add_stop_condition< pool_stop_condition >();
	}

	void optimizer_pool::push_back( u_ptr< optimizer > opt )
	{
		opt->enable_fitness_tracking( prediction_window_size_ );
		opt->add_stop_condition< predicted_fitness_condition >( info().worst_fitness(), prediction_window_size_, prediction_window_min_size_ );
		optimizers_.push_back( std::move( opt ) );
	}

	void optimizer_pool::interrupt() const
	{
		for ( auto& o : optimizers_ )
			o->interrupt();
		interruptible::interrupt();
	}

	std::vector< double > optimizer_pool::compute_predicted_fitnesses()
	{
		std::vector< double > priorities;

		size_t active_count = 0;
		for ( auto& o : optimizers_ )
		{
			if ( active_count < max_concurrent_optimizations_ && o->test_stop_conditions() == nullptr )
			{
				if ( o->current_step() >= prediction_window_min_size_ )
					priorities.push_back( o->predicted_fitness( prediction_window_size_ ) );
				else priorities.push_back( info().best_fitness() );
				++active_count;
			}
			else priorities.push_back( info().worst_fitness() );
		}

		return priorities;
	}

	void optimizer_pool::internal_step()
	{
		xo_assert( optimizers_.size() > 0 );

		if ( step_queue_.empty() )
		{
			// choose best active optimizer
			auto predictions = compute_predicted_fitnesses();
			auto best_indices = sort_indices( predictions );
			if ( info().maximize() )
				std::reverse( best_indices.begin(), best_indices.end() );

			for ( auto it = best_indices.begin(); it != best_indices.end() && !optimizers_[ *it ]->test_stop_conditions(); ++it )
			{
				step_queue_.push_back( *it );
				if ( ( it < best_indices.end() - 1 ) && predictions[ *it ] != predictions[ *( it + 1 ) ] )
					break; // stop if the next one is worse
			}

			string str = stringf( "%d (%.0f):", current_step(), best_fitness() );
			for ( int i = 0; i < optimizers_.size(); ++i )
			{
				auto& opt = *optimizers_[ i ];
				auto sc = opt.test_stop_conditions();
				auto bf = xo::clamped( opt.best_fitness(), -9999.0, 9999.0 );
				auto pf = xo::clamped( predictions[ i ], -9999.0, 9999.0 );
				str += stringf( "\t%d/%.0f/%.0f", opt.current_step(), bf, pf );
				//log::info( i, ": step=", opt.step_count(), " best=", opt.best_fitness(), " pred=", predictions[ i ], " ", sc ? sc->what() : "" );
			}
			log::info( str );
		}

		// process a single optimizer from the step queue
		if ( !step_queue_.empty() )
		{
			auto idx = step_queue_.front();
			optimizers_[ idx ]->step();

			// copy results if better
			if ( is_better( optimizers_[ idx ]->best_fitness(), best_fitness_ ) )
			{
				best_fitness_ = optimizers_[ idx ]->best_fitness();
				best_point_ = optimizers_[ idx ]->best_point();

				// update prediction targets for all optimizers
				for ( auto& o : optimizers() )
					o->find_stop_condition< predicted_fitness_condition >().fitness_ = best_fitness_;
			}

			step_queue_.pop_front();
		}
	}
}
