#include "optimizer.h"

#include <future>
#include <cmath>

#include "xo/container/prop_node_tools.h"
#include "xo/numerical/polynomial.h"
#include "xo/numerical/regression.h"
#include "xo/system/assert.h"
#include "xo/system/log.h"
#include "xo/system/system_tools.h"
#include "xo/utility/irange.h"
#include "xo/container/flat_set.h"
#include "xo/container/view_if.h"
#include "xo/system/profiler_config.h"
#include "xo/container/container_tools.h"

namespace spot
{
	optimizer::optimizer( const objective& o, evaluator& e ) :
		objective_( o ),
		evaluator_( e ),
		step_count_( 0 ),
		best_fitness_( o.info().worst_fitness() ),
		best_point_( o.info() ),
		current_step_best_fitness_( o.info().worst_fitness() ),
		current_step_best_point_( o.info() ),
		fitness_history_samples_( 0 ),
		fitness_trend_step_( no_index ),
		stop_condition_( nullptr ),
		max_errors_( 0 )
	{
		xo_error_if( o.dim() <= 0, "Objective has no free parameters" );

		add_stop_condition( std::make_unique< abort_condition >() );
		add_stop_condition( std::make_unique< error_condition >() );
		//boundary_transformer_ = std::make_unique< cmaes_boundary_transformer >( o.info() );
	}

	optimizer::~optimizer()
	{}

	const stop_condition* optimizer::step()
	{
		XO_PROFILE_FUNCTION( profiler_ );

		// send out start callback if this is the first step
		if ( step_count_ == 0 )
			signal_reporters( &reporter::on_start, *this );

		// signal reporters
		signal_reporters( &reporter::on_pre_step, *this );

		// perform actual step
		if ( internal_step() )
		{
			signal_reporters( &reporter::on_post_step, *this );
			++step_count_;
		}

		// test stop conditions (e.g. error, abort, min_progress, flat_fitness)
		return test_stop_conditions();
	}

	const stop_condition* optimizer::run( size_t number_of_steps )
	{
		profiler_.start();

		if ( number_of_steps == 0 )
			number_of_steps = xo::constants<size_t>::max();

		for ( size_t n = 0; n < number_of_steps; ++n )
			if ( const stop_condition* sc = step() )
				return sc;

		return nullptr;
	}

	stop_condition* optimizer::test_stop_conditions()
	{
		if ( stop_condition_ )
			return stop_condition_; // already stopped and signaled

		for ( auto& sc : stop_conditions_ )
		{
			if ( sc->test( *this ) )
			{
				stop_condition_ = sc.get();
				signal_reporters( &reporter::on_stop, *this, *sc );
				return stop_condition_;
			}
		}
		return nullptr;
	}

	stop_condition& optimizer::add_stop_condition( u_ptr<stop_condition> new_sc )
	{
		if ( new_sc->minimum_fitness_tracking_window_size() > fitness_tracking_window_size() )
			set_fitness_tracking_window_size( new_sc->minimum_fitness_tracking_window_size() );
		return *stop_conditions_.emplace_back( std::move( new_sc ) );
	}

	reporter& optimizer::add_reporter( u_ptr<reporter> new_rep )
	{
		return *reporters_.emplace_back( std::move( new_rep ) );
	}

	xo::linear_function< float > optimizer::fitness_trend() const
	{
		if ( fitness_trend_step_ != fitness_history_samples_ )
		{
			if ( fitness_history_.size() >= 2 )
			{
				auto range = xo::make_irange< int >( int( fitness_history_samples_ - fitness_history_.size() ), int( fitness_history_samples_ ) );
				//auto start = fitness_history_samples_ - fitness_history_.size();
				fitness_trend_ = xo::repeated_median_regression( range.begin(), range.end(), fitness_history_.begin(), fitness_history_.end() );
			}
			else fitness_trend_ = xo::linear_function< float >();

			fitness_trend_step_ = fitness_history_samples_;
		}

		return fitness_trend_;
	}

	float optimizer::progress() const
	{
		xo_error_if( fitness_history_.capacity() == 0, "fitness tracking must be enabled for this method" );
		if ( fitness_history_.size() >= 2 )
		{
			const auto reg = fitness_trend();
			auto imp = reg.slope() / reg( fitness_history_samples_ - 0.5f * fitness_history_.size() );
			return info().minimize() ? -imp : imp;
		}
		else return 0.0f;
	}

	float optimizer::predicted_fitness( size_t steps_ahead ) const
	{
		xo_error_if( fitness_history_.capacity() == 0, "fitness tracking must be enabled for this method" );

		if ( fitness_history_.size() >= 2 )
			return fitness_trend()( static_cast<float>( current_step() + steps_ahead ) );
		else return 0.0f;
	}

	par_vec& optimizer::try_apply_boundary_transform( par_vec& v ) const
	{
		if ( boundary_transformer_ )
			boundary_transformer_->apply( v );
		return v;
	}

	vector< result<fitness_t> > optimizer::evaluate( const search_point_vec& point_vec, priority_t prio )
	{
		XO_PROFILE_FUNCTION( profiler_ );
		return evaluator_.evaluate( objective_, point_vec, stop_source_.get_token(), prio );
	}

	bool optimizer::evaluate_step( const search_point_vec& point_vec, priority_t prio )
	{
		// run callbacks
		signal_reporters( &reporter::on_pre_evaluate_population, *this, point_vec );

		// compute fitnesses
		auto results = evaluate( point_vec );

		// stop if there were too many errors
		if ( verify_results( results ) )
		{
			// copy results
			current_step_fitnesses_.resize( results.size() );
			for ( index_t i = 0; i < results.size(); ++i )
				current_step_fitnesses_[ i ] = results[ i ] ? results[ i ].value() : info().worst_fitness();

			// update current step best
			auto best_idx = objective_.info().find_best_fitness( current_step_fitnesses_ );
			current_step_best_fitness_ = current_step_fitnesses_[ best_idx ];
			current_step_best_point_ = point_vec[ best_idx ];

			// update all-time best
			bool has_new_best = objective_.info().is_better( current_step_fitnesses_[ best_idx ], best_fitness_ );
			if ( has_new_best )
			{
				best_fitness_ = current_step_fitnesses_[ best_idx ];
				best_point_.set_values( point_vec[ best_idx ].values() );
				signal_reporters( &reporter::on_new_best, *this, best_point_, best_fitness_ );
			}

			// update fitness tracking
			update_fitness_tracking();

			// run post-evaluate callbacks (AFTER current_best is updated!)
			signal_reporters( &reporter::on_post_evaluate_population, *this, point_vec, current_step_fitnesses_, has_new_best );

			return true;
		}
		else return false;
	}

	bool optimizer::verify_results( const vector< result<fitness_t> >& results )
	{
		const auto errors = xo::count_if( results, []( const auto& r ) { return !r; } );
		const auto max_errors = max_errors_ >= 0 ? max_errors_ : int( results.size() ) + max_errors_;
		if ( errors > max_errors )
		{
			// collect error messages
			xo::flat_set<string> messages;
			for ( const auto& r : results )
				if ( !r ) messages.insert( r.error().message() );
			if ( messages.empty() )
				messages.insert( "Unknown error" );

			auto err_message = xo::stringf( "Errors found in %zu of %zu evaluations (%d allowed):\n", errors, results.size(), max_errors );
			find_stop_condition<error_condition>().set( err_message + xo::to_str( messages ) );
			return false;
		}
		else return true;
	}

	void optimizer::update_fitness_tracking()
	{
		// update fitness history
		if ( fitness_tracking_window_size() > 0 )
		{
			if ( fitness_history_.full() ) fitness_history_.pop_front();
			fitness_history_.push_back( static_cast<float>( current_step_best_fitness() ) );
			++fitness_history_samples_;
		}
	}
}
