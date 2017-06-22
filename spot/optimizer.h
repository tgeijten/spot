#pragma once

#include "objective.h"
#include "reporter.h"

#include "flut/prop_node.hpp"
#include "flut/system/path.hpp"
#include "flut/system/types.hpp"

#include "flut/math/math.hpp"
#include "flut/math/optional.hpp"
#include "flut/system_tools.hpp"
#include "flut/circular_deque.hpp"

#include <atomic>
#include <functional>
#include <thread>

#if defined(_MSC_VER)
#	pragma warning( push )
#	pragma warning( disable: 4251 )
#endif

namespace spot
{
	class SPOT_API optimizer
	{
	public:
		enum stop_condition { no_stop_condition, max_steps_reached, min_progress_reached, target_fitness_reached, user_abort };

		optimizer( const objective& o );
		virtual ~optimizer();

		void run_threaded();
		virtual stop_condition run();
		virtual void step() { FLUT_NOT_IMPLEMENTED; }

		void add_reporter( reporter* cb ) { reporters_.push_back( cb ); }

		void signal_abort() { abort_flag_ = true; }
		void abort_and_wait();

		bool test_abort() const { return abort_flag_; }

		virtual stop_condition test_stop_condition() const;
		fitness_vec_t evaluate( const search_point_vec& pop );
		
		int max_threads() const { return max_threads_; }
		void set_max_threads( int val ) { max_threads_ = val; }

		void set_max_generations( size_t gen ) { max_generations_ = gen; }
		void set_min_progress( fitness_t relative_improvement_per_step, size_t window );

		size_t generation_count() const { return step_count_; }
		fitness_t current_fitness() const { return current_fitness_; }

		const objective_info& info() const { return objective_.info(); }
		const objective& obj() const { return objective_; }

	protected:
		// evaluation settings
		int max_threads_ = 1;
		std::atomic_bool abort_flag_ = false;
		stop_condition stop_condition_;

		std::thread background_thread;
		flut::thread_priority thread_priority;

		size_t step_count_;
		fitness_t current_fitness_;

		const objective& objective_;
		vector< reporter* > reporters_;

		// stop conditions
		size_t max_generations_ = 10000;
		fitness_t min_progress_ = 0;
		circular_deque< fitness_t > progress_window;
		optional< fitness_t > target_fitness_;

		void update_progress( fitness_t current_median );
	};
}

#if defined(_MSC_VER)
#	pragma warning( pop )
#endif
