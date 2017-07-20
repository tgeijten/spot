#pragma once

#include "types.h"

namespace spot
{
	SPOT_API par_value mahalanobis_distance( const par_vec& a, const par_vec& b, const vector< par_vec >& covariance );
	SPOT_API par_value normalized_distance( const par_vec& a, const par_vec& b, const par_vec& var );
}
