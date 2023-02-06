#include "test_objectives.h"

namespace spot
{
	fitness_t sphere( const par_vec& v )
	{
		fitness_t sum = 0.0;
		for ( unsigned int i = 0; i < v.size(); ++i )
			sum += xo::squared( v[i] );
		return sum;
	}

	fitness_t ellipsoid( const par_vec& v )
	{
		fitness_t sum = 0.0;
		for ( unsigned int i = 0; i < v.size(); ++i )
			sum += ( i + 1 ) * xo::squared( v[i] );
		return sum;
	}

	fitness_t himmelblau( const par_vec& v )
	{
		return xo::squared( xo::squared( v[0] ) + v[1] - 11 ) + xo::squared( v[0] + xo::squared( v[1] ) - 7 );
	}

	fitness_t rosenbrock( const par_vec& v )
	{
		fitness_t sum = 0.0;
		for ( unsigned int i = 0; i < v.size() - 1; i++ )
			sum += 100 * xo::squared( v[i + 1] - xo::squared( v[i] ) ) + xo::squared( 1. - v[i] );
		return sum;
	}

	fitness_t schwefel( const par_vec& v )
	{
		fitness_t sum = 0.0;
		for ( index_t i = 0; i < v.size(); ++i )
			sum += v[i] * sin( sqrt( fabs( v[i] ) ) );
		return 418.9829 * v.size() - sum;
	}

	fitness_t rastrigin( const par_vec& v )
	{
		fitness_t sum = 10.0 * v.size();
		for ( index_t i = 0; i < v.size(); ++i )
			sum += xo::squared( v[i] ) - 10.0 * cos( 2 * xo::constantsd::pi() * v[i] );
		return sum;
	}

	function_objective make_sphere_objective( size_t d, par_t mean, par_t stdev )
	{
		return function_objective( sphere, d, mean, stdev, -1e9, 1e9, xo::stringf( "sphere-%d", d ) );
	}

	function_objective make_ellipsoid_objective( size_t d, par_t mean, par_t stdev )
	{
		return function_objective( ellipsoid, d, mean, stdev, -1e9, 1e9, xo::stringf( "ellipsoid-%d", d ) );
	}

	function_objective make_himmelblau_objective()
	{
		return function_objective( himmelblau, 2, 0, 2.5, -5, 5, "himmelblau" );
	}

	function_objective make_rosenbrock_objective( size_t d )
	{
		return function_objective( rosenbrock, d, 2.5, 1.0, -5, 10, xo::stringf( "rosenbrock-%d", d ) );
	}

	function_objective make_schwefel_objective( size_t d )
	{
		return function_objective( schwefel, d, 0, 100, -500, 500, xo::stringf( "schwefel-%d", d ) );
	}

	function_objective make_rastrigin_objective( size_t d )
	{
		return function_objective( rastrigin, d, 0, 1.0, -5.12, 5.12, xo::stringf( "rastrigin-%d", d ) );
	}

	std::vector<function_objective> make_objectives( std::initializer_list<size_t> dims )
	{
		std::vector<function_objective> objs;
		for ( auto& d : dims )
		{
			objs.emplace_back( make_sphere_objective( d ) );
			objs.emplace_back( make_rosenbrock_objective( d ) );
			objs.emplace_back( make_rastrigin_objective( d ) );
			objs.emplace_back( make_schwefel_objective( d ) );
		}
		return objs;
	}
}
