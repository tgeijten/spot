#include "test.h"
#include "xo/system/log_sink.h"
#include "xo/diagnose/test_framework.h"
#include "test_functions.h"
#include "optimization_test.h"
#include <iostream>

using xo::string;

int main( int argc, char* argv[] )
{
	xo::log::stream_sink str( xo::log::info_level, std::cout );
	xo::log::add_sink( &str );

	try
	{
		spot::multi_optimizer_test();
	}
	catch ( std::exception& e )
	{
		std::cout << e.what();				
	}

#ifdef _DEBUG
	xo::wait_for_key();
#endif

	return 0;
}
