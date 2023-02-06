#pragma once

#include "xo/container/prop_node.h"
#include "xo/utility/optional.h"
#include "xo/string/stack_string.h"
#include "spot_types.h"
#include "par_info.h"
#include "par_options.h"
#include <stack>
#include <vector>
#include "xo/system/log.h"

namespace spot
{
	class SPOT_API par_io
	{
	public:
		par_io() : prefix_(), par_options_( { par_options{} } ) {}
		virtual ~par_io() {}

		virtual size_t dim() const = 0;

		virtual xo::optional< par_t > try_get( const string& name ) const = 0;
		virtual par_t add( const par_info& pi ) = 0;

		par_t get( const string& name, par_t mean, par_t std, par_t min = -1e15, par_t max = 1e15 );
		par_t get( const string& name, const prop_node& pn );
		par_t try_get( const string& name, const prop_node& parent_pn, const string& key, const par_t& default_value );

		const string& prefix() const { return prefix_.str(); }
		void set_prefix( const string& s ) { prefix_.set( s ); }
		void push_prefix( const string& s ) { prefix_.push_back( s ); }
		void pop_prefix() { prefix_.pop_back(); }

		const par_options& options() const { xo_assert( !par_options_.empty() );  return par_options_.top(); }
		par_options& options() { xo_assert( !par_options_.empty() ); return par_options_.top(); }
		void push_options() { par_options_.emplace( options() ); }
		void pop_options() { par_options_.pop(); }

	private:
		xo::stack_string prefix_;
		std::stack<par_options, std::vector<par_options>> par_options_;
	};

	struct scoped_prefix
	{
		scoped_prefix( par_io& ps, const string& prefix, bool add_dot = false ) : ps_( ps ) {
			if ( add_dot && !prefix.empty() && prefix.back() != '.' )
				ps_.push_prefix( prefix + '.' );
			else ps_.push_prefix( prefix );
		}
		operator par_io& ( ) { return ps_; }
		~scoped_prefix() { ps_.pop_prefix(); }
	private:
		par_io& ps_;
	};

	class SPOT_API null_objective_info : public par_io
	{
	public:
		size_t dim() const override { return 0; }
		xo::optional<par_t> try_get( const string& name ) const override { return {}; }
		par_t add( const par_info& pi ) override { return pi.mean; }
	};

	struct scoped_prefix_setter
	{
		scoped_prefix_setter( par_io& ps, const string& prefix ) : ps_( ps ), previous_( ps_.prefix() ) { ps_.set_prefix( prefix ); }
		operator par_io& ( ) { return ps_; }
		~scoped_prefix_setter() { ps_.set_prefix( previous_ ); }
	private:
		par_io& ps_;
		string previous_;
	};

	struct scoped_par_options
	{
		scoped_par_options( const prop_node& pn, par_io& par ) : ps_( par ) {
			ps_.push_options();
			pn.try_get( ps_.options().auto_std_factor, "auto_std_factor" );
			pn.try_get( ps_.options().auto_std_offset, "auto_std_offset" );
			pn.try_get( ps_.options().auto_std_minimum, "auto_std_minimum" );
			pn.try_get( ps_.options().aliases_, "parameter_aliases" );
		}
		~scoped_par_options() {
			ps_.pop_options();
		}
	private:
		par_io& ps_;
	};
}
