#include "test.h"
#include "math_test.h"
#include "buffer_test.h"
#include "flut/prop_node.hpp"
#include "flut/system_tools.hpp"
#include "prop_node_test.h"
#include "flut/stopwatch.hpp"
#include "flut/system/test_framework.hpp"
#include "string_test.h"
#include "flut/dictionary.hpp"
#include "flut/system/log_sink.hpp"
#include "factory_test.h"

using flut::string;

int main( int argc, char* argv[] )
{
	flut::log::stream_sink str( flut::log::info_level, std::cout );
	flut::log::add_sink( &str );

	try
	{
		flut_logvar2( flut::math::constants<double>::epsilon(), flut::math::constants<double>::relaxed_epsilon() );
		flut_logvar2( flut::math::constants<float>::epsilon(), flut::math::constants<float>::relaxed_epsilon() );

		flut::stopwatch sw;

		flut::factory_test();
		sw.add_measure( "factory" );

		flut::prop_node_test();
		sw.add_measure( "prop_node" );

		flut::string_test();
		sw.add_measure( "string" );

		flut::buffer_test();
		sw.add_measure( "buffer" );

		flut::math::function_test();
		flut::math::angle_test();
		flut::math::vec_quat_test();
		sw.add_measure( "math" );

		auto pn = sw.get_report();
		flut::log::info( pn );
	}
	catch ( std::exception& e )
	{
		std::cout << e.what();				
	}

	return flut::test_framework::get_instance().report();
}
