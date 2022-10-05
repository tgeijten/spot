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
#include "xo/utility/memory_tools.h"
#include "xo/utility/optional.h"
#include "xo/utility/pointer_types.h"
#include "xo/utility/result.h"
#include "xo/thread/stop_token.h"
#include "xo/system/profiler.h"
#include "xo/system/profiler_config.h"

namespace spot
{
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

		// optimization info
		index_t current_step() const { return step_count_; }
		virtual const fitness_vec& current_step_fitnesses() const { return current_step_fitnesses_; }
		virtual fitness_t current_step_best_fitness() const { return current_step_best_fitness_; }
		virtual const search_point& current_step_best_point() const { return current_step_best_point_; }
		virtual fitness_t best_fitness() const { return best_fitness_; }
		virtual const search_point& best_point() const { return best_point_; }

		// objective info
		const objective_info& info() const { return objective_.info(); }
		const objective& obj() const { return objective_; }
		bool is_better( fitness_t a, fitness_t b ) const { return objective_.info().is_better( a, b ); }

		// boundary transform
		void set_boundary_transformer( u_ptr<boundary_transformer> bt ) { boundary_transformer_ = std::move( bt ); }

		// fitness tracking and prediction
		void set_fitness_tracking_window_size( size_t window_size ) { fitness_history_.reserve( window_size ); }
		size_t fitness_tracking_window_size() const { return fitness_history_.capacity(); }
		xo::linear_function< float > fitness_trend() const;
		float progress() const;
		float predicted_fitness( size_t steps_ahead ) const;

		// state
		virtual void save_state( const path& filename ) const { XO_NOT_IMPLEMENTED; }
		virtual objective_info make_updated_objective_info() const { XO_NOT_IMPLEMENTED; }
		virtual vector< string > optimizer_state_labels() const { return {}; }
		virtual vector< par_t > optimizer_state_values() const { return {}; }

		// properties
		string name;

		virtual bool interrupt() { return stop_source_.request_stop(); }
		bool stop_requested() const { return stop_source_.stop_requested(); }

		xo::profiler& profiler() { return profiler_; }

	protected:
		virtual bool internal_step() = 0;
		par_vec& try_apply_boundary_transform( par_vec& v ) const;
		vector< result<fitness_t> > evaluate( const search_point_vec& point_vec, priority_t prio = 0 );
		bool evaluate_step( const search_point_vec& point_vec, priority_t prio = 0 );
		bool verify_results( const vector< result<fitness_t> >& results );
		void update_fitness_tracking();

		const objective& objective_;
		evaluator& evaluator_;

		// optimization info
		index_t step_count_;
		fitness_t best_fitness_;
		search_point best_point_;
		fitness_t current_step_best_fitness_;
		fitness_vec current_step_fitnesses_;
		search_point current_step_best_point_;

		// fitness tracking
		size_t fitness_history_samples_;
		xo::circular_deque< float > fitness_history_;
		mutable xo::linear_function< float > fitness_trend_;
		mutable index_t fitness_trend_step_;

		vector< u_ptr<reporter> > reporters_;
		vector< u_ptr<stop_condition> > stop_conditions_;
		u_ptr< boundary_transformer > boundary_transformer_;

		stop_condition* stop_condition_;
		xo::stop_source stop_source_;

		int max_errors_;

		xo::profiler profiler_;

		template< typename T, typename... Args > void signal_reporters( T fn, Args&&... args );
	};

	//
	// Implementation of template functions
	//

	template< typename T, typename... Args >
	void optimizer::signal_reporters( T fn, Args&&... args ) {
		XO_PROFILE_FUNCTION( profiler_ );
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
