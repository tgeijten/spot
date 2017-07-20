#include "test.h"
#include "flut/system/types.hpp"
#include "flut/system/log_sink.hpp"
#include "flut/system/test_framework.hpp"
#include "test_functions.h"
#include "optimization_test.h"

using flut::string;

int main( int argc, char* argv[] )
{
	flut::log::stream_sink str( flut::log::info_level, std::cout );
	flut::log::add_sink( &str );

	try
	{
		spot::multi_optimizer_test();
	}
	catch ( std::exception& e )
	{
		std::cout << e.what();				
	}

#ifdef _DEBUG
	flut::wait_for_key();
#endif

	return flut::test_framework::get_instance().report();
}
