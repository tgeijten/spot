#pragma once

#include "spot_types.h"
#include "optimizer.h"
#include "search_point.h"
#include <random>

namespace spot
{
	struct mes_options {
		int lambda = 0;
		int mu = 0;
		long random_seed = 123;
		par_t mean_sigma = 0.2;
		par_t var_sigma = 0.2;
		par_t mom_sigma = 0.2;
	};

	class SPOT_API mes_optimizer : public optimizer
	{
	public:
		mes_optimizer() = delete;
		mes_optimizer( const objective& o, evaluator& e, const mes_options& options = mes_options() );
		mes_optimizer( const mes_optimizer& ) = delete;
		mes_optimizer& operator=( const mes_optimizer& ) = delete;
		virtual ~mes_optimizer() = default;

	protected:
		par_t sample_parameter( par_t mean, par_t stdev, const par_info& pi );
		void sample_population();
		void update_distribution();
		virtual bool internal_step() override;
		search_point_vec& population() { return *population_; }

		int lambda_;
		int mu_;
		size_t max_resample_count;
		par_vec mean_;
		par_vec var_;
		par_vec mom_;
		par_t mean_sigma;
		par_t var_sigma;
		par_t mom_sigma;
		std::minstd_rand random_engine_;
		std::unique_ptr<search_point_vec> population_;
	};
}
