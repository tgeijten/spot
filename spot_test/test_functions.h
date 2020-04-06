#pragma once

#include "spot/spot_types.h"
#include "spot/function_objective.h"
#include "xo/numerical/math.h"

namespace spot
{
	double cigtab_c( double const *x, int N );
	double cigtab( const par_vec& x );

	// https://en.wikipedia.org/wiki/Himmelblau%27s_function
	// range: [-5, 5], optima: (3.0,2.0) (-2.805118, 3.131312) (-3.779310,-3.283186) (3.584428,-1.848126)
	double himmelblau( const par_vec& v );
	function_objective make_himmelblau_objective();

	// https://www.sfu.ca/~ssurjano/rosen.html
	// range: [-5, 10], optimum: (1, ..., 1)
	double rosenbrock( const par_vec& v );
	function_objective make_rosenbrock_objective( size_t d );

	// https://www.sfu.ca/~ssurjano/schwef.html
	// range: [-500, 500], optimum: (420.9678, ..., 420.9678)
	double schwefel( const par_vec& v );
	function_objective make_schwefel_objective( size_t d );
	function_objective make_slow_schwefel_objective( size_t d );

	// https://en.wikipedia.org/wiki/Rastrigin_function
	// range: [-5.12, 5.12], optimum: 0
	double rastrigin( const par_vec& v );
	function_objective make_rastrigin_objective( size_t d );
}
