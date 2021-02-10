#include "test_functions.h"

namespace spot
{
	using xo::squared;

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

	double himmelblau( const par_vec& v )
	{
		return squared( squared( v[ 0 ] ) + v[ 1 ] - 11 ) + squared( v[ 0 ] + squared( v[ 1 ] ) - 7 );
	}

	function_objective make_himmelblau_objective()
	{
		return function_objective( himmelblau, 2, 0, 2.5, -5, 5 );
	}

	double rosenbrock( const par_vec& v )
	{
		double sum = 0.0;
		for ( unsigned int i = 0; i < v.size() - 1; i++ )
			sum += 100 * squared( v[ i + 1 ] - squared( v[ i ] ) ) + squared( 1. - v[ i ] );
		return sum;
	}

	function_objective make_rosenbrock_objective( size_t d )
	{
		return function_objective( rosenbrock, d, 2.5, 3.75, -5, 10 );
	}

	double schwefel( const par_vec& v )
	{
		double sum = 0.0;
		for ( index_t i = 0; i < v.size(); ++i )
			sum += v[ i ] * sin( sqrt( fabs( v[ i ] ) ) );
		return 418.9829 * v.size() - sum;
	}

	function_objective make_schwefel_objective( size_t d )
	{
		return function_objective( schwefel, d, 0, 250, -500, 500 );
	}

	double rastrigin( const par_vec& v )
	{
		double sum = 10.0 * v.size();
		for ( index_t i = 0; i < v.size(); ++i )
			sum += xo::squared( v[ i ] ) - 10.0 * cos( 2 * xo::constantsd::pi() * v[ i ] );
		return sum;
	}

	function_objective make_rastrigin_objective( size_t d )
	{
		return function_objective( rastrigin, d, 0, 2.56, -5.12, 5.12 );
	}

	double slow_schwefel( const par_vec& v )
	{
		double result = 0;
		auto dim = v.size();
		size_t evals = 1ULL << dim;
		for ( size_t i = 0; i < evals; ++i )
		{
			par_vec v2 = v;
			for ( size_t d = 0; d < dim; ++d )
				if ( ( 1ULL << d ) & i )
					v2[ d ] *= 1.0001;
			auto f = schwefel( v2 );
			result += f / evals;
		}
		return result;
	}

	function_objective make_slow_schwefel_objective( size_t d )
	{
		return function_objective( slow_schwefel, d, 0, 250, -500, 500 );
	}
}
