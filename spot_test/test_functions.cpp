#include "test_functions.h"

namespace spot
{
	double cigtab_c( double const* x, int N )
	{
		double sum = 1e4 * x[0] * x[0] + 1e-4 * x[1] * x[1];
		for ( int i = 2; i < N; ++i )
			sum += x[i] * x[i];
		return sum;
	}

	double cigtab( const par_vec& x )
	{
		double sum = 1e4 * x[0] * x[0] + 1e-4 * x[1] * x[1];
		for ( int i = 2; i < x.size(); ++i )
			sum += x[i] * x[i];
		return sum;
	}
}
