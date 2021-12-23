#include "console_reporter.h"

#include "optimizer.h"
#include "xo/container/container_algorithms.h"

namespace spot
{
	
	console_reporter::console_reporter( int individual_precision, int summary_precision , bool newline_best ) :
		individual_precision_( individual_precision ),
		summary_precision_( summary_precision ),
		newline_best_( newline_best )
	{}

	void console_reporter::on_post_evaluate_point( const optimizer& opt, const search_point& point, fitness_t fitness )
	{
		printf( "%.*f ", individual_precision_, fitness );
	}

	void console_reporter::on_post_evaluate_population( const optimizer& opt, const search_point_vec& pop, const fitness_vec& fitnesses, bool new_best )
	{
		auto avg = xo::average( fitnesses );
		auto med = xo::median( fitnesses );
		printf( " A=%.*f M=%.*f", summary_precision_, avg, summary_precision_, med );
		if ( new_best )
			printf( " B=%.*f", summary_precision_, opt.best_fitness() );
		if ( newline_best_ && new_best )
			printf( "\n" );
		else printf( "\r" );
	}

	void console_reporter::on_stop( const optimizer& opt, const stop_condition& s )
	{
		printf( "\nOptimization finished: %s\n", s.what().c_str() );
	}

	void console_reporter::on_pre_step( const optimizer& opt )
	{
		printf( "%04d: ", ( int )opt.current_step() );
	}

	void console_reporter::on_start( const optimizer& opt )
	{
		printf( "Starting optimization, dim=%d\n", ( int )opt.info().dim() );
	}
}
