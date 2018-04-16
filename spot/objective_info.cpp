#include "objective_info.h"

#include "xo/container/container_tools.h"
#include "xo/system/assert.h"
#include <fstream>
#include "xo/filesystem/filesystem.h"
#include "xo/system/log.h"

namespace spot
{
	optional_par_value objective_info::try_get( const string& name ) const
	{
		auto it = find( name );
		if ( it != par_infos_.end() )
			return it->mean;
		else return try_get_locked( name );
	}

	optional_par_value objective_info::try_get_locked( const string& name ) const
	{
		auto it = locked_pars_.find( name );
		if ( it != locked_pars_.end() )
			return it->second;
		else return optional_par_value();
	}

	xo::index_t objective_info::find_best_fitness( const fitness_vec_t& f ) const
	{
		if ( minimize() )
			return std::min_element( f.begin(), f.end() ) - f.begin();
		else return std::max_element( f.begin(), f.end() ) - f.begin();
	}

	par_value objective_info::add( const string& name, par_value mean, par_value std, par_value min, par_value max )
	{
		xo_error_if( find( name ) != par_infos_.end(), "Parameter already exists: " + name );
		xo_error_if( std <= 0, "Initial STD must be > 0: " + name + " " + xo_varstr( std ) );
		par_infos_.emplace_back( par_info{ name, mean, std, min, max } );
		return par_infos_.back().mean;
	}

	index_t objective_info::find_index( const string& name ) const
	{
		auto it = find( name );
		return it != par_infos_.end() ? it - par_infos_.begin() : no_index;
	}

	std::pair< size_t, size_t > objective_info::import_mean_std( const path& filename, bool import_std, double std_factor, double std_offset )
	{
		size_t params_set = 0;
		size_t params_not_found = 0;

		std::ifstream str( xo::find_file( filename ).str() );
		xo_error_if( !str.good(), "Error opening file: " + filename.str() );
		while ( str.good() )
		{
			std::string name;
			double value, mean, std;
			str >> name >> value >> mean >> std;

			if ( !str.fail() )
			{
				if ( std == 0 )
				{
					if ( lock_parameter( name, value ) )
						++params_set;
					else ++params_not_found;
				}
				else if ( auto p = try_find( name ) )
				{
					// read existing parameter, updating mean / std
					p->mean = mean;
					if ( import_std )
						p->std = std_offset + std_factor * std;
					else if ( std_factor != 1.0 ) // set std to factor of mean
						p->std = std_offset + std_factor * p->mean;
					++params_set;
				}
				else
				{
					log::trace( "Ignored parameter ", name );
					++params_not_found;
				}
			}
		}

		return { params_set, params_not_found };
	}

	std::pair< size_t, size_t > objective_info::import_locked( const path& filename )
	{
		size_t params_locked = 0;
		size_t params_not_found = 0;

		xo::char_stream str( filename );
		while ( str.good() )
		{
			string name;
			double value, mean, std;
			str >> name >> value >> mean >> std;
			if ( !str.fail() )
			{
				if ( lock_parameter( name, value ) )
					++params_locked;
				else ++params_not_found;
			}
		}
		return { params_locked, params_not_found };
	}

	void objective_info::set_global_std( double factor, double offset )
	{
		for ( auto& p : par_infos_ )
			p.std = factor * fabs( p.mean ) + offset;
	}

	void objective_info::set_mean_std( const par_vec& mean, const par_vec& std )
	{
		xo_assert( mean.size() == size() && std.size() == size() );
		for ( index_t i = 0; i < size(); ++i )
		{
			par_infos_[ i ].mean = mean[ i ];
			par_infos_[ i ].std = std[ i ];
		}
	}

	bool objective_info::is_feasible( const par_vec& vec ) const
	{
		for ( index_t i = 0; i < size(); ++i )
			if ( vec[ i ] > par_infos_[ i ].max || vec[ i ] < par_infos_[ i ].min )
				return false;
		return true;
	}

	void objective_info::clamp( par_vec& vec ) const
	{
		for ( index_t i = 0; i < size(); ++i )
		{
			auto& pi = par_infos_[ i ];
			if ( !xo::is_between( vec[ i ], pi.min, pi.max ) )
			{
				xo::log::debug( "Clamping parameter ", pi.name, "; value=", vec[ i ], " min=", pi.min, " max=", pi.max );
				xo::clamp( vec[ i ], pi.min, pi.max );
			}
		}
	}

	std::vector< objective_info::par_info >::const_iterator objective_info::find( const string& name ) const
	{
		return xo::find_if( par_infos_, [&]( const par_info& p ) { return p.name == name; } );
	}

	std::vector< objective_info::par_info >::iterator objective_info::find( const string& name )
	{
		return xo::find_if( par_infos_, [&]( par_info& p ) { return p.name == name; } );
	}

	const spot::objective_info::par_info* objective_info::try_find( const string& name ) const
	{
		auto it = find( name );
		return it != par_infos_.end() ? &*it : nullptr;
	}

	spot::objective_info::par_info* objective_info::try_find( const string& name )
	{
		auto it = find( name );
		return it != par_infos_.end() ? &*it : nullptr;
	}

	bool objective_info::lock_parameter( const string& name, par_value value )
	{
		auto iter = find( name );
		if ( iter != par_infos_.end() )
		{
			// convert parameter to locked
			locked_pars_[ iter->name ] = value;
			par_infos_.erase( iter ); // remove existing parameter
			return true;
		}
		else
		{
			// see if it's already a locked parameter
			auto it2 = locked_pars_.find( name );
			if ( it2 != locked_pars_.end() )
			{
				it2->second = value; // overwrite value
				return true;
			}
		}
		return false;
 	}
}
