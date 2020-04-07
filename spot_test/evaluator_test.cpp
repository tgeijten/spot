#include "xo/system/test_case.h"

#include "test_functions.h"
#include "spot/cma_optimizer.h"
#include "spot/pooled_evaluator.h"
#include "xo/system/log.h"
#include "spot/async_evaluator.h"
#include "xo/time/stopwatch.h"
#include <thread>
#include <chrono>
#include <iostream>

using namespace std::chrono_literals;
using std::cout;

namespace spot
{
	inline constexpr int g_dim = 16;
	inline constexpr cma_options g_cma{ 0 };
	inline constexpr int g_iterations = 10;
	inline constexpr int g_optimizers = 10;

	fitness_t evaluator_test( evaluator& e, size_t num_opt = g_optimizers )
	{
		auto obj = make_slow_schwefel_objective( g_dim );
		std::vector<u_ptr<cma_optimizer>> optimizers;
		std::vector<std::future<const stop_condition*>> threads;
		while ( optimizers.size() < num_opt )
		{
			optimizers.emplace_back( std::make_unique<cma_optimizer>( obj, e, g_cma ) );
			optimizers.back()->add_stop_condition( std::make_unique< max_steps_condition >( g_iterations ) );
		}

		for ( auto& opt : optimizers )
		{
			cout << "starting " << threads.size() << " lambda=" << opt->lambda() << "\n";
			threads.push_back( std::async( std::launch::async, [&]( cma_optimizer& o ) { return o.run(); }, std::ref( *opt ) ) );
		}

		size_t finished = 0;
		while ( finished < threads.size() )
		{
			for ( index_t i = 0; i < threads.size(); ++i )
				if ( threads[ i ].valid() && threads[ i ].wait_for( 1ms ) == std::future_status::ready )
				{
					cout << "finished " << i << "\n";
					threads[ i ] = std::future<const stop_condition*>();
					++finished;
				}
		}
		cout << "DONE!" << std::endl;

		return optimizers.front()->best_fitness();
	}

	XO_TEST_CASE( evaluator_test )
	{
		auto seq_eval = sequential_evaluator();
		auto async_eval = async_evaluator( 64, xo::thread_priority::low );
		auto pooled_eval = pooled_evaluator( 0, xo::thread_priority::low );

		xo::stopwatch sw;
		auto f1 = evaluator_test( seq_eval );
		sw.add_measure( "seq" );
		auto f2 = evaluator_test( async_eval );
		sw.add_measure( "async" );
		auto f3 = evaluator_test( pooled_eval );
		sw.add_measure( "pool" );

		//XO_CHECK( f1 == f2 );
		XO_CHECK( f2 == f3 );

		xo::log::info( "results:\n", sw.get_report() );
	}
}
