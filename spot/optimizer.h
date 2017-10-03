#pragma once

#include "objective.h"
#include "reporter.h"

#include "flut/prop_node.hpp"
#include "flut/system/types.hpp"

#include "flut/math/math.hpp"
#include "flut/math/optional.hpp"
#include "flut/system_tools.hpp"
#include "flut/circular_deque.hpp"

#include <atomic>
#include <functional>
#include <thread>
#include "stop_condition.h"
#include "flut/interruptible.hpp"

#if defined(_MSC_VER)
#	pragma warning( push )
#	pragma warning( disable: 4251 )
#endif

namespace spot
{
	// TODO: this class is a bit of mess and should be cleaned up
	// Perhaps take step / threading out of this class?
	// make interface for stop conditions cleaner
	class SPOT_API optimizer : public flut::interruptible
	{
	public:
		optimizer( const objective& o, const prop_node& pn = prop_node() );
		optimizer( const optimizer& ) = delete;
		optimizer& operator=( const optimizer& ) = delete;
		virtual ~optimizer();

		const stop_condition* step();
		const stop_condition* run( size_t number_of_steps = 0 );

		stop_condition& add_stop_condition( u_ptr< stop_condition > cb );
		stop_condition* test_stop_conditions();

		template< typename T > T& get_stop_condition() { 
			for ( auto& c : stop_conditions_ )
				if ( auto* p = dynamic_cast<T*>( c.get() ) )
					return *p;
			return dynamic_cast<T&>( add_stop_condition( u_ptr< stop_condition >( new T() ) ) );
		}

		void add_reporter( s_ptr< reporter > cb ) { reporters_.emplace_back( std::move( cb ) ); }
		fitness_vec_t evaluate( const search_point_vec& pop );
		void set_max_threads( int val ) { max_threads = val; }
		index_t current_step() const { return step_count_; }
		fitness_t current_step_median() const { return current_step_median_; }
		fitness_t current_step_average() const { return current_step_average_; }
		fitness_t current_step_best() const { return current_step_best_; }
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
		virtual void save_state( const path& filename ) const { FLUT_NOT_IMPLEMENTED; }
		virtual objective_info make_updated_objective_info() const { FLUT_NOT_IMPLEMENTED; }

		// properties
		int max_threads = 1;
		thread_priority thread_priority_;
		mutable string name; // TODO: not this, name should be const

	protected:
		virtual void internal_step() = 0;

		index_t step_count_;
		fitness_t current_step_median_;
		fitness_t current_step_average_;
		fitness_t current_step_best_;
		search_point current_step_best_point_;

		fitness_t best_fitness_;
		search_point best_point_;

		size_t fitness_history_samples_;
		circular_deque< float > fitness_history_;

		const objective& objective_;
		vector< s_ptr< reporter > > reporters_;
		vector< u_ptr< stop_condition > > stop_conditions_;
	};
}

#if defined(_MSC_VER)
#	pragma warning( pop )
#endif
