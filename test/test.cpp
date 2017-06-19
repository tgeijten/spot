#include "test.h"
#include "flut/system/types.hpp"
#include "flut/system/log_sink.hpp"
#include "flut/system/test_framework.hpp"

using flut::string;

int main( int argc, char* argv[] )
{
	flut::log::stream_sink str( flut::log::info_level, std::cout );
	flut::log::add_sink( &str );

	try
	{
	}
	catch ( std::exception& e )
	{
		std::cout << e.what();				
	}

	return flut::test_framework::get_instance().report();
}
