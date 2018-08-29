#include "console_reporter.h"

namespace spot
{
	
	console_reporter::console_reporter( int individual_precision, int summary_precision ) : individual_precision_( individual_precision ), summary_precision_( summary_precision )
	{

	}

	void console_reporter::on_post_evaluate_point( const optimizer& opt, const search_point& point, fitness_t fitness )
	{
		printf( "%.*f ", individual_precision_, fitness );
	}

	void console_reporter::on_post_evaluate_population( const optimizer& opt, const search_point_vec& pop, const fitness_vec_t& fitnesses, bool new_best )
	{
		auto avg = xo::average( fitnesses );
		auto med = xo::median( fitnesses );
		printf( " A=%.*f M=%.*f", summary_precision_, avg, summary_precision_, med );
		if ( new_best )
			printf( " B=%.*f\n", summary_precision_, opt.best_fitness() );
		else printf( "\r" );
	}

	void console_reporter::on_stop( const optimizer& opt, const stop_condition& s )
	{
		printf( "\nOptimization finished: %s", s.what().c_str() );
	}

	void console_reporter::on_next_step( const optimizer& opt, size_t gen )
	{
		printf( "%04d: ", ( int )gen );
	}

	void console_reporter::on_start( const optimizer& opt )
	{
		printf( "Starting optimization, dim=%d\n", ( int )opt.info().dim() );
	}

}
