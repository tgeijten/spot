#pragma once

#include "optimizer.h"
#include "types.h"
#include "xo/utility/interruptible.h"
#include <deque>

#if defined(_MSC_VER)
#	pragma warning( push )
#	pragma warning( disable: 4251 )
#endif

namespace spot
{
	class SPOT_API optimizer_pool : public optimizer
	{
	public:
		optimizer_pool( const objective& o, size_t prediction_window = 100, size_t prediction_window_min = 10, size_t max_concurrent_optimizations = 4 );
		optimizer_pool( const optimizer_pool& ) = delete;
		optimizer_pool& operator=( const optimizer_pool& ) = delete;
		virtual ~optimizer_pool() {}

		void push_back( u_ptr< optimizer > opt );
		const std::vector< u_ptr< optimizer > >& optimizers() const { return optimizers_; }
		size_t size() const { return optimizers_.size(); }

		virtual void interrupt() const override;

	protected:
		size_t prediction_window_size_;
		size_t prediction_start_;
		size_t max_concurrent_optimizations_;

		virtual std::vector< double > compute_predicted_fitnesses();
		virtual void internal_step() override;

		std::vector< u_ptr< optimizer > > optimizers_;
		std::deque< index_t > step_queue_;
	};
}

#if defined(_MSC_VER)
#	pragma warning( pop )
#endif
