#pragma once

#include "spot_types.h"
#include "optimizer.h"
#include <deque>

namespace spot
{
	/// Pool of independent optimizations, prioritized based on their predicted fitness.
	class SPOT_API optimizer_pool : public optimizer
	{
	public:
		optimizer_pool( const objective& o, evaluator& e, const prop_node& pn );
		optimizer_pool( const optimizer_pool& ) = delete;
		optimizer_pool& operator=( const optimizer_pool& ) = delete;
		virtual ~optimizer_pool() {}

		virtual const fitness_vec& current_step_fitnesses() const override { return best_optimizer().current_step_fitnesses(); }
		virtual fitness_t current_step_best_fitness() const override { return best_optimizer().current_step_best_fitness(); }
		virtual const search_point& current_step_best_point() const override { return best_optimizer().current_step_best_point(); }
		virtual fitness_t best_fitness() const override { return best_optimizer().best_fitness(); }
		virtual const search_point& best_point() const override { return best_optimizer().best_point(); }

		void push_back( u_ptr< optimizer > opt );
		const vector< u_ptr< optimizer > >& optimizers() const { return optimizers_; }
		size_t size() const { return optimizers_.size(); }

		virtual bool interrupt() override;
		virtual objective_info make_updated_objective_info() const override;

		/// Number of generations on which to base the prediction.
		size_t prediction_window_;

		/// Number of generations after which prediction starts.
		size_t prediction_start_;

		/// Prediction look-ahead.
		size_t prediction_look_ahead_;

		/// Stop when predicted fitness is worse than current best.
		bool use_predicted_fitness_stop_condition_;

		/// Maximum number of active optimizations.
		size_t active_optimizations_;

		/// Maximum number of optimizations running concurrently.
		size_t concurrent_optimizations_;

	protected:
		fitness_t best_fitness_;

		virtual vector< double > compute_predicted_fitnesses();
		virtual void internal_step() override;

		vector< u_ptr< optimizer > > optimizers_;
		std::deque< index_t > step_queue_;
		index_t best_optimizer_idx_;
		const optimizer& best_optimizer() const { xo_assert( best_optimizer_idx_ < optimizers_.size() ); return *optimizers_[ best_optimizer_idx_ ]; }
	};
}
