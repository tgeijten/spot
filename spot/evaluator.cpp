#include "evaluator.h"

#include "objective.h"
#include "xo/system/system_tools.h"
#include "spot/search_point.h"
#include "async_evaluator.h"

namespace spot
{
	vector< result<fitness_t> > sequential_evaluator::evaluate( const objective& o, const search_point_vec& point_vec, const xo::stop_token& st, priority_t prio )
	{
		// single threaded evaluation
		vector< result<fitness_t> > results;
		results.reserve( point_vec.size() );
		for ( const auto& sp : point_vec )
			results.push_back( o.evaluate_noexcept( sp, st ) );
	
		return results;
	}
}
