#pragma once

#include "flut/system/types.hpp"
#include "flut/flat_map.hpp"
#include "flut/prop_node.hpp"
#include "objective_info.h"
#include "par_io.h"
#include "types.h"

#if defined(_MSC_VER)
#	pragma warning( push )
#	pragma warning( disable: 4251 )
#endif

namespace spot
{
	class SPOT_API search_point : public par_io
	{
	public:
		search_point( const objective_info& inf );
		search_point( const objective_info& inf, const par_vec& values );
		search_point( const objective_info& inf, par_vec&& values );
		search_point( const objective_info& inf, const path& filename );

		search_point& operator=( const search_point& other ) { flut_assert( info_.dim() == other.info_.dim() ); values_ = other.values_; return *this; }

		virtual size_t dim() const override { return info_.dim(); }
		virtual optional_par_value try_get( const string& full_name ) const override;
		virtual par_value add( const string& name, par_value mean, par_value std, par_value min, par_value max ) override;

		const par_value& operator[]( index_t i ) const { return values_[ i ]; }
		par_value& operator[]( index_t i ) { return values_[ i ]; }
		using par_io::get;

		const objective_info& info() const { return info_; }
		size_t size() const { return info_.size(); }
		size_t import_values( const path& filename );

		friend SPOT_API std::ostream& operator<<( std::ostream& str, const search_point& ps );

		void set_values( const par_vec& values );
		const par_vec& values() const { return values_; }

	private:
		void round_values();
		par_value rounded( par_value );

		const objective_info& info_;
		par_vec values_;
	};

	using search_point_vec = vector< search_point >;
	pair< par_vec, par_vec > SPOT_API compute_mean_std( const search_point_vec& pop );
}

#if defined(_MSC_VER)
#	pragma warning( pop )
#endif
