#pragma once

#include <random>
#include <cmath>
#include "xo/system/log.h"
#include "xo/container/container_algorithms.h"

namespace spot
{
	double variance_test( unsigned int seed ) {
		auto rng = std::default_random_engine( seed );
		const int lambda = 6;
		const int mu = lambda / 2;
		const double var_start = 1;
		const double c = 0.1;
		//const double c = std::log( lambda );
		//const double cupd = 0.20282;

		std::vector<double> x( lambda );

		double mean = 2.0;
		double var = var_start;
		double stdev = std::sqrt( var );
		for ( int g = 0; g < 1000; ++g ) {
			auto dist = std::normal_distribution( mean, stdev );
			for ( int i = 0; i < lambda; ++i )
				x[ i ] = dist( rng );
			auto [x_mean, x_var] = xo::mean_var( x );

			auto lambda_var = 0.0;
			for ( int i = 0; i < lambda; ++i )
				lambda_var += xo::squared( x[ i ] - mean );
			lambda_var /= lambda;

			auto mu_var = 0.0;
			auto mu_var_x_mean = 0.0;
			auto x_var_mean = 0.0;
			for ( int i = 0; i < mu; ++i )
			{
				mu_var_x_mean += xo::squared( x[ i ] - x_mean );
				mu_var += xo::squared( x[ i ] - mean );
			}

			mu_var_x_mean /= ( mu - 1 );
			mu_var /= mu;
			auto rel_mu_var = mu_var / lambda_var;
			auto rel_mu_std = std::sqrt( mu_var ) / std::sqrt( lambda_var );

			//var = ( 1 - c ) * var + c * mu_var_x_mean;
			//var = ( 1 - c ) * var + c * mu_var;
			var = ( 1 - c ) * var + c * ( rel_mu_var * var );

			stdev = std::sqrt( var );
			//stdev = ( 1 - c ) * stdev + c * std::sqrt( nvar );
			//var = stdev * stdev;
			//xo_logvar2( var, stdev );
		}
		//xo_logvar4( var / var_start, var_start, var, stdev );
		xo_logvar2( var, stdev );
		return var / var_start;
	}
}
