#pragma once

#include "optimizer.h"
#include "types.h"
#include "flut/interruptible.h"
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

		void run( int max_steps = constants< int >::max() );
		bool step();
		virtual void interrupt() const override;

	private:
		struct fitness_tracker
		{
			fitness_tracker( size_t s, size_t min_s ) : promise_( 1 ), progress_( 1 ), history_( s ), min_window_size_( min_s ) {}
			void update( const optimizer& opt );
			float promise() const { return promise_; }
			bool operator<( const fitness_tracker& other ) const { return promise() > other.promise(); }
			float progress_;
			float promise_;

		private:
			size_t min_window_size_;
			flut::circular_deque< float > history_;
			flut::linear_function< float > regression_;
		};

		vector< fitness_tracker > fitness_trackers_;
		size_t max_window_size_;
		size_t min_window_size_;
		vector< u_ptr< optimizer > > optimizers_;
		std::deque< index_t > step_queue_;
	};
}

#if defined(_MSC_VER)
#	pragma warning( pop )
#endif
