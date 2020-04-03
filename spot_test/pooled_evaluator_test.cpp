#include "xo/system/test_case.h"

#include "test_functions.h"
#include "spot/cma_optimizer.h"
#include "spot/pooled_evaluator.h"
#include "xo/system/log.h"

namespace spot
{
	XO_TEST_CASE( pooled_evaluator_test )
	{
		auto obj = make_schwefel_objective( 10 );
		auto eval = pooled_evaluator( 4, xo::thread_priority::low );

		cma_optimizer opt( obj, cma_options(), eval );
		opt.add_stop_condition( std::make_unique< max_steps_condition >( 1000 ) );
		auto* sc = opt.run();

		xo::log::info( "best_fitness = ", opt.best_fitness() );
	}
}
