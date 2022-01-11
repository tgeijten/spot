#pragma once

#include "function_objective.h"

namespace spot
{
	// sphere function
	// optimum: (0, ..., 0) 
	SPOT_API fitness_t sphere( const par_vec& v );

	// https://en.wikipedia.org/wiki/Himmelblau%27s_function
	// range: [-5, 5], optima: (3.0,2.0) (-2.805118, 3.131312) (-3.779310,-3.283186) (3.584428,-1.848126)
	SPOT_API fitness_t himmelblau( const par_vec& v );

	// https://www.sfu.ca/~ssurjano/rosen.html
	// range: [-5, 10], optimum: (1, ..., 1)
	SPOT_API fitness_t rosenbrock( const par_vec& v );

	// https://www.sfu.ca/~ssurjano/schwef.html
	// range: [-500, 500], optimum: (420.9678, ..., 420.9678)
	SPOT_API fitness_t schwefel( const par_vec& v );

	// https://en.wikipedia.org/wiki/Rastrigin_function
	// range: [-5.12, 5.12], optimum: 0
	SPOT_API fitness_t rastrigin( const par_vec& v );

	// create function objectives
	SPOT_API function_objective make_sphere_objective( size_t d, par_t mean = 0.0, par_t stdev = 1.0 );
	SPOT_API function_objective make_himmelblau_objective();
	SPOT_API function_objective make_rosenbrock_objective( size_t d );
	SPOT_API function_objective make_schwefel_objective( size_t d );
	SPOT_API function_objective make_rastrigin_objective( size_t d );
	SPOT_API std::vector<function_objective> make_objectives( std::initializer_list<size_t> d );
}
