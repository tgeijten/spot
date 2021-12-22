#pragma once

#include "par_io.h"

#include <vector>

#include "xo/xo_types.h"
#include "xo/container/flat_map.h"
#include "xo/filesystem/path.h"
#include "xo/numerical/constants.h"
#include <utility>
#include "xo/string/pattern_matcher.h"

namespace spot
{
	class SPOT_API objective_info : public par_io
	{
	public:
		objective_info( bool min = true ) : minimize_( min ), target_fitness_( 0 ) {}
		objective_info( vector<par_info> par_infos, bool min = true ) : par_infos_( std::move( par_infos ) ), minimize_( min ), target_fitness_( 0 ) {}

		virtual size_t dim() const override { return par_infos_.size(); }
		virtual par_t add( const par_info& pi ) override;
		virtual xo::optional< par_t > try_get( const string& name ) const override;
		xo::optional< par_t > try_get_locked( const string& name ) const;
		const string& name() const { return name_; }

		fitness_t target_fitness() const { return target_fitness_; }
		void set_target_fitness( fitness_t f ) { target_fitness_ = f; }

		/// minimize / maximize
		bool minimize() const { return minimize_; }
		bool maximize() const { return !minimize_; }
		bool is_better( fitness_t a, fitness_t b ) const { return minimize() ? a < b : a > b; }
		index_t find_best_fitness( const fitness_vec& f ) const;
		template< typename T > T worst() const { return minimize() ? xo::constants<T>::max() : xo::constants<T>::lowest(); }
		template< typename T > T best() const { return minimize() ? xo::constants<T>::lowest() : xo::constants<T>::max(); }
		fitness_t worst_fitness() const { return worst< fitness_t >(); }
		fitness_t best_fitness() const { return best< fitness_t >(); }
		void set_minimize( bool m ) { minimize_ = m; }

		/// access by index
		const par_info& operator[]( index_t i ) const { return par_infos_[ i ]; }

		/// access by name
		index_t find_index( const string& name ) const;

		/// iterator access
		vector< par_info >::const_iterator begin() const { return par_infos_.begin(); }
		vector< par_info >::const_iterator end() const { return par_infos_.end(); }

		/// properties
		size_t size() const { return par_infos_.size(); }
		bool empty() const { return par_infos_.empty(); }

		/// import / export
		pair< size_t, size_t > import_mean_std( const path& filename, bool import_std, par_t std_factor = 1.0, par_t std_offset = 0.0,
			const xo::pattern_matcher& include = {}, const xo::pattern_matcher& exclude = {} );
		pair< size_t, size_t > import_locked( const path& filename );
		void set_std_minimum( par_t value, par_t factor );
		void set_mean_std( const par_vec& mean, const par_vec& std );
		void set_name( const string& name ) { name_ = name; }

		/// check vector
		bool is_feasible( const par_vec& vec ) const;
		void clamp( par_vec& vec ) const;

	private:
		vector< par_info > par_infos_;
		xo::flat_map< string, par_t > locked_pars_;
		bool minimize_;
		fitness_t target_fitness_;
		string name_;

		vector< par_info >::const_iterator find( const string& name ) const;
		vector< par_info >::iterator find( const string& name );
		const par_info* try_find( const string& name ) const;
		par_info* try_find( const string& name );
		bool lock_parameter( const string& name, par_t value );
	};

	SPOT_API objective_info make_objective_info( size_t d, par_t mean, par_t stdev, par_t lower, par_t upper );
	SPOT_API objective_info make_objective_info( const par_vec& mean, const par_vec& stdev, const par_vec& lower, const par_vec& upper );
}
