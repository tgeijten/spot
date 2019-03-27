#pragma once

#include "xo/container/prop_node.h"
#include "xo/utility/optional.h"
#include "xo/string/stack_string.h"
#include "types.h"
#include "par_info.h"

namespace spot
{
	class SPOT_API par_io
	{
	public:
		par_io() {}
		virtual ~par_io() {}

		virtual size_t dim() const = 0;

		virtual xo::optional< par_value > try_get( const string& name ) const = 0;
		virtual par_value add( const par_info& pi ) = 0;

		par_value get( const string& name, par_value mean, par_value std, par_value min = -1e15, par_value max = 1e15 );
		par_value get( const string& name, const prop_node& pn );
		par_value try_get( const string& name, const prop_node& parent_pn, const string& key, const par_value& default_value );

		void set_prefix( const string& s ) { prefix_.set( s ); }
		void push_prefix( const string& s ) { prefix_.push_back( s ); }
		void pop_prefix() { prefix_.pop_back(); }
		const string& prefix() const { return prefix_.str(); }

	private:
		xo::stack_string prefix_;
	};

	struct scoped_prefix
	{
		scoped_prefix( par_io& ps, const string& prefix ) : ps_( ps ) { ps_.push_prefix( prefix ); }
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
