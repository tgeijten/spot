#pragma once

#include "par_io.h"
#include "flut/flat_map.hpp"

#if defined(_MSC_VER)
#	pragma warning( push )
#	pragma warning( disable: 4251 )
#endif

namespace spot
{
	class SPOT_API static_par_set : public par_io
	{
	public:
		static_par_set() : par_io() {}
		static_par_set( const path& filename ) : par_io() { load( filename ); }

		virtual size_t dim() const override { return values_.size(); }
		virtual optional_par_value try_get( const string& name ) const override;
		virtual par_value add( const string& name, par_value mean, par_value std, par_value min = -1e15, par_value max = 1e15 ) override;

		size_t load( const path& filename );
		size_t merge( const path& filename, bool overwrite );

	private:
		flat_map< string, par_value > values_;
	};
}

#if defined(_MSC_VER)
#	pragma warning( pop )
#endif
