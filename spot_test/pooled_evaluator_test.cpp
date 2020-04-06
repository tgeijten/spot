#include "xo/system/test_case.h"

#include "test_functions.h"
#include "spot/cma_optimizer.h"
#include "spot/pooled_evaluator.h"
#include "xo/system/log.h"
#include "spot/async_evaluator.h"
#include "xo/time/stopwatch.h"

namespace spot
{
	fitness_t evaluator_test( evaluator& e )
	{
		auto obj = make_slow_schwefel_objective( 10 );
		cma_optimizer opt( obj, e, cma_options{ 32 } );
		opt.add_stop_condition( std::make_unique< max_steps_condition >( 1000 ) );
		auto* sc = opt.run();
		return opt.best_fitness();
	}

	XO_TEST_CASE( evaluator_test )
	{
		auto pooled_eval = pooled_evaluator( 8, xo::thread_priority::low );
		auto async_eval = async_evaluator( 32, xo::thread_priority::low );
		auto seq_eval = sequential_evaluator();

		xo::stopwatch sw;
		auto f1 = evaluator_test( seq_eval );
		sw.add_measure( "seq" );
		auto f2 = evaluator_test( async_eval );
		sw.add_measure( "async" );
		auto f3 = evaluator_test( pooled_eval);
		sw.add_measure( "pool" );

		XO_CHECK( f1 == f2 );
		XO_CHECK( f2 == f3 );

		xo::log::info( sw.get_report() );
	}
}
