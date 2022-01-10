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
		xo::averaged avg;
		for ( unsigned int s = 0; s < 100; ++s )
			avg.add( spot::variance_test( s ) );
		xo::log::info( "AVERAGE=", avg.get() );
		//spot::compare_optimizers();
	}
	catch ( std::exception& e )
	{
		xo::log::critical( "CRITICAL ERROR: ", e.what() );
		return -1;
	}

	return 0;
}
