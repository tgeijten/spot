#include "static_par_set.h"

#include "xo/xo_types.h"
#include "xo/serialization/char_stream.h"
#include "xo/filesystem/filesystem.h"

namespace spot
{
	size_t static_par_set::load( const path& filename )
	{
		xo::char_stream str( xo::load_string( filename ) );
		while ( str.good() )
		{
			string name;
			double value, mean, std;
			str >> name >> value >> mean >> std;
			if ( !str.fail() )
				values_[name] = value;
		}

		return values_.size();
	}

	size_t static_par_set::merge( const path& filename, bool overwrite )
	{
		size_t read = 0;
		xo::char_stream str( xo::load_string( filename ) );
		while ( str.good() )
		{
			string name;
			double value, mean, std;
			str >> name >> value >> mean >> std;
			if ( !str.fail() )
			{
				if ( overwrite || values_.find( name ) == values_.end() )
				{
					values_[name] = value;
					++read;
				}
			}
		}

		return read;
	}

	xo::optional< par_t > static_par_set::try_get( const string& name ) const
	{
		auto it = values_.find( name );
		if ( it != values_.end() )
			return it->second;
		else return {};
	}

	par_t static_par_set::add( const par_info& pi )
	{
		return values_[pi.name] = pi.mean;
	}
}
