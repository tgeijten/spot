#pragma once

#include <random>
#include "xo/system/log.h"

namespace spot
{
	double variance_test( unsigned int seed ) {
		auto rng = std::default_random_engine( seed );
		const int lambda = 10;
		const double var_start = 0.1;
		const double c = 0.2;
		//const double cupd = 0.20282;

		std::vector<double> samples( lambda );

		double mean = 2.0;
		double var = var_start;
		double stdev = std::sqrt( var );
		for ( int g = 0; g < 1000; ++g ) {
			auto dist = std::normal_distribution( mean, stdev );
			for ( int i = 0; i < lambda; ++i )
				samples[ i ] = dist( rng );

			auto smean = xo::average( samples );
			auto svar1 = 0.0, svar2 = 0.0;
			for ( auto& v : samples )
			{
				svar1 += xo::squared( v - smean );
				svar2 += xo::squared( v - mean );
			}
			svar1 /= ( lambda - 1 );
			svar2 /= lambda;

			var = ( 1 - c ) * var + c * svar1;
			//var = ( 1 - c ) * var + c * svar2;
			//var = var + c * ( nvar - var );
			stdev = std::sqrt( var );
			//stdev = ( 1 - c ) * stdev + c * std::sqrt( nvar );
			//var = stdev * stdev;
			//xo_logvar2( var, stdev );
		}
		xo_logvar4( var / var_start, var_start, var, stdev );
		return var / var_start;
	}
}
