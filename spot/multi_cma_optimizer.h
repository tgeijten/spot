#pragma once

#include "optimizer.h"
#include "cma_optimizer.h"

#if defined(_MSC_VER)
#	pragma warning( push )
#	pragma warning( disable: 4251 )
#endif

namespace spot
{
	class SPOT_API multi_cma_optimizer : public optimizer
	{
	public:
		multi_cma_optimizer( const objective& obj, size_t max_solutions, size_t max_searches, double min_distance = 0.0, int seed = 123 );
		virtual ~multi_cma_optimizer() { abort_and_wait(); }

		// inherited from stop_condition
		struct multi_stop_condition : public stop_condition
		{
			virtual string what() const override { return "All optimizations have terminated"; }
			virtual bool test( const optimizer& opt ) override {
				auto& o = dynamic_cast< const multi_cma_optimizer& >( opt );
				return ( ( o.optimizers_.size() >= o.max_solutions_ || o.search_count_ == o.max_searches_ ) && !o.optimizers_.back()->is_active() );
			}
		};

		virtual void signal_abort() override;
		virtual void abort_and_wait() override;

	protected:
		virtual void internal_step() override;

		int seed_;
		size_t search_count_;
		vector< u_ptr< cma_optimizer > > optimizers_;
		size_t max_solutions_;
		size_t max_searches_;
		s_ptr< similarity_condition > similarity_stop_;
	};
}

#if defined(_MSC_VER)
#	pragma warning( pop )
#endif
