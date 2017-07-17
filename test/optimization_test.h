#pragma once
#include "test_functions.h"
#include "spot/cma_optimizer.h"
#include "spot/console_reporter.h"

namespace spot
{
	void optimization_test()
	{
		for ( int i = 1; i < 100; ++i )
		{
			auto obj = make_schwefel_objective( 20 );
			cma_optimizer opt( obj, 0, i );

			min_progress_condition mp( 1000, 0.0001 );
			opt.add_stop_condition( &mp );

			//console_reporter cr;
			//opt.add_reporter( &cr );

			auto* sc = opt.run();

			log::info( i, ":\t", opt.best_fitness(), "\tsteps=", opt.current_step(), "\t", sc->what() );
			//std::cout << opt.best();
		}
	}
}
