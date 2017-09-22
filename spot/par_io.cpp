#include "par_io.h"
#include "flut/math/bounds.hpp"

namespace spot
{
	const par_value default_std_factor = 0.1;

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

		// check if the prop_node has children
		optional< par_value > mean, std, min, max;
		if ( pn.size() > 0 )
		{
			if ( pn.get< bool >( "is_free", true ) )
			{
				mean = pn.get_any< par_value >( { "mean", "init_mean" } );
				std = pn.get_any< par_value >( { "std", "init_std" } );
				min = pn.get< par_value >( "min", -1e18 );
				max = pn.get< par_value >( "max", 1e18 );
			}
			else return pn.get_any< par_value >( { "mean", "init_mean" } ); // is_free = 0, return mean
		}

		// parse the string, format mean~std[min,max]
		char_stream str( pn.get_value().c_str() );
		while ( str.good() )
		{
			char c = str.peekc();
			if ( c == '~' )
			{
				str.getc();
				str >> std;
			}
			else if ( c == '[' )
			{
				str.getc();
				str >> min;
				flut_error_if( str.getc() != ',', "Error parsing parameter '" + name + "': expected ','" );
				str >> max;
				flut_error_if( str.getc() != ']', "Error parsing parameter '" + name + "': expected ']'" );
			}
			else // just a value, interpret as mean
			{
				flut_error_if( mean, "Error parsing parameter '" + name + "': mean already defined" );
				str >> mean;
			}
		}

		// do some sanity checking and fixing
		flut_error_if( min && max && ( *min > *max ), "Error parsing parameter '" + name + "': min > max" );
		flut_error_if( !mean && !std && !min && !max, "Error parsing parameter '" + name + "': no parameter defined" );
		if ( mean && !std && !min && !max )
			return *mean; // just a value

		if ( std && !mean )
		{ mean = std; std = default_std_factor * abs( *mean ); }
		if ( !std && min && max )
			std = ( *max - *min ) / 4;
		if ( !mean && min && max )
			mean = *min + ( *max - *min ) / 2;
		if ( !min ) min = -1e18;
		if ( !max ) max = 1e18;

		return add( full_name, *mean, *std, *min, *max );
	}

	spot::par_value par_io::try_get( const string& name, const prop_node& parent_pn, const string& key, const par_value& default_value )
	{
		if ( auto* pn = parent_pn.try_get_child( key ) )
			return get( name, *pn );
		else return default_value;
	}
}
