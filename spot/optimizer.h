#pragma once

#include "objective.h"
#include "reporter.h"

#include "xo/container/prop_node.h"
#include "xo/utility/types.h"

#include "xo/utility/optional.h"
#include "xo/system/system_tools.h"
#include "xo/container/circular_deque.h"

#include <atomic>
#include <functional>
#include <thread>
#include "stop_condition.h"
#include "xo/utility/interruptible.h"
#include "xo/numerical/polynomial.h"
#include "xo/utility/memory_tools.h"
#include "boundary_transformer.h"
#include "xo/utility/pointer_type.h"

#if defined(_MSC_VER)
#	pragma warning( push )
#	pragma warning( disable: 4251 )
#endif

namespace spot
{
	// TODO: this class is a bit of mess and should be cleaned up
	// Perhaps take step / threading out of this class?
	class SPOT_API optimizer : public xo::interruptible
	{
	public:
		optimizer( const objective& o );
		optimizer( const optimizer& ) = delete;
		optimizer& operator=( const optimizer& ) = delete;
		virtual ~optimizer();

		const stop_condition* step();
		const stop_condition* run( size_t number_of_steps = 0 );

		virtual stop_condition* test_stop_conditions();
		template< typename T, typename... Args > T& add_stop_condition( Args&&... a );
		void add_stop_condition( s_ptr< stop_condition > s ) { stop_conditions_.emplace_back( s ); }
		template< typename T > const T& find_stop_condition() const;
		template< typename T > T& find_stop_condition();

		template< typename T, typename... Args > T& add_reporter( Args&&... a );

		const fitness_vec_t& evaluate( const search_point_vec& pop );

		void set_max_threads( int val ) { max_threads_ = val; }
		void set_thread_priority( thread_priority tp ) { thread_priority_ = tp; }

		index_t step_count() const { return step_count_; }
		fitness_t current_step_median() const;
		fitness_t current_step_average() const;
		fitness_t current_step_best() const { return current_step_best_fitness_; }
		const search_point& current_step_best_point() const { return current_step_best_point_; }

		fitness_t best_fitness() const { return best_fitness_; }
		const search_point& best_point() const { return best_point_; }

		const objective_info& info() const { return objective_.info(); }
		const objective& obj() const { return objective_; }
		bool is_better( fitness_t a, fitness_t b ) const { return objective_.info().is_better( a, b ); }

		// more statistics
		void enable_fitness_tracking( size_t window_size ) { fitness_history_.reserve( window_size ); }
		linear_function< float > fitness_trend() const;
		float progress() const;
		float predicted_fitness( size_t step ) const;

		// state
		virtual void save_state( const path& filename ) const { XO_NOT_IMPLEMENTED; }
		virtual objective_info make_updated_objective_info() const { XO_NOT_IMPLEMENTED; }

		// properties
		mutable string name; // TODO: not this, name should be const

	protected:
		virtual void internal_step() = 0;
		par_vec& boundary_transform( par_vec& v ) const;

		const objective& objective_;

		index_t step_count_;

		fitness_t current_step_best_fitness_;
		fitness_vec_t current_step_fitnesses_;
		search_point current_step_best_point_;

		fitness_t best_fitness_;
		search_point best_point_;

		size_t fitness_history_samples_;
		circular_deque< float > fitness_history_;
		mutable linear_function< float > fitness_trend_;
		mutable index_t fitness_trend_step_;

		std::vector< s_ptr< reporter > > reporters_;
		std::vector< s_ptr< stop_condition > > stop_conditions_;
		u_ptr< boundary_transformer > boundary_transformer_;

		int max_threads_;
		thread_priority thread_priority_;

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
			log::error( "Error in reporter: ", e.what() );
		}
	}

	template< typename T, typename... Args >
	T& spot::optimizer::add_stop_condition( Args&&... a ) {
		stop_conditions_.emplace_back( std::make_unique< T >( std::forward< Args >( a )... ) );
		return static_cast<T&>( *stop_conditions_.back() );
	}

	template< typename T, typename... Args >
	T& spot::optimizer::add_reporter( Args&&... a )
	{
		reporters_.emplace_back( std::make_unique< T >( std::forward< Args >( a )... ) );
		return static_cast<T&>( *reporters_.back() );
	}

	template< typename T > T& spot::optimizer::find_stop_condition() {
		for ( auto& s : stop_conditions_ )
			if ( auto sp = dynamic_cast<T*>( s.get() ) )
				return *sp;
		xo_error( "Could not find stop condition" );
	}

	template< typename T > const T& spot::optimizer::find_stop_condition() const {
		for ( auto& s : stop_conditions_ )
			if ( auto sp = dynamic_cast<const T*>( s.get() ) )
				return *sp;
		xo_error( "Could not find stop condition" );
	}
}

#if defined(_MSC_VER)
#	pragma warning( pop )
#endif
