#include "optimizer_pool.h"
#include "xo/numerical/regression.h"
#include "xo/system/log.h"
#include "stop_condition.h"
#include "xo/container/prop_node_tools.h"
#include <future>

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

	optimizer_pool::optimizer_pool( const objective& o, evaluator& e, const prop_node& pn ) :
		optimizer( o, e ),
		INIT_MEMBER( pn, prediction_window_, 100 ),
		INIT_MEMBER( pn, prediction_start_, prediction_window_ ),
		INIT_MEMBER( pn, prediction_look_ahead_, prediction_window_ ),
		INIT_MEMBER( pn, use_predicted_fitness_stop_condition_, true ),
		INIT_MEMBER( pn, active_optimizations_, 6 ),
		INIT_MEMBER( pn, concurrent_optimizations_, 3 ),
		best_fitness_( o.info().worst_fitness() ),
		best_optimizer_idx_( no_index )
	{
		add_stop_condition( std::make_unique< pool_stop_condition >() );
	}

	void optimizer_pool::push_back( u_ptr< optimizer > opt )
	{
		opt->enable_fitness_tracking( prediction_window_ );
		if ( use_predicted_fitness_stop_condition_ )
			opt->add_stop_condition(
				std::make_unique< predicted_fitness_condition >(
				info().worst_fitness(), prediction_look_ahead_, prediction_start_ ) );
		optimizers_.push_back( std::move( opt ) );
	}

	bool optimizer_pool::interrupt()
	{
		for ( auto& o : optimizers_ )
			o->interrupt();
		return optimizer::interrupt();
	}

	objective_info optimizer_pool::make_updated_objective_info() const
	{
		xo_assert( best_optimizer_idx_ != no_index );
		return optimizers_[ best_optimizer_idx_ ]->make_updated_objective_info();
	}

	vector< double > optimizer_pool::compute_predicted_fitnesses()
	{
		vector< double > priorities;

		size_t active_count = 0;
		for ( auto& o : optimizers_ )
		{
			if ( active_count < active_optimizations_ && o->test_stop_conditions() == nullptr )
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
				if ( ( it < best_indices.end() - 1 )
					&& predictions[ *it ] != predictions[ *( it + 1 ) ] // next one is worse
					&& step_queue_.size() >= concurrent_optimizations_ )
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

		vector< std::future< index_t > > futures;
		while ( !step_queue_.empty() && futures.size() < concurrent_optimizations_ )
		{
			futures.push_back( std::async( [&]( index_t i ) { optimizers_[ i ]->step(); return i; }, step_queue_.front() ) );
			step_queue_.pop_front();
		}

		for ( auto& f : futures )
		{
			auto idx = f.get();
			bool new_best = best_optimizer_idx_ == no_index || is_better( optimizers_[ idx ]->best_fitness(), best_fitness_ );
			if ( new_best )
			{
				// copy results if better
				best_optimizer_idx_ = idx;
				best_fitness_ = best_optimizer().best_fitness();

				// update prediction targets for all optimizers
				if ( use_predicted_fitness_stop_condition_ )
					for ( auto& o : optimizers() )
						o->find_stop_condition<predicted_fitness_condition>().fitness_ = best_fitness();

				signal_reporters( &reporter::on_new_best, *this, best_point(), best_fitness() );
			}

			// run post-evaluate callbacks (AFTER current_best is updated!)
			signal_reporters( &reporter::on_post_evaluate_population, *this, search_point_vec(), current_step_fitnesses(), new_best );
		}
	}
}
