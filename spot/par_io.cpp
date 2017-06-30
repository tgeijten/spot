#include "par_io.h"

namespace spot
{
	par_value par_io::get( const string& name, par_value mean, par_value std, par_value min, par_value max )
	{
		auto full_name = prefix() + name;

		// check if we already have a value for this name
		if ( auto v = try_get( full_name ) )
			return *v;

		// make a new parameter if we are allowed to
		return add( full_name, mean, std, min, max );
	}

	par_value par_io::get( const string& name, const prop_node& pn )
	{
		auto full_name = prefix() + name;

		// check if we already have a value for this name
		if ( auto val = try_get( full_name ) )
			return *val;

		// interpret the struct
		auto vec = str_to_vec< par_value >( pn.get_value(), 4, " \t," );
		switch ( vec.size() )
		{
		case 0:
		{
			if ( pn.get< bool >( "is_free", true ) )
				return add( full_name, pn.get_any< par_value >( { "mean", "init_mean" } ),
					pn.get_any< par_value >( { "std", "init_std" } ),
					pn.get< par_value >( "min", -1e18 ), pn.get< par_value >( "max", 1e18 ) );
			else return pn.get_any< par_value >( { "mean", "init_mean" } ); // is_free = 0, return mean
		}
		case 1: return vec[ 0 ]; // we have only a value, this is no parameter
		case 2: return add( full_name, vec[ 0 ], vec[ 1 ], vec[ 0 ] - 2 * vec[ 1 ], vec[ 0 ] + 2 * vec[ 1 ] ); // use -2 STD / +2 STD as min / max
		case 4: return add( full_name, vec[ 0 ], vec[ 1 ], vec[ 2 ], vec[ 3 ] );
		default: flut_error( "Invalid number of values" );
		}
	}

	par_value par_io::get_or( const string& name, const prop_node* prop, const par_value& default_value )
	{
		if ( prop )
			return get( name, *prop );
		else return default_value;
	}
}
