#pragma once

#include "search_point.h"
#include "xo/geometry/vec3_type.h"
#include "xo/string/string_tools.h"
#include <array>

#define INIT_PAR( _pn_, _ps_, _var_, _default_ ) \
	_var_ = (decltype( _var_ ))::spot::try_get_par( _ps_, ::xo::tidy_identifier( #_var_ ), _pn_, decltype( _var_ )( _default_ ) )

#define INIT_PAR_NAMED( _pn_, _ps_, _var_, _name_, _default_ ) \
	_var_ = (decltype( _var_ ))::spot::try_get_par( _ps_, _name_, _pn_, decltype( _var_ )( _default_ ) )

namespace spot
{
	template< typename T >
	inline T try_get_par( par_io& ps, const string& name, const prop_node& pn, T def )
	{
		if ( auto* p = pn.try_get_child( name ) )
			return T( ps.get( name, *p ) );
		else if ( auto* p = pn.try_get_query( name ) )
			return T( ps.get( name, *p ) );
		else return def;
	}

	template< typename T >
	inline T try_get_par( par_io& ps, const string& name, const prop_node& pn, T mean, T std, T lower, T upper )
	{
		if ( auto* p = pn.try_get_child( name ) )
			return T( ps.get( name, *p ) );
		else if ( auto* p = pn.try_get_query( name ) )
			return T( ps.get( name, *p ) );
		else return T( ps.get( name, par_value( mean ), std, lower, upper ) );
	}

	template< typename T >
	inline xo::vec3_<T> try_get_par( par_io& ps, const string& name, const prop_node& pn, const xo::vec3_<T>& def )
	{
		if ( auto vec3pn = pn.try_get< xo::vec3_<prop_node> >( name ) )
		{
			// if the node exists, then all vector elements must be defined
			return xo::vec3_<T>(
				ps.get( name + ".x", vec3pn->x ),
				ps.get( name + ".y", vec3pn->y ),
				ps.get( name + ".z", vec3pn->z ) );
		}
		else return def;
	}

	template< typename T, size_t N >
	inline std::array< T, N > try_get_par( par_io& ps, const string& name, const prop_node& pn, const std::array< T, N >& def )
	{
		std::array< T, N > r;
		for ( int i = 0; i < N; ++i )
			r[ i ] = T( try_get_par( ps, name + xo::stringf( ".%d", i ), pn, def[ i ] ) );

		return r;
	}
}
