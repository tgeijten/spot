#include "par_io.h"
#include <cctype>
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

		// if value starts with a letter: must be a reference to another parameter
		if ( !pn.raw_value().empty() && std::isalpha( pn.raw_value().front() ) )
		{
			auto par_ref = pn.get<string>();
			auto val = try_get( par_ref );
			xo_error_if( !val, "Could not find " + par_ref );
			return *val;
		}

		// parameter not found, try adding a new one
		par_info pi = par_info( full_name, pn );
		if ( pi.is_constant() )
			return pi.mean;
		else return add( pi );
	}

	par_value par_io::try_get( const string& name, const prop_node& parent_pn, const string& key, const par_value& default_value )
	{
		if ( auto* pn = parent_pn.try_get_child( key ) )
			return get( name, *pn );
		else return default_value;
	}
}
