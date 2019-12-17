#pragma once

#include "test_functions.h"

#include "spot/cma_optimizer.h"
#include "spot/console_reporter.h"

#include "xo/container/storage.h"
#include "xo/system/log.h"

#include "spot/tools.h"
#include "spot/optimizer_pool.h"

namespace spot
{
	void min_progress_test()
	{
		for ( int i = 1; i < 100; ++i )
		{
			auto obj = make_schwefel_objective( 10 );

			cma_optimizer opt1( obj, 0, i );
			opt1.add_stop_condition( std::make_unique< min_progress_condition >( 1e-6, 1000 ) );
			auto* sc1 = opt1.run();

			cma_optimizer opt2( obj, 0, i );
			opt2.add_stop_condition( std::make_unique< max_steps_condition >( 50000 ) );
			auto* sc2 = opt2.run();

			auto f1 = opt1.best_fitness();
			auto f2 = opt2.best_fitness();

			xo::log::infof( "%3d: f1=%8.3f s1=%4d f2=%8.3f s2=%4d improvement=%5.3f", i, f1, opt1.current_step(), f2, opt2.current_step(), f1 / f2 );
		}
	}

	void multimodal_test()
	{
#if 0
		auto obj = make_schwefel_objective( 2 );
		int opt_count = 10;
		storage< float > sto( 1000, opt_count * 2 );
		vector< cma_optimizer > opt;

		for ( int i = 0; i < opt_count; ++i )
		{
			sto.set_label( i * 2, stringf( "O%d.X", i ) );
			sto.set_label( i * 2 + 1, stringf( "O%d.Y", i ) );
			opt.emplace_back( obj, 0, i + 1 );
			opt.back().add_stop_condition( std::make_unique< min_progress_condition >( 100, 1e-6 ) );
			opt.back().add_stop_condition( std::make_unique< max_steps_condition >( 1000 ) );

		}

		int active_count = 10;
		for ( int step = 0; active_count > 0; ++step )
		{
			for ( int i = 0; i < opt_count; ++i )
			{
				string s = stringf( "%2d: ", i );

				if ( opt[ i ].is_active() && opt[ i ].step() )
					--active_count;
				sto( step, i * 2 ) = (float)opt[ i ].current_mean()[ 0 ];
				sto( step, i * 2 + 1 ) = (float)opt[ i ].current_mean()[ 1 ];

				s += stringf( " F=%6.3f", opt[ i ].current_step_best() );

				for ( int j = 0; j < opt_count; ++j )
				{
					auto d = normalized_distance( opt[ i ].current_mean(), opt[ j ].current_mean(), opt[ i ].current_std() );
					s += stringf( " %6.3f", d );
				}
				log::info( s );
			}
		}

		std::ofstream( "X:/opt_output.txt" ) << sto;
#endif
	}

	void multi_optimizer_test()
	{
		auto obj = make_rosenbrock_objective( 3 );
		optimizer_pool pool( obj, 50, 50, 3 );

		for ( int i = 0; i < 12; ++i )
			pool.push_back( std::make_unique< cma_optimizer >( obj, 0, i + 1 ) );
		pool.run();
	}
}
