#include "tools.h"
#include "flut/system/assert.hpp"
#include "flut/math/math.hpp"

namespace spot
{
	par_value mahalanobis_distance( const par_vec& a, const par_vec& b, const vector< par_vec >& covariance )
	{
		flut_assert( a.size() == b.size() && b.size() == covariance.size() );
		par_value dist = 0;
		for ( index_t i = 0; i < a.size(); ++i )
			dist += math::squared( a[ i ] - b[ i ] ) / covariance[ i ][ i ];
		return sqrt( dist );
	}

	par_value normalized_distance( const par_vec& a, const par_vec& b, const par_vec& var )
	{
		flut_assert( a.size() == b.size() && b.size() == var.size() );
		par_value dist = 0;
		for ( index_t i = 0; i < a.size(); ++i )
			dist += math::squared( a[ i ] - b[ i ] ) / var[ i ];
		return sqrt( dist );
	}
}
