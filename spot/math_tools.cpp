#include "math_tools.h"
#include "xo/system/assert.h"
#include "xo/numerical/math.h"

namespace spot
{
	par_t mahalanobis_distance( const par_vec& a, const par_vec& b, const vector< par_vec >& covariance )
	{
		xo_assert( a.size() == b.size() && b.size() == covariance.size() );
		par_t dist = 0;
		for ( index_t i = 0; i < a.size(); ++i )
			dist += xo::squared( a[ i ] - b[ i ] ) / covariance[ i ][ i ];
		return sqrt( dist );
	}

	par_t normalized_distance( const par_vec& a, const par_vec& b, const par_vec& var )
	{
		xo_assert( a.size() == b.size() && b.size() == var.size() );
		par_t dist = 0;
		for ( index_t i = 0; i < a.size(); ++i )
			dist += xo::squared( a[ i ] - b[ i ] ) / var[ i ];
		return sqrt( dist );
	}
}
