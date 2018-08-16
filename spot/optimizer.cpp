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
	best_fitness_( o.info().worst_fitness() ),
	best_point_( o.info() ),
	current_step_best_point_( o.info() ),
	step_count_( 0 ),
	current_step_best_fitness_( o.info().worst_fitness() ),
	fitness_history_samples_( 0 ),
	fitness_trend_step_( no_index ),
	max_threads_( xo::max<int>( 4, std::thread::hardware_concurrency() ) ),
	thread_priority_( thread_priority::lowest )
	{
		xo_error_if( o.dim() <= 0, "Objective has no free parameters" );

		add_stop_condition< abort_condition >();
		//boundary_transformer_ = std::make_unique< cmaes_boundary_transformer >( o.info() );
	}

	optimizer::~optimizer()
	{
	}

	const spot::stop_condition* optimizer::step()
	{
		// test stop conditions and report finish
		if ( auto* sc = test_stop_conditions() )
			return sc;

		// send out start callback if this is the first step
		if ( step_count_ == 0 )
			signal_reporters( &reporter::on_start, *this );

		// signal reporters
		signal_reporters( &reporter::on_next_step, *this, step_count_ );

		// perform actual step
		internal_step();
		++step_count_;

		return nullptr;
	}

	const stop_condition* optimizer::run( size_t number_of_steps )
	{
		if ( number_of_steps == 0 )
			number_of_steps = const_max<size_t>();

		const stop_condition* sc = nullptr;
		for ( size_t n = 0; n < number_of_steps && !sc; ++n )
			sc = step();
		return sc;
	}

	spot::stop_condition* optimizer::test_stop_conditions()
	{
		for ( auto& sc : stop_conditions_ )
		{
			if ( sc->test( *this ) )
			{
				signal_reporters( &reporter::on_stop, *this, *sc );
				return sc.get();
			}
		}
		return nullptr;
	}

	const fitness_vec_t& optimizer::evaluate( const search_point_vec& pop )
	{
		// run pre-evaluate callbacks
		signal_reporters( &reporter::on_pre_evaluate_population, *this, pop );

		// compute fitnesses
		current_step_fitnesses_ = objective_.evaluate_async( pop, max_threads_, thread_priority_ );

		// update current step best
		auto best_idx = objective_.info().find_best_fitness( current_step_fitnesses_ );
		current_step_best_fitness_ = current_step_fitnesses_[ best_idx ];
		current_step_best_point_ = pop[ best_idx ];

		// update fitness history
		if ( fitness_history_.capacity() > 0 )
		{
			if ( fitness_history_.full() )
				fitness_history_.pop_front();
			fitness_history_.push_back( static_cast<float>( current_step_best_fitness_ ) );
			++fitness_history_samples_;
		}

		// update all-time best
		bool has_new_best = objective_.info().is_better( current_step_fitnesses_[ best_idx ], best_fitness_ );
		if ( has_new_best )
		{
			best_fitness_ = current_step_fitnesses_[ best_idx ];
			best_point_.set_values( pop[ best_idx ].values() );
			signal_reporters( &reporter::on_new_best, *this, best_point_, best_fitness_ );
		}

		// run post-evaluate callbacks (AFTER current_best is updated!)
		signal_reporters( &reporter::on_post_evaluate_population, *this, pop, current_step_fitnesses_, best_idx, has_new_best );

		return current_step_fitnesses_;
	}

	spot::fitness_t optimizer::current_step_median() const
	{
		return xo::median( current_step_fitnesses_ );
	}

	spot::fitness_t optimizer::current_step_average() const
	{
		return xo::average( current_step_fitnesses_ );
	}

	xo::linear_function< float > optimizer::fitness_trend() const
	{
		if ( fitness_trend_step_ != fitness_history_samples_ )
		{
			if ( fitness_history_.size() >= 2 )
			{
				auto range = make_irange< int >( int( fitness_history_samples_ - fitness_history_.size() ), int( fitness_history_samples_ ) );
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
			auto& reg = fitness_trend();
			auto imp = reg.slope() / reg( fitness_history_samples_ - 0.5f * fitness_history_.size() );
			return info().minimize() ? -imp : imp;
		}
		else return 0.0f;
	}

	float optimizer::predicted_fitness( size_t step ) const
	{
		xo_error_if( fitness_history_.capacity() == 0, "fitness tracking must be enabled for this method" );

		if ( fitness_history_.size() >= 2 )
			return fitness_trend()( static_cast< float >( step ) );
		else return 0.0f;
	}

	par_vec& optimizer::boundary_transform( par_vec& v ) const
	{
		if ( boundary_transformer_ )
			boundary_transformer_->apply( v );
		return v;
	}
}
