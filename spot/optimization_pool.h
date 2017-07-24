#pragma once

#include "optimizer.h"
#include "types.h"
#include "flut/interruptible.h"

#if defined(_MSC_VER)
#	pragma warning( push )
#	pragma warning( disable: 4251 )
#endif

namespace spot
{
	class SPOT_API optimization_pool : interruptible
	{
	public:
		optimization_pool( size_t fitness_tracking_window = 100 );
		optimization_pool( const optimization_pool& ) = delete;
		optimization_pool& operator=( const optimization_pool& ) = delete;
		virtual ~optimization_pool() {}

		void push_back( u_ptr< optimizer > opt );
		optimizer& back() { return *optimizers_.back(); }
		optimizer& front() { return *optimizers_.front(); }
		size_t size() const { return optimizers_.size(); }

		void run( int max_steps = num_const< int >::max() );
		void step();
		virtual void interrupt() const override;

	private:
		struct fitness_tracker
		{
			fitness_tracker( size_t s ) : promise_( 1 ), progress_( 1 ), history_( s ) {}
			void update( const optimizer& opt );
			float promise() const { return promise_; }
			bool operator<( const fitness_tracker& other ) const { return promise() < other.promise(); }
			float progress_;
			float promise_;

		private:
			flut::circular_deque< float > history_;
			flut::linear_function< float > regression_;
		};

		vector< fitness_tracker > fitness_trackers_;
		size_t promise_window_;
		vector< u_ptr< optimizer > > optimizers_;
	};
}

#if defined(_MSC_VER)
#	pragma warning( pop )
#endif
