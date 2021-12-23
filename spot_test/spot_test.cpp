//#include "xo/system/test_main.h"

#include "xo/system/log.h"
#include "xo/system/log_sink.h"
#include "compare_optimizers.h"

int main( int argc, char** argv )
{
	xo::log::console_sink log_sink( xo::log::level::trace );

	try
	{
		spot::compare_optimizers();
	}
	catch ( std::exception& e )
	{
		xo::log::critical( "CRITICAL ERROR: ", e.what() );
		return -1;
	}

	return 0;
}
