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
	prediction_window_( promise_window ),
	prediction_start_( min_steps > 1 ? min_steps : promise_window ),
	prediction_look_ahead_( promise_window ),
	concurrent_optimizations_( max_concurrent_optimizers ),
	best_optimizer_idx_( no_index ),
	best_fitness_( o.info().worst_fitness() )
	{
		add_new_stop_condition< pool_stop_condition >();
	}

	void optimizer_pool::push_back( u_ptr< optimizer > opt )
	{
		opt->enable_fitness_tracking( prediction_window_ );
		opt->add_new_stop_condition< predicted_fitness_condition >( info().worst_fitness(), prediction_look_ahead_, prediction_start_ );
		optimizers_.push_back( std::move( opt ) );
	}

	void optimizer_pool::interrupt() const
	{
		for ( auto& o : optimizers_ )
			o->interrupt();
		interruptible::interrupt();
	}

	spot::objective_info optimizer_pool::make_updated_objective_info() const
	{
		xo_assert( best_optimizer_idx_ != no_index );
		return optimizers_[ best_optimizer_idx_ ]->make_updated_objective_info();
	}

	std::vector< double > optimizer_pool::compute_predicted_fitnesses()
	{
		std::vector< double > priorities;

		size_t active_count = 0;
		for ( auto& o : optimizers_ )
		{
			if ( active_count < concurrent_optimizations_ && o->test_stop_conditions() == nullptr )
			{
				if ( o->current_step() >= prediction_start_ )
					priorities.push_back( o->predicted_fitness( prediction_look_ahead_ ) );
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
			auto best_indices = xo::sort_indices( predictions, [&]( fitness_t a, fitness_t b ) { return info().is_better( a, b ); } );

			for ( auto it = best_indices.begin(); it != best_indices.end() && !optimizers_[ *it ]->test_stop_conditions(); ++it )
			{
				step_queue_.push_back( *it );
				if ( ( it < best_indices.end() - 1 ) && predictions[ *it ] != predictions[ *( it + 1 ) ] )
					break; // stop if the next one is worse
			}

			string str = xo::stringf( "%d (%.0f):", current_step(), current_step() > 0 ? best_fitness() : 0.0 );
			for ( int i = 0; i < optimizers_.size(); ++i )
			{
				auto& opt = *optimizers_[ i ];
				auto bf = xo::clamped( opt.best_fitness(), -9999.0, 9999.0 );
				auto pf = xo::clamped( predictions[ i ], -9999.0, 9999.0 );
				str += xo::stringf( "\t%d/%.0f/%.0f", opt.current_step(), bf, pf );
			}
		}

		// process a single optimizer from the step queue
		if ( !step_queue_.empty() )
		{
			auto idx = step_queue_.front();
			optimizers_[ idx ]->step();

			// copy results if better
			bool new_best = best_optimizer_idx_ == no_index || is_better( optimizers_[ idx ]->best_fitness(), best_fitness_ );
			if ( new_best )
			{
				best_optimizer_idx_ = idx;
				best_fitness_ = best_optimizer().best_fitness();

				// update prediction targets for all optimizers
				for ( auto& o : optimizers() )
					o->find_stop_condition< predicted_fitness_condition >().fitness_ = best_fitness();

				signal_reporters( &reporter::on_new_best, *this, best_point(), best_fitness() );
			}

			// run post-evaluate callbacks (AFTER current_best is updated!)
			signal_reporters( &reporter::on_post_evaluate_population, *this, search_point_vec(), current_step_fitnesses(), new_best );

			step_queue_.pop_front();
		}
	}
}
