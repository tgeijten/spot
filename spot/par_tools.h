#pragma once

#include "search_point.h"
#include "flut/math/vec3_type.hpp"

#define INIT_PAR( _pn_, _ps_, _var_, _default_ ) \
	_var_ = (decltype( _var_ ))::spot::try_get_par( _ps_, #_var_, _pn_, decltype( _var_ )( _default_ ) )

#define INIT_PAR_NAMED( _pn_, _ps_, _var_, _name_, _default_ ) \
	_var_ = (decltype( _var_ ))::spot::try_get_par( _ps_, _name_, _pn_, decltype( _var_ )( _default_ ) )

namespace spot
{
	inline par_value try_get_par( par_io& ps, const string& name, const prop_node& pn, const par_value& def )
	{
		if ( auto* p = pn.try_get_child( name ) )
			return ps.get( name, *p );
		else if ( auto* p = pn.try_get_child_delimited( name ) )
			return ps.get( name, *p );
		else return def;
	}

	template< typename T >
	inline math::vec3_<T> try_get_par( par_io& ps, const string& name, const prop_node& pn, const math::vec3_<T>& def )
	{
		math::vec3_<T> r;
		r.x = T( try_get_par( ps, name + ".x", pn, (par_value)def.x ) );
		r.y = T( try_get_par( ps, name + ".y", pn, (par_value)def.y ) );
		r.z = T( try_get_par( ps, name + ".z", pn, (par_value)def.z ) );
		return r;
	}

	template< typename T, size_t N >
	inline std::array< T, N > try_get_par( par_io& ps, const string& name, const prop_node& pn, const std::array< T, N >& def )
	{
		std::array< T, N > r;
		for ( int i = 0; i < N; ++i )
			r[ i ] = T( try_get_par( ps, name + stringf( ".%d", i ), pn, def[ i ] ) );

		return r;
	}
}
