#include "static_par_set.h"

#include "flut/char_stream.hpp"
#include "flut/system/types.hpp"

namespace spot
{
	size_t static_par_set::load( const path& filename )
	{
		flut::char_stream str( filename );
		while ( str.good() )
		{
			string name;
			double value, mean, std;
			str >> name >> value >> mean >> std;
			if ( !str.fail() )
				values_[ name ] = value;
		}

		return values_.size();
	}

	size_t static_par_set::merge( const path& filename, bool overwrite )
	{
		size_t read = 0;
		flut::char_stream str( filename );
		while ( str.good() )
		{
			string name;
			double value, mean, std;
			str >> name >> value >> mean >> std;
			if ( !str.fail() )
			{
				if ( overwrite || values_.find( name ) == values_.end() )
				{
					values_[ name ] = value;
					++read;
				}
			}
		}

		return read;
	}

	optional_par_value static_par_set::try_get( const string& name ) const
	{
		auto it = values_.find( name );
		if ( it != values_.end() )
			return it->second;
		else return optional_par_value();
	}

	par_value static_par_set::add( const string& name, par_value mean, par_value std, par_value min, par_value max )
	{
		return values_[ name ] = mean;
	}
}
