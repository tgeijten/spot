#include "optimizer.h"
#include <future>
#include "xo/system/log.h"
#include <cmath>
#include "xo/system/system_tools.h"
#include "xo/serialization/prop_node_tools.h"
#include "xo/system/assert.h"
#include "xo/numerical/linear_regression.h"
#include "xo/numerical/polynomial.h"

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
	fitness_history_samples_( 0 ),
	thread_priority_( thread_priority::lowest )
	{
		xo_error_if( o.dim() <= 0, "Objective has no free parameters" );
		INIT_PROP( pn, max_threads, XO_IS_DEBUG_BUILD ? 1 : 32 );
		INIT_PROP( pn, thread_priority_, thread_priority::lowest );
		INIT_PROP( pn, fitness_history_bin_size_, 10 );

		add_stop_condition< abort_condition >();
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
		try
		{
			for ( auto& cb : reporters_ )
				cb->evaluate_population_start( *this, pop );

			vector< double > results( pop.size(), objective_.info().worst_fitness() );
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
								cb->evaluate_point_finish( *this, pop[ it->second ], results[ it->second ] );
							it = threads.erase( it );
						}
						else ++it;
					}
				}

				// add new thread
				threads.push_back( std::make_pair( std::async( std::launch::async, [&]( const search_point& p ) { set_thread_priority( thread_priority_ ); return objective_.evaluate( p ); }, pop[ eval_idx ] ), eval_idx ) );
			}

			// wait for remaining threads
			for ( auto& f : threads )
			{
				results[ f.second ] = f.first.valid() ? f.first.get() : objective_.info().worst_fitness();

				// run callbacks
				for ( auto& cb : reporters_ )
					cb->evaluate_point_finish( *this, pop[ f.second ], results[ f.second ] );
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
			current_step_average_ = top_average( results, pop.size() / 2 );
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
				cb->evaluate_population_finish( *this, pop, results, best_idx, new_best );
				if ( new_best )
					cb->new_best( *this, best_point_, best_fitness_ );
			}

			return results;
		}
		catch ( std::exception& e )
		{
			log::critical( "Error during multi-threaded evaluation: ", e.what() );
			return vector< double >( pop.size(), objective_.info().worst_fitness() );
		}
	}

	xo::linear_function< float > optimizer::fitness_trend() const
	{
		if ( fitness_history_.size() >= 2 * fitness_history_bin_size_ )
		{
			std::vector< float > values( fitness_history_.size() / fitness_history_bin_size_, objective().info().worst< float >() );
			index_t start_idx = fitness_history_.size() % fitness_history_bin_size_;

			for ( index_t i = start_idx; i < fitness_history_.size(); ++i )
			{
				auto& cur_best = values[ ( i - start_idx ) / fitness_history_bin_size_ ];
				if ( info().is_better( fitness_history_[ i ], cur_best ) )
					cur_best = fitness_history_[ i ];
			}

			auto start = start_idx + ( fitness_history_samples_ - fitness_history_.size() ) + fitness_history_bin_size_ / 2;
			return xo::linear_regression( float( start ), float( fitness_history_bin_size_ ), values );
		}
		else return xo::linear_function< float >();
	}

	float optimizer::progress() const
	{
		xo_error_if( fitness_history_.capacity() == 0, "fitness tracking must be enabled for this method" );
		if ( fitness_history_.size() >= 2 )
		{
			auto& reg = fitness_trend();
			auto slope = reg.slope() / reg( fitness_history_samples_ - 0.5f * fitness_history_.size() );
			return info().minimize() ? -slope : slope;
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
}
