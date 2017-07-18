#pragma once
#include "test_functions.h"
#include "spot/cma_optimizer.h"
#include "spot/console_reporter.h"
#include <memory>
#include "flut/storage.hpp"

namespace spot
{
	void min_progress_test()
	{
		for ( int i = 1; i < 100; ++i )
		{
			auto obj = make_schwefel_objective( 10 );

			cma_optimizer opt1( obj, 0, i );
			opt1.add_stop_condition( std::make_unique< min_progress_condition >( 1000, 1e-6 ) );
			auto* sc1 = opt1.run();

			cma_optimizer opt2( obj, 0, i );
			opt2.add_stop_condition( std::make_unique< max_steps_condition >( 50000 ) );
			auto* sc2 = opt2.run();

			auto f1 = opt1.best_fitness();
			auto f2 = opt2.best_fitness();

			log::messagef( log::info_level, "%3d: f1=%8.3f s1=%4d f2=%8.3f s2=%4d improvement=%5.3f", i, f1, opt1.current_step(), f2, opt2.current_step(), f1 / f2 );
		}
	}

	void multimodal_test()
	{
		auto obj = make_himmelblau_objective();

		int optimizations = 10;
		storage< float > sto( 1000, optimizations * 2 );
		for ( int i = 0; i < 10; ++i )
		{
			sto.set_label( i * 2, stringf( "O%d.X", i ) );
			sto.set_label( i * 2 + 1, stringf( "O%d.Y", i ) );
			cma_optimizer opt( obj, 0, i + 1 );
			opt.add_stop_condition( std::make_unique< min_progress_condition >( 100, 1e-6 ) );
			opt.add_stop_condition( std::make_unique< max_steps_condition >( 1000 ) );
			for ( int step = 0; !opt.step(); ++step )
			{
				sto( step, i * 2 ) = (float)opt.current_mean()[ 0 ];
				sto( step, i * 2 + 1 ) = (float)opt.current_mean()[ 1 ];
			}
			log::infof( "%d: %f", i, opt.best_fitness() );
		}

		std::ofstream( "X:/opt_output.txt" ) << sto;
	}
}
