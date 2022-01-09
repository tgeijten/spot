#pragma once

#include "spot_types.h"
#include "optimizer.h"
#include "search_point.h"
#include <random>

namespace spot
{
	struct eva_options {
		int lambda = 0;
		int mu = 0;
		long random_seed = 123;
		par_t step_size = 0.2;
		par_t var_update = 0.2;
		par_t ev_update = 0.2;
		par_t ev_offset = 1.0;
		par_t ev_stdev = 1.0;
	};

	class SPOT_API eva_optimizer : public optimizer
	{
	public:
		eva_optimizer( const objective& o, evaluator& e, const eva_options& options = eva_options() );
		virtual ~eva_optimizer() = default;

		int lambda() const { return lambda_; }
		int mu() const { return mu_; }
		const par_vec& current_mean() const { return mean_; }
		par_vec current_std() const;
		virtual objective_info make_updated_objective_info() const override;
		virtual vector< string > optimizer_state_labels() const override;
		virtual vector< par_t > optimizer_state_values() const override;

	protected:
		par_t sample_parameter( par_t mean, par_t stdev, const par_info& pi );
		void sample_population();
		void update_distribution();
		virtual bool internal_step() override;
		search_point_vec& population() { return population_; }

		int lambda_;
		int mu_;
		size_t max_resample_count;
		par_vec mean_;
		par_vec var_;
		par_vec ev_;
		eva_options options_;
		std::minstd_rand random_engine_;
		search_point_vec population_;
	};
}
