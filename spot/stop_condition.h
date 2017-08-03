#pragma once

#include "flut/container_tools.hpp"
#include "flut/math/linear_regression.hpp"
#include "flut/math/polynomial.hpp"
#include "flut/circular_deque.hpp"
#include "search_point.h"
#include "tools.h"

#if defined(_MSC_VER)
#	pragma warning( push )
#	pragma warning( disable: 4251 )
#endif

namespace spot
{
	class optimizer;

	struct SPOT_API stop_condition
	{
		stop_condition() {}
		virtual ~stop_condition() {}
		virtual const char* what() const { return ""; }
		virtual bool test( const optimizer& opt ) = 0;
	};

	struct SPOT_API abort_condition : public stop_condition
	{
		virtual const char* what() const override { return "aborted by user"; }
		virtual bool test( const optimizer& opt ) override;
	};

	struct SPOT_API flat_fitness_condition : public stop_condition
	{
		flat_fitness_condition( fitness_t epsilon ) : epsilon_( epsilon ) {}
		virtual const char* what() const override { return "flat fitness detected"; }
		virtual bool test( const optimizer& opt ) override;
		fitness_t epsilon_;
	};


	struct SPOT_API max_steps_condition : public stop_condition
	{
		max_steps_condition( size_t max_steps ) : max_steps_( max_steps ) {}
		virtual const char* what() const override { return "maximum number of steps reached"; }
		virtual bool test( const optimizer& opt ) override;
		size_t max_steps_;
	};

	struct SPOT_API min_progress_condition : public stop_condition
	{
		min_progress_condition( size_t window_size, double min_progress ) : progress_window_( window_size ), min_progress_( min_progress ) {}
		virtual const char* what() const override { return "minimum progress threshold reached"; }
		virtual bool test( const optimizer& opt ) override;

		fitness_t min_progress_;
		size_t progress_window_;
	};

	struct SPOT_API similarity_condition : public stop_condition
	{
		similarity_condition( par_value min_distance = 1.0, int min_steps = 10 ) : min_distance_( min_distance ), min_steps_( min_steps ) {}
		virtual const char* what() const override { return "similar search point detected"; }
		virtual bool test( const optimizer& opt ) override;

		vector< par_vec > similarity_points;
		vector< double > similarities;
		index_t similar_idx;

		int min_steps_;
		par_value min_distance_;
	};
}

#if defined(_MSC_VER)
#	pragma warning( pop )
#endif
