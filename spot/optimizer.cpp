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

namespace spot
{
	optimizer::optimizer( const objective& o ) :
		objective_( o ),
		step_count_( 0 ),
		fitness_history_samples_( 0 ),
		fitness_trend_step_( no_index ),
		max_threads_( xo::max<int>( 4, std::thread::hardware_concurrency() ) ),
		thread_priority_( xo::thread_priority::lowest ),
		stop_condition_( nullptr )
	{
		xo_error_if( o.dim() <= 0, "Objective has no free parameters" );

		add_stop_condition( std::make_unique< abort_condition >() );
		//boundary_transformer_ = std::make_unique< cmaes_boundary_transformer >( o.info() );
	}

	optimizer::~optimizer()
	{
	}

	const stop_condition* optimizer::step()
	{
		// test stop conditions and report finish
		if ( auto* sc = test_stop_conditions() )
			return sc;

		// send out start callback if this is the first step
		if ( step_count_ == 0 )
			signal_reporters( &reporter::on_start, *this );

		// signal reporters
		signal_reporters( &reporter::on_pre_step, *this );

		// perform actual step
		internal_step();

		// update fitness history
		if ( fitness_tracking_window_size() > 0 )
		{
			if ( fitness_history_.full() ) fitness_history_.pop_front();
			fitness_history_.push_back( static_cast<float>( current_step_best_fitness() ) );
			++fitness_history_samples_;
		}

		// signal reporters
		signal_reporters( &reporter::on_post_step, *this );

		++step_count_;

		return nullptr;
	}

	const stop_condition* optimizer::run( size_t number_of_steps )
	{
		if ( number_of_steps == 0 )
			number_of_steps = xo::constants<size_t>::max();

		const stop_condition* sc = nullptr;
		for ( size_t n = 0; n < number_of_steps && !sc; ++n )
			sc = step();
		return sc;
	}

	spot::stop_condition* optimizer::test_stop_conditions()
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

	par_vec& optimizer::boundary_transform( par_vec& v ) const
	{
		if ( boundary_transformer_ )
			boundary_transformer_->apply( v );
		return v;
	}

	fitness_vec optimizer::evaluate( const search_point_vec& point_vec ) const
	{
		return objective_.evaluate_async( point_vec, max_threads_, thread_priority_ );
	}
}
