#pragma once

#include "spot_types.h"
#include "optimizer.h"

namespace spot
{
	enum class cma_weights { equal = 0, linear = 1, log = 2 };

	struct cma_options {
		int lambda = 0;
		long random_seed = 123;
		cma_weights weights = cma_weights::log; // #todo: this setting is currently ignored :S
		double update_eigen_modulo = -1;
	};

	class SPOT_API cma_optimizer : public optimizer
	{
	public:
		cma_optimizer( const objective& o, evaluator& e, const cma_options& options = cma_options() );
		virtual ~cma_optimizer();

		// optimization
		const search_point_vec& sample_population();
		void update_distribution( const fitness_vec& results );

		// fitness info
		virtual const fitness_vec& current_step_fitnesses() const override { return current_step_fitnesses_; }
		virtual fitness_t current_step_best_fitness() const override { return current_step_best_fitness_; }
		virtual const search_point& current_step_best_point() const override { return current_step_best_point_; }
		virtual fitness_t best_fitness() const override { return best_fitness_; }
		virtual const search_point& best_point() const override { return best_point_; }

		// analysis
		par_vec current_mean() const;
		par_vec current_std() const;
		//vector< par_vec > current_covariance() const;

		// state
		virtual void save_state( const path& filename ) const override;
		virtual objective_info make_updated_objective_info() const override;

		// actual parameters
		int dim() const;
		int lambda() const;
		int mu() const;
		int random_seed() const;
		double sigma() const;

	protected:
		fitness_t best_fitness_;
		search_point best_point_;
		fitness_t current_step_best_fitness_;
		fitness_vec current_step_fitnesses_;
		search_point current_step_best_point_;

		size_t max_resample_count;

		virtual void internal_step() override;
		struct pimpl_t* pimpl;
	};
}
