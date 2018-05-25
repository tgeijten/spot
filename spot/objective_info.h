#pragma once

#include "par_io.h"

#include <vector>

#include "xo/system/platform.h"
#include "xo/utility/types.h"
#include "xo/container/flat_map.h"
#include "xo/filesystem/path.h"
#include "xo/numerical/constants.h"

#if defined(_MSC_VER)
#	pragma warning( push )
#	pragma warning( disable: 4251 )
#endif

namespace spot
{
	class SPOT_API objective_info : public par_io
	{
	public:
		objective_info( bool min = true ) : minimize_( min ), target_fitness_( 0 ) {}

		struct par_info
		{
			string name;
			par_value mean;
			par_value std;
			par_value min;
			par_value max;
		};

		virtual size_t dim() const override { return par_infos_.size(); }
		virtual par_value add( const string& name, par_value mean, par_value std, par_value min = -1e15, par_value max = 1e15 ) override;
		using par_io::add;
		virtual optional_par_value try_get( const string& name ) const override;
		optional_par_value try_get_locked( const string& name ) const;
		const string& name() const { return name_; }

		fitness_t target_fitness() const { return target_fitness_; }
		void set_target_fitness( fitness_t f ) { target_fitness_ = f; }

		/// minimize / maximize
		bool minimize() const { return minimize_; }
		bool maximize() const { return !minimize_; }
		bool is_better( fitness_t a, fitness_t b ) const { return minimize() ? a < b : a > b; }
		index_t find_best_fitness( const fitness_vec_t& f ) const;
		template< typename T > T worst() const { return minimize() ? max<T>() : lowest<T>(); }
		template< typename T > T best() const { return minimize() ? lowest<T>() : max<T>(); }
		fitness_t worst_fitness() const { return worst< fitness_t >(); }
		fitness_t best_fitness() const { return best< fitness_t >(); }
		void set_minimize( bool m ) { minimize_ = m; }

		/// access by index
		const par_info& operator[]( index_t i ) const { return par_infos_[ i ]; }

		/// access by name
		index_t find_index( const string& name ) const;

		/// iterator access
		std::vector< par_info >::const_iterator begin() const { return par_infos_.begin(); }
		std::vector< par_info >::const_iterator end() const { return par_infos_.end(); }

		/// properties
		size_t size() const { return par_infos_.size(); }
		bool empty() const { return par_infos_.empty(); }

		/// import / export
		std::pair< size_t, size_t > import_mean_std( const path& filename, bool import_std, double std_factor = 1.0, double std_offset = 0.0 );
		std::pair< size_t, size_t > import_locked( const path& filename );
		void set_global_std( double factor, double offset );
		void set_mean_std( const par_vec& mean, const par_vec& std );
		void set_name( const string& name ) { name_ = name; }

		/// check vector
		bool is_feasible( const par_vec& vec ) const;
		void clamp( par_vec& vec ) const;

	private:
		std::vector< par_info > par_infos_;
		flat_map< string, par_value > locked_pars_;
		bool minimize_;
		fitness_t target_fitness_;
		string name_;

		std::vector< par_info >::const_iterator find( const string& name ) const;
		std::vector< par_info >::iterator find( const string& name );
		const par_info* try_find( const string& name ) const;
		par_info* try_find( const string& name );
		bool lock_parameter( const string& name, par_value value );
	};
}

#if defined(_MSC_VER)
#	pragma warning( pop )
#endif
