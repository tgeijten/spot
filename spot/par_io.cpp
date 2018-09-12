#include "par_io.h"
#include "xo/serialization/char_stream.h"

namespace spot
{
	par_value par_io::get( const string& name, par_value mean, par_value std, par_value min, par_value max )
	{
		auto full_name = prefix() + name;

		// check if we already have a value for this name
		if ( auto v = try_get( full_name ) )
			return *v;

		// make a new parameter if we are allowed to
		return add( par_info( full_name, mean, std, min, max ) );
	}

	par_value par_io::get( const string& name, const prop_node& pn )
	{
		auto full_name = prefix() + name;

		// check if we already have a value for this name
		if ( auto val = try_get( full_name ) )
			return *val;

		// see if this is a reference to another parameter
		if ( !pn.get_value().empty() && pn.get_value().front() == '@' )
		{
			auto val = try_get( pn.get_value().substr( 1 ) );
			xo_error_if( !val, "Could not find " + pn.get_value() );
			return *val;
		}

		// parameter not found, try adding a new one
		par_info pi = par_info( full_name, pn );
		if ( pi.is_valid() )
			return add( pi );
		else return pi.mean;
	}

	spot::par_value par_io::try_get( const string& name, const prop_node& parent_pn, const string& key, const par_value& default_value )
	{
		if ( auto* pn = parent_pn.try_get_child( key ) )
			return get( name, *pn );
		else return default_value;
	}
}
