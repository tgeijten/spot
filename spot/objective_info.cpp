#include "objective_info.h"

#include "flut/container_tools.hpp"
#include "flut/system/assert.hpp"
#include <fstream>

namespace spot
{
	optional_par_value objective_info::try_get( const string& name ) const
	{
		auto it = find( name );
		if ( it != par_infos_.end() )
			return it->mean;
		else return try_get_fixed( name );
	}

	optional_par_value objective_info::try_get_fixed( const string& name ) const
	{
		auto it = fixed_pars_.find( name );
		if ( it != fixed_pars_.end() )
			return it->second;
		else return optional_par_value();
	}

	flut::index_t objective_info::find_best_fitness( const fitness_vec_t& f ) const
	{
		if ( minimize() )
			return std::min_element( f.begin(), f.end() ) - f.begin();
		else return std::max_element( f.begin(), f.end() ) - f.begin();
	}

	par_value objective_info::add( const string& name, par_value mean, par_value std, par_value min, par_value max )
	{
		flut_assert( find( name ) == par_infos_.end() );
		par_infos_.emplace_back( par_info{ name, mean, std, min, max } );
		return par_infos_.back().mean;
	}

	index_t objective_info::find_index( const string& name ) const
	{
		auto it = find( name );
		return it != par_infos_.end() ? it - par_infos_.begin() : no_index;
	}

	size_t objective_info::import_mean_std( const path& filename, bool import_std )
	{
		size_t params_set = 0;
		size_t params_not_found = 0;

		std::ifstream str( filename.str() );
		flut_error_if( !str.good(), "Error opening file: " + filename.str() );
		while ( str.good() )
		{
			std::string name;
			double value, mean, std;
			str >> name >> value >> mean >> std;

			auto iter = find( name );
			if ( iter != end() )
			{
				// read existing parameter, updating mean / std
				iter->mean = mean;
				if ( import_std )
					iter->std = std;
				++params_set;
			}
			else ++params_not_found;
		}
		return params_set;
	}

	size_t objective_info::import_fixed( const path& filename )
	{
		fixed_pars_.clear();

		flut::char_stream str( filename );
		while ( str.good() )
		{
			string name;
			double value, mean, std;
			str >> name >> value >> mean >> std;
			if ( !str.fail() )
			{
				if ( find( name ) == par_infos_.end() )
					fixed_pars_[ name ] = value;
			}
		}
		return fixed_pars_.size();
	}

	void objective_info::set_global_std( double factor, double offset )
	{
		for ( auto& p : par_infos_ )
			p.std = factor * fabs( p.mean ) + offset;
	}

	void objective_info::set_mean_std( const vector< par_value >& mean, const vector< par_value >& std )
	{
		flut_assert( mean.size() == size() && std.size() == size() );
		for ( index_t i = 0; i < size(); ++i )
		{
			par_infos_[ i ].mean = mean[ i ];
			par_infos_[ i ].std = std[ i ];
		}
	}

	objective_info::par_info_vec::const_iterator objective_info::find( const string& name ) const
	{
		return flut::find_if( par_infos_, [&]( const par_info& p ) { return p.name == name; } );
	}

	objective_info::par_info_vec::iterator objective_info::find( const string& name )
	{
		return flut::find_if( par_infos_, [&]( par_info& p ) { return p.name == name; } );
	}
}
