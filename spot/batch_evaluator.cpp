#include "batch_evaluator.h"

#include "objective.h"

namespace spot
{
	batch_evaluator::batch_evaluator( xo::thread_priority thread_prio ) :
		evaluator(),
		thread_prio_( thread_prio )
	{}

	vector< result<fitness_t> > batch_evaluator::evaluate( const objective& o, const search_point_vec& point_vec, const xo::stop_token& st, priority_t prio )
	{
		// create threads
		vector< std::future< xo::result< fitness_t > > > futures;
		for ( const auto& point : point_vec )
		{
			futures.emplace_back(
				std::async( std::launch::async,	[&]() {
					xo::set_thread_priority( thread_prio_ );
					return o.evaluate_noexcept( point, st );
				} )
			);
		}

		// wait for results
		vector< result<fitness_t> > results;
		results.reserve( point_vec.size() );
		for ( auto& f : futures )
			results.push_back( f.get() );

		return results;
	}

	void batch_evaluator::set_thread_priority( xo::thread_priority prio )
	{
		thread_prio_ = prio;
	}
}
