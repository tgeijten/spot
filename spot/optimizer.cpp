#include "optimizer.h"
#include <future>
#include "flut/system/log.hpp"
#include <cmath>
#include "flut/system_tools.hpp"

namespace spot
{
	optimizer::optimizer( const objective& o ) :
	objective_( o ),
	current_fitness_( o.info().worst_fitness() ),
	step_count_( 0 ),
	stop_condition_( no_stop_condition )
	{}

	optimizer::~optimizer()
	{
		abort_and_wait();
	}

	void optimizer::run_threaded()
	{
		abort_flag_ = false;
		background_thread = std::thread( [this]() { flut::set_thread_priority( thread_priority ); this->run(); } );
	}

	optimizer::stop_condition optimizer::run()
	{
		for ( auto cb : reporters_ )
			cb->start( *this );

		step_count_ = 0;
		stop_condition_ = test_stop_condition();

		while ( stop_condition_ == no_stop_condition )
		{
			for ( auto cb : reporters_ )
				cb->next_step( *this, step_count_ );

			step();
			++step_count_;

			stop_condition_ = test_stop_condition();
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

		if ( generation_count() >= max_generations_ )
			return max_steps_reached;

		if ( target_fitness_ && objective_.info().is_better( current_fitness(), *target_fitness_ ) )
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

				// first make sure enough threads are available
				while ( threads.size() >= max_threads() )
				{
					for ( auto it = threads.begin(); it != threads.end(); )
					{
						if ( it->first.wait_for( std::chrono::milliseconds( 1 ) ) == std::future_status::ready )
						{
							// a thread is finished, add it to the results and make room for a new thread
							results[ it->second ] = it->first.get();

							// run callbacks
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

			// run callbacks
			for ( auto cb : reporters_ )
				cb->evaluate( *this, pop, results );

			auto best_idx = objective_.info().find_best_fitness( results );
			if ( results[ best_idx ] > current_fitness_ )
			{
				current_fitness_ = results[ best_idx ];
				for ( auto cb : reporters_ )
					cb->new_best( *this, pop[ best_idx ], results[ best_idx ] );
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
