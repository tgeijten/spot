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
		virtual const char* what() const override { return "flat fitness detected"; }
		virtual bool test( const optimizer& opt ) override;
		fitness_t epsilon_ = 1e-6;
	};


	struct SPOT_API max_steps_condition : public stop_condition
	{
		virtual const char* what() const override { return "maximum number of steps reached"; }
		virtual bool test( const optimizer& opt ) override;
		size_t max_steps_ = 10000;
	};

	struct SPOT_API min_progress_condition : public stop_condition
	{
		virtual const char* what() const override { return "minimum progress threshold reached"; }
		virtual bool test( const optimizer& opt ) override;

		fitness_t min_progress_ = 0.0;
		size_t progress_window_ = 0;
	};

	struct SPOT_API similarity_condition : public stop_condition
	{
		virtual const char* what() const override { return "similar search point detected"; }
		virtual bool test( const optimizer& opt ) override;

		vector< par_vec > similarity_points;
		vector< double > similarities;
		index_t similar_idx;

		int min_steps_ = 10;
		par_value min_distance_ = 1.0;
	};
}

#if defined(_MSC_VER)
#	pragma warning( pop )
#endif
