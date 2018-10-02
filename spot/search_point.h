#pragma once

#include "xo/utility/types.h"
#include "xo/container/flat_map.h"
#include "xo/container/prop_node.h"

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

		search_point& operator=( const search_point& other ) { xo_assert( info_.dim() == other.info_.dim() ); values_ = other.values_; return *this; }

		virtual size_t dim() const override { return info_.dim(); }
		virtual optional< par_value > try_get( const string& full_name ) const override;
		virtual par_value add( const par_info& pi ) override;

		const par_value& operator[]( index_t i ) const { return values_[ i ]; }
		par_value& operator[]( index_t i ) { return values_[ i ]; }
		using par_io::get;

		const objective_info& info() const { return info_; }
		size_t size() const { return info_.size(); }
		std::pair< size_t, size_t > import_values( const path& filename );

		void set_values( const par_vec& values );
		const par_vec& values() const { return values_; }

		const xo::optional< fitness_t >& fitness() const { return fitness_; }
		void set_fitness( fitness_t f ) { fitness_ = f; }

	private:
		void round_values();
		par_value rounded( par_value );

		const objective_info& info_;
		par_vec values_;
		xo::optional< fitness_t > fitness_;
	};

	using search_point_vec = std::vector< search_point >;

	SPOT_API std::pair< par_vec, par_vec > compute_mean_std( const search_point_vec& pop );
	SPOT_API std::ostream& operator<<( std::ostream& str, const search_point& ps );
}

#if defined(_MSC_VER)
#	pragma warning( pop )
#endif
