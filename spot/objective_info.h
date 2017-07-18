#pragma once

#include "par_io.h"

#include "flut/system/platform.hpp"
#include "flut/system/types.hpp"
#include "flut/flat_map.hpp"
#include "flut/system/path.hpp"
#include "flut/math/math.hpp"

#if defined(_MSC_VER)
#	pragma warning( push )
#	pragma warning( disable: 4251 )
#endif

namespace spot
{
	class SPOT_API objective_info : public par_io
	{
	public:
		objective_info( bool min = true ) : minimize_( min ) {}

		struct par_info
		{
			string name;
			par_value mean;
			par_value std;
			par_value min;
			par_value max;
		};
		using par_info_vec = vector< par_info >;

		virtual size_t dim() const override { return par_infos_.size(); }
		virtual par_value add( const string& name, par_value mean, par_value std, par_value min = -1e15, par_value max = 1e15 ) override;
		virtual optional_par_value try_get( const string& name ) const override;
		optional_par_value try_get_fixed( const string& name ) const;
		const string& name() const { return name_; }

		/// minimize / maximize
		bool minimize() const { return minimize_; }
		bool maximize() const { return !minimize_; }
		bool is_better( fitness_t a, fitness_t b ) const { return minimize() ? a < b : a > b; }
		index_t find_best_fitness( const fitness_vec_t& f ) const;
		fitness_t worst_fitness() const { return minimize() ? num_const< fitness_t >::max() : num_const< fitness_t >::lowest(); }
		void set_minimize( bool m ) { minimize_ = m; }

		/// access by index
		const par_info& operator[]( index_t i ) const { return par_infos_[ i ]; }

		/// access by name
		index_t find_index( const string& name ) const;

		/// iterator access
		par_info_vec::const_iterator begin() const { return par_infos_.begin(); }
		par_info_vec::const_iterator end() const { return par_infos_.end(); }

		/// properties
		size_t size() const { return par_infos_.size(); }
		bool empty() const { return par_infos_.empty(); }

		/// import / export
		size_t import_mean_std( const path& filename, bool import_std );
		size_t import_fixed( const path& filename );
		void set_global_std( double factor, double offset );
		void set_mean_std( const vector< par_value >& mean, const vector< par_value >& std );
		void set_name( const string& name ) { name_ = name; }

	private:
		par_info_vec par_infos_;
		flat_map< string, par_value > fixed_pars_;
		bool minimize_;
		string name_;

		par_info_vec::const_iterator find( const string& name ) const;
		par_info_vec::iterator find( const string& name );
	};
}

#if defined(_MSC_VER)
#	pragma warning( pop )
#endif
