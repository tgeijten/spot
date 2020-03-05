#pragma once

#include "par_io.h"
#include "xo/container/flat_map.h"
#include "xo/filesystem/path.h"

namespace spot
{
	class SPOT_API static_par_set : public par_io
	{
	public:
		static_par_set() : par_io() {}
		static_par_set( const path& filename ) : par_io() { load( filename ); }

		virtual size_t dim() const override { return values_.size(); }
		virtual xo::optional< par_t > try_get( const string& name ) const override;
		virtual par_t add( const par_info& pi ) override;

		size_t load( const path& filename );
		size_t merge( const path& filename, bool overwrite );

	private:
		xo::flat_map< string, par_t > values_;
	};
}
