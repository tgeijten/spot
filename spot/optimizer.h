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

#if defined(_MSC_VER)
#	pragma warning( push )
#	pragma warning( disable: 4251 )
#endif

namespace spot
{
	class SPOT_API optimizer
	{
	public:
		optimizer( const objective& o, const prop_node& pn = prop_node() );
		virtual ~optimizer();

		void run_threaded();
		virtual stop_condition* run( size_t number_of_steps = 0 );
		virtual stop_condition* test_stop_conditions() const;
		virtual void step() { FLUT_NOT_IMPLEMENTED; }

		void add_stop_condition( stop_condition* cb ) { stop_conditions_.push_back( cb ); }
		void add_reporter( reporter* cb ) { reporters_.push_back( cb ); }

		void signal_abort() { abort_flag_ = true; }
		void abort_and_wait();

		bool test_abort() const { return abort_flag_; }

		fitness_vec_t evaluate( const search_point_vec& pop );
		
		void set_max_threads( int val ) { max_threads = val; }
		void set_max_steps( size_t gen ) { max_steps = gen; }
		void set_min_progress( fitness_t relative_improvement_per_step, size_t window );

		int current_step() const { return current_step_; }
		fitness_t current_step_median() const { return current_step_mean_; }
		fitness_t current_step_average() const { return current_step_average_; }
		fitness_t current_step_best() const { return current_step_best_; }

		fitness_t best_fitness() const { return best_fitness_; }
		const search_point& best() const { return best_point_; }

		const objective_info& info() const { return objective_.info(); }
		const objective& obj() const { return objective_; }
		bool is_better( fitness_t a, fitness_t b ) const { return objective_.info().is_better( a, b ); }

		// state
		virtual void save_state( const path& filename ) const { FLUT_NOT_IMPLEMENTED; }
		virtual objective_info make_updated_objective_info() const { FLUT_NOT_IMPLEMENTED; }

		// properties
		int max_threads = 1;
		size_t max_steps = 10000;

		stop_condition* stop_condition_;
		abort_condition abort_condition_;
		flat_fitness_condition flat_fitness_condition_;

	protected:
		// evaluation settings
		std::atomic_bool abort_flag_ = false;

		std::thread background_thread;
		flut::thread_priority thread_priority;

		int current_step_;
		fitness_t current_step_mean_;
		fitness_t current_step_average_;
		fitness_t current_step_best_;

		fitness_t best_fitness_;
		search_point best_point_;

		const objective& objective_;
		vector< reporter* > reporters_;
		vector< stop_condition* > stop_conditions_;
	};
}

#if defined(_MSC_VER)
#	pragma warning( pop )
#endif
