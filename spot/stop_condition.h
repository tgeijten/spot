#pragma once

#include "xo/container/container_tools.h"
#include "xo/numerical/regression.h"
#include "xo/numerical/polynomial.h"
#include "xo/container/circular_deque.h"
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
		virtual string what() const { return ""; }
		virtual bool test( const optimizer& opt ) = 0;
	};

	struct SPOT_API abort_condition : public stop_condition
	{
		virtual string what() const override { return "aborted by user"; }
		virtual bool test( const optimizer& opt ) override;
	};

	struct SPOT_API flat_fitness_condition : public stop_condition
	{
		flat_fitness_condition( fitness_t epsilon = 1e-6 ) : epsilon_( epsilon ) {}
		virtual string what() const override { return "flat fitness"; }
		virtual bool test( const optimizer& opt ) override;
		fitness_t epsilon_;
	};


	struct SPOT_API max_steps_condition : public stop_condition
	{
		max_steps_condition( size_t steps ) : max_steps_( steps = 99999 ) {}
		virtual string what() const override { return "maximum number of steps reached"; }
		virtual bool test( const optimizer& opt ) override;
		size_t max_steps_;
	};

	struct SPOT_API min_progress_condition : public stop_condition
	{
		min_progress_condition( fitness_t progress, size_t min_samples = 500 ) : min_progress_( progress ), min_samples_( min_samples ) {}
		virtual string what() const override { return "minimum progress threshold reached"; }
		virtual bool test( const optimizer& opt ) override;

		fitness_t min_progress_;
		size_t min_samples_;
	};

	struct SPOT_API similarity_condition : public stop_condition
	{
		virtual string what() const override { return "similar search point detected"; }
		virtual bool test( const optimizer& opt ) override;

		std::vector< par_vec > similarity_points;
		std::vector< double > similarities;
		index_t similar_idx;

		int min_steps_ = 10;
		par_value min_distance_ = 1.0;
	};
}

#if defined(_MSC_VER)
#	pragma warning( pop )
#endif
