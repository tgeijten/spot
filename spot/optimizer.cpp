#include "optimizer.h"
#include <future>
#include "flut/system/log.hpp"
#include <cmath>
#include "flut/system_tools.hpp"
#include "flut/prop_node_tools.hpp"

namespace spot
{
	optimizer::optimizer( const objective& o, const prop_node& pn ) :
	objective_( o ),
	current_best_fitness_( o.info().worst_fitness() ),
	current_best_( o.info() ),
	current_step_( 0 ),
	stop_condition_( no_stop_condition )
	{
		flut_error_if( o.dim() <= 0, "There are no parameters to optimize" );

		INIT_PROP( pn, max_threads, 32 );
		INIT_PROP( pn, max_steps, 10000 );
	}

	optimizer::~optimizer()
	{
		abort_and_wait();
	}

	void optimizer::run_threaded()
	{
		abort_flag_ = false;
		background_thread = std::thread( [this]() { flut::set_thread_priority( thread_priority ); this->run(); } );
	}

	optimizer::stop_condition optimizer::run( size_t number_of_steps )
	{
		if ( number_of_steps == 0 )
			number_of_steps = max_steps;

		for ( auto cb : reporters_ )
			cb->start( *this );


		for ( size_t n = 0; n < number_of_steps; ++n )
		{
			stop_condition_ = test_stop_condition();
			if ( stop_condition_ != no_stop_condition )
				break;

			for ( auto cb : reporters_ )
				cb->next_step( *this, current_step_ );

			step();
			++current_step_;
		}

		for ( auto cb : reporters_ )
			cb->finish( *this );

		return stop_condition_;
	}

	void optimizer::abort_and_wait()
	{
		if ( background_thread.joinable() )
		{
			signal_abort();
			background_thread.join();
		}
	}

	optimizer::stop_condition optimizer::test_stop_condition() const
	{
		if ( test_abort() )
			return user_abort;

		if ( current_step() >= max_steps )
			return max_steps_reached;

		if ( target_fitness_ && objective_.info().is_better( best_fitness(), *target_fitness_ ) )
			return target_fitness_reached;

		// none of the criteria is met -> return false
		return no_stop_condition;
	}

	fitness_vec_t optimizer::evaluate( const search_point_vec& pop )
	{
		vector< double > results( pop.size(), objective_.info().worst_fitness() );
		try
		{
			vector< std::pair< std::future< double >, index_t > > threads;

			for ( index_t eval_idx = 0; eval_idx < pop.size(); ++eval_idx )
			{
				if ( abort_flag_.load() )
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
							for ( auto cb : reporters_ )
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
				for ( auto cb : reporters_ )
					cb->evaluate( *this, pop[ f.second ], results[ f.second ] );
			}

			auto best_idx = objective_.info().find_best_fitness( results );
			bool new_best = objective_.info().is_better( results[ best_idx ], current_best_fitness_ );
			if ( new_best )
			{
				current_best_fitness_ = results[ best_idx ];
				current_best_.set_values( pop[ best_idx ].values() );
			}

			// run callbacks (AFTER current_best is updated!)
			for ( auto cb : reporters_ )
			{
				cb->evaluate( *this, pop, results, best_idx, new_best );
				if ( new_best )
					cb->new_best( *this, current_best_, current_best_fitness_ );
			}
		}
		catch ( std::exception& e )
		{
			log::critical( "Error during multi-threaded evaluation: ", e.what() );
		}

		return results;
	}

	void optimizer::set_min_progress( fitness_t relative_improvement_per_step, size_t window )
	{
		min_progress_ = relative_improvement_per_step;
		progress_window.reserve( window );
	}

	void optimizer::update_progress( fitness_t current_median )
	{
		if ( progress_window.capacity() > 0 )
		{
			if ( progress_window.size() == progress_window.capacity() )
				progress_window.pop_front();
			progress_window.push_back( current_median );
		}
	}
}
