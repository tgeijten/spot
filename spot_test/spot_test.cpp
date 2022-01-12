//#include "xo/system/test_main.h"

#include "xo/system/log.h"
#include "xo/system/log_sink.h"
#include "compare_optimizers.h"
#include "math_test.h"
#include "xo/numerical/average.h"

int main( int argc, char** argv )
{
	xo::log::console_sink log_sink( xo::log::level::trace );

	try
	{
		std::vector< spot::fitness_t > results;
		for ( unsigned int s = 0; s < 1000; ++s )
			results.emplace_back( spot::variance_test( s ) );
		auto [mean, stdev] = xo::mean_std( results );
		xo::log::info( "M=", mean, " S=", stdev );
		//spot::compare_optimizers();
	}
	catch ( std::exception& e )
	{
		xo::log::critical( "CRITICAL ERROR: ", e.what() );
		return -1;
	}

	return 0;
}
