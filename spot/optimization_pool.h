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
	class SPOT_API optimization_pool : interruptible
	{
	public:
		optimization_pool( size_t fitness_tracking_window = 100, size_t min_window_size = 0 );
		optimization_pool( const optimization_pool& ) = delete;
		optimization_pool& operator=( const optimization_pool& ) = delete;
		virtual ~optimization_pool() {}

		void push_back( u_ptr< optimizer > opt );
		optimizer& back() { return *optimizers_.back(); }
		optimizer& front() { return *optimizers_.front(); }
		size_t size() const { return optimizers_.size(); }

		void run( int max_steps = max<int>() );
		bool step();
		virtual void interrupt() const override;

	private:
		size_t max_window_size_;
		size_t min_window_size_;
		std::vector< u_ptr< optimizer > > optimizers_;
		std::deque< index_t > step_queue_;
	};
}

#if defined(_MSC_VER)
#	pragma warning( pop )
#endif
