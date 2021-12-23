#pragma once

#include "spot/spot_types.h"
#include "spot/objective.h"
#include "test_functions.h"
#include "spot/cma_optimizer.h"
#include "spot/console_reporter.h"
#include "xo/system/log.h"
#include "spot/mes_optimizer.h"

namespace spot
{
	static constexpr fitness_t min_progress = 1e-5;

	fitness_t test_cma_optimizer( const objective& obj )
	{
		sequential_evaluator eval;
		cma_options options;
		auto cma = cma_optimizer( obj, eval, options );
		//cma.add_reporter( std::make_unique<spot::console_reporter>( 0, 3 ) );
		cma.add_stop_condition( std::make_unique<spot::min_progress_condition>( min_progress ) );
		cma.run( 1000 );
		//cma.profiler().log_results();
		//xo::log::info( xo::stringf( "%-20s\tCMA-ES\t%d\t%g", obj.name().c_str(), cma.current_step(), cma.best_fitness() ));
		return cma.best_fitness();
	}

	fitness_t test_mes_optimizer( const objective& obj, par_t mean_sigma, par_t var_sigma, par_t mom_sigma )
	{
		sequential_evaluator eval;
		mes_options options;
		options.mean_sigma = mean_sigma;
		options.var_sigma = var_sigma;
		options.mom_sigma = mom_sigma;
		auto mes = mes_optimizer( obj, eval, options );
		//mes.add_reporter( std::make_unique<spot::console_reporter>( 0, 3 ) );
		mes.add_stop_condition( std::make_unique<spot::min_progress_condition>( min_progress ) );
		mes.run( 1000 );
		//cma.profiler().log_results();
		//xo::log::info( xo::stringf( "%-20s\tMVM-ES\t%d\t%g", obj.name().c_str(), mes.current_step(), mes.best_fitness() ));
		return mes.best_fitness();
	}

	void compare_optimizers() {
		auto objs = make_objectives( { 2, 20, 200 } );
		std::vector< std::vector< fitness_t > > total_results;

		for ( auto& obj : objs )
		{
			auto& res = total_results.emplace_back();
			res.emplace_back( test_cma_optimizer( obj ) );
			res.emplace_back( test_mes_optimizer( obj, 0.2, 0.2, 0.0 ) );
			res.emplace_back( test_mes_optimizer( obj, 0.2, 0.2, 0.2 ) );
			res.emplace_back( test_mes_optimizer( obj, 0.2, 0.2, 0.5 ) );

			std::string s = obj.name();
			for ( auto& r : res )
				s += "\t" + xo::stringf( "%8g", r );
			xo::log::info( s );
		}
	}
}
