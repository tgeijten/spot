#pragma once

#include "spot_types.h"
#include "boundary_transformer.h"
#include "objective.h"
#include "reporter.h"
#include "evaluator.h"
#include "stop_condition.h"

#include "xo/container/circular_deque.h"
#include "xo/container/prop_node.h"
#include "xo/numerical/polynomial.h"
#include "xo/system/log.h"
#include "xo/system/system_tools.h"
#include "xo/utility/interruptible.h"
#include "xo/utility/memory_tools.h"
#include "xo/utility/optional.h"
#include "xo/utility/pointer_types.h"
#include "xo/utility/result.h"

#include <thread>
#include <functional>
#include "xo/utility/stop_token.h"

namespace spot
{
	// #todo: this class is a bit of mess and should be cleaned up
	// Perhaps take step / threading out of this class?
	class SPOT_API optimizer
	{
	public:
		optimizer( const objective& o, evaluator& e );
		optimizer( const optimizer& ) = delete;
		optimizer& operator=( const optimizer& ) = delete;
		virtual ~optimizer();

		const stop_condition* step();
		const stop_condition* run( size_t number_of_steps = 0 );

		virtual stop_condition* test_stop_conditions();
		stop_condition& add_stop_condition( u_ptr<stop_condition> new_sc );
		template< typename T > const T& find_stop_condition() const;
		template< typename T > T& find_stop_condition();

		reporter& add_reporter( u_ptr<reporter> new_rep );

#if !SPOT_EVALUATOR_ENABLED
		void set_max_threads( int val ) { max_threads_ = val; }
		void set_thread_priority( xo::thread_priority tp ) { thread_priority_ = tp; }
#endif

		index_t current_step() const { return step_count_; }

		virtual const fitness_vec& current_step_fitnesses() const = 0;
		virtual fitness_t current_step_best_fitness() const = 0;
		virtual const search_point& current_step_best_point() const = 0;
		virtual fitness_t best_fitness() const = 0;
		virtual const search_point& best_point() const = 0;

		const objective_info& info() const { return objective_.info(); }
		const objective& obj() const { return objective_; }
		bool is_better( fitness_t a, fitness_t b ) const { return objective_.info().is_better( a, b ); }

		// more statistics
		void enable_fitness_tracking( size_t window_size ) { fitness_history_.reserve( window_size ); }
		xo::linear_function< float > fitness_trend() const;
		float progress() const;
		size_t fitness_tracking_window_size() const { return fitness_history_.capacity(); }
		float predicted_fitness( size_t steps_ahead ) const;

		// state
		virtual void save_state( const path& filename ) const { XO_NOT_IMPLEMENTED; }
		virtual objective_info make_updated_objective_info() const { XO_NOT_IMPLEMENTED; }

		// properties
		mutable string name; // #todo: not this, name should be const

		virtual bool interrupt() { return stop_source_.request_stop(); }
		bool stop_requested() const { return stop_source_.stop_requested(); }

	protected:
		virtual stop_condition* internal_step() = 0;
		par_vec& boundary_transform( par_vec& v ) const;
		vector< result<fitness_t> > evaluate( const search_point_vec& point_vec, priority_t prio = 0.0 );

		const objective& objective_;
		evaluator& evaluator_;

		index_t step_count_;

		size_t fitness_history_samples_;
		xo::circular_deque< float > fitness_history_;
		mutable xo::linear_function< float > fitness_trend_;
		mutable index_t fitness_trend_step_;

		vector< u_ptr<reporter> > reporters_;
		vector< u_ptr<stop_condition> > stop_conditions_;
		u_ptr< boundary_transformer > boundary_transformer_;

#if !SPOT_EVALUATOR_ENABLED
		int max_threads_;
		xo::thread_priority thread_priority_;
#endif

		stop_condition* stop_condition_;
		xo::stop_source stop_source_;

		// check if the results have errors and return &error_stop_condition if there are too many errors
		stop_condition* check_results( const vector< result<fitness_t> >& results, int max_errors );
		error_condition error_stop_condition_;

		template< typename T, typename... Args > void signal_reporters( T fn, Args&&... args );
	};

	//
	// Implementation of template functions
	//

	template< typename T, typename... Args >
	void spot::optimizer::signal_reporters( T fn, Args&&... args ) {
		try {
			for ( auto& r : reporters_ )
				std::mem_fn( fn )( *r, std::forward< Args >( args )... );
		}
		catch ( std::exception& e ) {
			xo::log::error( "Error in reporter: ", e.what() );
		}
	}

	template< typename T > T& optimizer::find_stop_condition() {
		for ( auto& s : stop_conditions_ )
			if ( auto sp = dynamic_cast<T*>( s.get() ) )
				return *sp;
		xo_error( "Could not find stop condition" );
	}

	template< typename T > const T& optimizer::find_stop_condition() const {
		for ( auto& s : stop_conditions_ )
			if ( auto sp = dynamic_cast<const T*>( s.get() ) )
				return *sp;
		xo_error( "Could not find stop condition" );
	}
}
