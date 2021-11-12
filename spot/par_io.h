#pragma once

#include "xo/container/prop_node.h"
#include "xo/utility/optional.h"
#include "xo/string/stack_string.h"
#include "spot_types.h"
#include "par_info.h"
#include "par_options.h"

namespace spot
{
	class SPOT_API par_io
	{
	public:
		par_io() {}
		virtual ~par_io() {}

		virtual size_t dim() const = 0;

		virtual xo::optional< par_t > try_get( const string& name ) const = 0;
		virtual par_t add( const par_info& pi ) = 0;

		par_t get( const string& name, par_t mean, par_t std, par_t min = -1e15, par_t max = 1e15 );
		par_t get( const string& name, const prop_node& pn );
		par_t try_get( const string& name, const prop_node& parent_pn, const string& key, const par_t& default_value );

		void set_prefix( const string& s ) { prefix_.set( s ); }
		void push_prefix( const string& s ) { prefix_.push_back( s ); }
		void pop_prefix() { prefix_.pop_back(); }
		const string& prefix() const { return prefix_.str(); }
		const par_options& options() const { return par_options_; }
		par_options& options() { return par_options_; }

	private:
		xo::stack_string prefix_;
		par_options par_options_;
	};

	struct scoped_prefix
	{
		scoped_prefix( par_io& ps, const string& prefix, bool add_dot = false ) : ps_( ps ) {
			if ( add_dot && !prefix.empty() && prefix.back() != '.' )
				ps_.push_prefix( prefix + '.' );
			else ps_.push_prefix( prefix );
		}
		operator par_io&( ) { return ps_; }
		~scoped_prefix() { ps_.pop_prefix(); }
	private:
		par_io& ps_;
	};

	struct scoped_prefix_setter
	{
		scoped_prefix_setter( par_io& ps, const string& prefix ) : ps_( ps ), previous_( ps_.prefix() ) { ps_.set_prefix( prefix ); }
		operator par_io&() { return ps_; }
		~scoped_prefix_setter() { ps_.set_prefix( previous_ ); }
	private:
		par_io& ps_;
		string previous_;
	};
}
