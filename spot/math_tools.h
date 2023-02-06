#pragma once

#include "spot_types.h"
#include "xo/system/assert.h"
#include "xo/numerical/constants.h"
#include <cmath>

namespace spot
{
	SPOT_API par_t mahalanobis_distance( const par_vec& a, const par_vec& b, const vector<par_vec>& covariance );
	SPOT_API par_t normalized_distance( const par_vec& a, const par_vec& b, const par_vec& var );

	inline void update( par_t& v, const par_t& nv, const par_t& rate ) {
		v = ( 1 - rate ) * v + rate * nv;
	}
	inline par_t length( const par_vec& vec ) {
		par_t sum = 0;
		for ( const auto& v : vec )
			sum += v * v;
		return std::sqrt( sum );
	}

	inline par_t dot_product( const par_vec& v1, const par_vec& v2 ) {
		xo_assert( v1.size() == v2.size() );
		par_t sum = 0;
		for ( index_t i = 0; i < v1.size(); ++i )
			sum += v1[i] * v2[i];
		return sum;
	}

	inline par_vec vec_sub( par_vec a, const par_vec& b ) {
		xo_assert( a.size() == b.size() );
		for ( index_t i = 0; i < a.size(); ++i )
			a[i] -= b[i];
		return a;
	}

	inline par_vec vec_add( par_vec a, const par_vec& b ) {
		xo_assert( a.size() == b.size() );
		for ( index_t i = 0; i < a.size(); ++i )
			a[i] += b[i];
		return a;
	}

	inline par_vec vec_mul( par_t s, par_vec a ) {
		for ( index_t i = 0; i < a.size(); ++i )
			a[i] *= s;
		return a;
	}

	inline par_vec projected( const par_vec& a, const par_vec& ab, const par_vec& p ) {
		auto ap = vec_sub( p, a );
		auto dot_ab = dot_product( ab, ab );
		if ( dot_ab > xo::num<par_t>::ample_epsilon ) {
			auto t = dot_product( ap, ab ) / dot_ab;
			return vec_add( a, vec_mul( t, ab ) );
		}
		else return a;
	}
}
