#include "optimizer.h"
#include <future>
#include "flut/system/log.hpp"
#include <cmath>
#include "flut/system_tools.hpp"
#include "flut/prop_node_tools.hpp"
#include "flut/system/assert.hpp"
#include "flut/math/linear_regression.hpp"
#include "flut/math/polynomial.hpp"

namespace spot
{
	optimizer::optimizer( const objective& o, const prop_node& pn ) :
	objective_( o ),
	best_fitness_( o.info().worst_fitness() ),
	best_point_( o.info() ),
	current_step_best_point_( o.info() ),
	step_count_( 0 ),
	current_step_average_( o.info().worst_fitness() ),
	current_step_best_( o.info().worst_fitness() ),
	current_step_median_( o.info().worst_fitness() ),
	fitness_history_samples_( 0 )
	{
		flut_error_if( o.dim() <= 0, "Objective has no free parameters" );
		INIT_PROP( pn, max_threads, 32 );

		stop_conditions_.push_back( std::make_unique< abort_condition >() );
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
			for ( auto& cb : reporters_ )
				cb->start( *this );

		// signal reporters
		for ( auto& cb : reporters_ )
			cb->next_step( *this, step_count_ );

		// perform actual step
		internal_step();
		++step_count_;

		return nullptr;
	}

	const stop_condition* optimizer::run( size_t number_of_steps )
	{
		if ( number_of_steps == 0 )
			number_of_steps = constants< size_t >::max();

		const stop_condition* sc = nullptr;
		for ( size_t n = 0; n < number_of_steps && !sc; ++n )
			sc = step();
		return sc;
	}

	void optimizer::add_stop_condition( s_ptr< stop_condition > cb )
	{
		for ( auto& sc : stop_conditions_ )
			flut_error_if( sc->what() == cb->what(), "there already is a stop_condition of the same type" );

		stop_conditions_.emplace_back( cb );
	}

	spot::stop_condition* optimizer::test_stop_conditions()
	{
		for ( auto& sc : stop_conditions_ )
		{
			if ( sc->test( *this ) )
			{
				for ( auto& cb : reporters_ )
					cb->finish( *this );
				return sc.get();
			}
		}
		return nullptr;
	}

	fitness_vec_t optimizer::evaluate( const search_point_vec& pop )
	{
		vector< double > results( pop.size(), objective_.info().worst_fitness() );
		try
		{
			vector< std::pair< std::future< double >, index_t > > threads;

			for ( index_t eval_idx = 0; eval_idx < pop.size(); ++eval_idx )
			{
				if ( test_interrupt_flag() )
					break;

				// wait for threads to finish
				while ( threads.size() >= max_threads )
				{
					for ( auto it = threads.begin(); it != threads.end(); )
					{
						if ( it->first.wait_for( std::chrono::milliseconds( 1 ) ) == std::future_status::ready )
						{
							// a thread is finished, add it to the results and make room for a new thread
							results[ it->second ] = it->first.get();
							for ( auto& cb : reporters_ )
								cb->evaluate( *this, pop[ it->second ], results[ it->second ] );
							it = threads.erase( it );
						}
						else ++it;
					}
				}

				// add new thread
				threads.push_back( std::make_pair( std::async( std::launch::async, [&]( const search_point& p ) { return objective_.evaluate( p ); }, pop[ eval_idx ] ), eval_idx ) );
			}

			// wait for remaining threads
			for ( auto& f : threads )
			{
				results[ f.second ] = f.first.valid() ? f.first.get() : objective_.info().worst_fitness();

				// run callbacks
				for ( auto& cb : reporters_ )
					cb->evaluate( *this, pop[ f.second ], results[ f.second ] );
			}

			auto best_idx = objective_.info().find_best_fitness( results );
			bool new_best = objective_.info().is_better( results[ best_idx ], best_fitness_ );
			if ( new_best )
			{
				best_fitness_ = results[ best_idx ];
				best_point_.set_values( pop[ best_idx ].values() );
			}

			// update current mean, avg and best
			current_step_median_ = median( results );
			current_step_average_ = average( results );
			current_step_best_ = results[ best_idx ];
			current_step_best_point_ = pop[ best_idx ];

			// update fitness history
			if ( fitness_history_.capacity() > 0 )
			{
				if ( fitness_history_.full() )
					fitness_history_.pop_front();
				fitness_history_.push_back( static_cast< float >( current_step_best_ ) );
				++fitness_history_samples_;
			}

			// run callbacks (AFTER current_best is updated!)
			for ( auto& cb : reporters_ )
			{
				cb->evaluate( *this, pop, results, best_idx, new_best );
				if ( new_best )
					cb->new_best( *this, best_point_, best_fitness_ );
			}
		}
		catch ( std::exception& e )
		{
			log::critical( "Error during multi-threaded evaluation: ", e.what() );
		}

		return results;
	}

	float optimizer::progress() const
	{
		flut_error_if( fitness_history_.capacity() == 0, "fitness tracking must be enabled for this method" );
		if ( fitness_history_.size() >= 2 )
		{
			auto reg = flut::linear_regression( static_cast< float >( fitness_history_samples_ - fitness_history_.size() ), 1.0f, fitness_history_ );
			auto slope = reg.slope() / reg( fitness_history_samples_  - 0.5f * fitness_history_.size() );
			return info().minimize() ? -slope : slope;
		}
		else return 1.0f;
	}

	float optimizer::promise() const
	{
		flut_error_if( fitness_history_.capacity() == 0, "fitness tracking must be enabled for this method" );

		if ( fitness_history_.size() >= 2 )
		{
			auto reg = flut::linear_regression( float( fitness_history_samples_ - fitness_history_.size() ), 1.0f, fitness_history_ );
			auto steps_to_target = flut::intersect_y( reg, float( info().target_fitness() ) ) - fitness_history_samples_ ;

			//if ( steps_to_target <= 0 )
			//	log::infof( "steps_to_target < 0, slope=%.6f offset=%.6f samples=%d hist=%d", reg.slope(), reg.offset(), fitness_history_samples_, fitness_history_.size() );
			if ( steps_to_target >= 1.0f )
				return 1.0f / steps_to_target;
			else return info().minimize() == ( reg.slope() < 0 ) ? 1.0f : 0.0f; // return 0 or 1 depending on slope sign
		}
		else return 1.0f;
	}
}
