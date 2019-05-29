#include "par_info.h"

#include <cctype>
#include "xo/serialization/char_stream.h"
#include "xo/numerical/constants.h"
#include "xo/numerical/math.h"

namespace spot
{
	par_info::par_info( string pname, par_value pmean, par_value pstd, par_value pmin, par_value pmax ) :
		name( pname ), mean( pmean ), std( pstd ), min( pmin ), max( pmax )
	{}

	par_info::par_info( string full_name, const prop_node& pn ) :
	name( full_name ),
	mean( xo::constants<par_value>::NaN() ),
	std( xo::constants<par_value>::NaN() ),
	min( -1e12 ),
	max( 1e12 )
	{
		// check if the prop_node has children
		if ( pn.size() > 0 )
		{
			mean = pn.get_any< par_value >( { "mean", "init_mean" } );
			std = pn.get_any< par_value >( { "std", "init_std" } );
			min = pn.get< par_value >( "min", -1e12 );
			max = pn.get< par_value >( "max", 1e12 );
		}
		else
		{
			// parse the string, format mean~std[min,max]
			// TODO: use string_view instead of char_stream?
			xo::char_stream str( pn.get<string>() );
			while ( str.good() )
			{
				char c = str.peekc();
				if ( str.good() )
				{
					if ( c == '~' )
					{
						xo_error_if( !std::isnan( std ), "Error parsing parameter '" + full_name + "': Standard deviation already set" );
						str.getc();
						str >> std;
					}
					else if ( c == '[' || c == '<' || c == '(' )
					{
						str.getc();
						str >> min;
						xo_error_if( str.fail(), "Error parsing parameter '" + full_name + "': Could not read minimum value from range" );
						xo_error_if( str.getc() != ',', "Error parsing parameter '" + full_name + "': expected ','" );
						str >> max;
						xo_error_if( str.fail(), "Error parsing parameter '" + full_name + "': Could not read maximum value from range" );
						char c2 = str.getc();
						if ( ( c == '[' && c2 != ']' ) || ( c == '<' && c2 != '>' ) || ( c == '(' && c2 != ')' ) )
							xo_error( "Error parsing parameter '" + full_name + "': opening bracket " + c + " does not match closing bracket " + c2 );
					}
					else if ( std::isdigit( c ) )// just a value, interpret as mean
					{
						xo_error_if( !std::isnan( std ), "Error parsing parameter '" + full_name + "': mean already defined" );
						str >> mean;
						xo_error_if( str.fail(), "Error parsing parameter '" + full_name + "': Could not read mean value" );
					}
					else xo_error( "Error parsing parameter '" + full_name + "': unexpected character '" + c + "'" );
				}
			}
		}

		// do some sanity checking and fixing
		xo_error_if( min >= max, "Error parsing parameter '" + full_name + "': min >= max" );
		xo_error_if( std::isnan( mean ) && std == 0 && min == default_lower_boundaray && max == default_upper_boundaray, "Error parsing parameter '" + full_name + "': no parameter defined" );

		if ( std != 0 && std::isnan( mean ) ) { 
			// using ~value notation, derive mean and std
			mean = std;
			std = xo::max( default_std_factor * abs( mean ), default_std_minimum );
		}
		if ( min != default_lower_boundaray && max != default_upper_boundaray ) {
			// min and max are set, derive mean and / or std
			if ( std::isnan( mean ) ) mean = min + ( max - min ) / 2;
			if ( std == 0 ) std = ( max - min ) / 4;
		}
	}
}
