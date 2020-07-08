#include "xo/system/test_case.h"

#include "spot/cma_optimizer.h"
#include "test_functions.h"
#include "xo/system/log.h"
#include "xo/time/stopwatch.h"
#include "spot/async_evaluator.h"
#include "spot/batch_evaluator.h"
#include "spot/pooled_evaluator.h"
#include <chrono>
#include <thread>

using namespace std::chrono_literals;

namespace spot
{
	inline constexpr int g_dim = 12;
	inline constexpr cma_options g_cma{ 32 };
	inline constexpr int g_iterations = 100;
	inline constexpr int g_optimizers = 20;

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
			printf( "+%zd", threads.size() );
			threads.push_back( std::async( std::launch::async, [&]( cma_optimizer& o ) { return o.run(); }, std::ref( *opt ) ) );
		}
		printf( " " );

		size_t finished = 0;
		while ( finished < threads.size() )
		{
			for ( index_t i = 0; i < threads.size(); ++i )
				if ( threads[ i ].valid() && threads[ i ].wait_for( 1ms ) == std::future_status::ready )
				{
					printf( "-%zd", i );
					threads[ i ] = std::future<const stop_condition*>();
					++finished;
				}
		}
		printf( "...DONE!\n" );

		return optimizers.front()->best_fitness();
	}

	XO_TEST_CASE( evaluator_test )
	{
		auto seq_eval = sequential_evaluator();
		auto async_eval = async_evaluator( 0, xo::thread_priority::low );
		auto batch_eval = batch_evaluator( xo::thread_priority::low );
		auto pooled_eval = pooled_evaluator( 0, xo::thread_priority::low );

		xo::stopwatch sw;
		auto fs = evaluator_test( seq_eval );
		sw.split( "seq" );
		auto fa = evaluator_test( async_eval );
		sw.split( "async" );
		auto fb = evaluator_test( batch_eval );
		sw.split( "batch" );
		auto fp = evaluator_test( pooled_eval );
		sw.split( "pool" );

		XO_CHECK( fa == fp );

		xo::log::info( "results:\n", sw.get_report() );
	}
}
