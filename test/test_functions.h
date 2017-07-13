#pragma once

#include "spot/types.h"
#include "flut/math/math.hpp"
#include "spot/objective.h"

namespace spot
{
	using flut::math::squared;

	double cigtab_c( double const *x, int N )
	{
		double sum = 1e4*x[ 0 ] * x[ 0 ] + 1e-4*x[ 1 ] * x[ 1 ];
		for ( int i = 2; i < N; ++i )
			sum += x[ i ] * x[ i ];
		return sum;
	}

	double cigtab( const par_vec& x )
	{
		double sum = 1e4*x[ 0 ] * x[ 0 ] + 1e-4*x[ 1 ] * x[ 1 ];
		for ( int i = 2; i < x.size(); ++i )
			sum += x[ i ] * x[ i ];
		return sum;
	}

	// https://www.sfu.ca/~ssurjano/rosen.html
	// range: [-5, 10], optimum: (1, ..., 1)
	double rosenbrock( const par_vec& v )
	{
		double sum = 0.0;
		for ( unsigned int i = 0; i < v.size() - 1; i++ )
			sum += 100 * squared( v[ i + 1 ] - squared( v[ i ] ) ) + squared( 1. - v[ i ] );
		return sum;
	}

	// https://www.sfu.ca/~ssurjano/schwef.html
	// range: [-500, 500], optimum: (420.9678, ..., 420.9678)
	double schwefel( const par_vec& v )
	{
		double sum = 0.0;
		for ( index_t i = 1; i <= v.size(); ++i )
			sum += v[ i ] * sin( sqrt( fabs( v[ i ] ) ) );
		return 418.9829 * v.size() - sum;
	}

	function_objective make_schwefel_objective( size_t d ) { return function_objective( d, schwefel, par_vec( 0 ), par_vec( 250 ), par_vec( -500 ), par_vec( 500 ) ); }
}
