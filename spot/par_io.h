#pragma once

#include "xo/string/string_tools.h"
#include "xo/container/prop_node.h"
#include "xo/utility/optional.h"
#include "types.h"

#if defined(_MSC_VER)
#	pragma warning( push )
#	pragma warning( disable: 4251 )
#endif

namespace spot
{
	class SPOT_API par_io
	{
	public:
		par_io() {}
		virtual ~par_io() {}

		virtual size_t dim() const = 0;

		virtual optional_par_value try_get( const string& name ) const = 0;
		virtual par_value add( const string& full_name, par_value mean, par_value std, par_value min = -1e15, par_value max = 1e15 ) = 0;

		par_value get( const string& name, par_value mean, par_value std, par_value min = -1e15, par_value max = 1e15 );
		par_value get( const string& name, const prop_node& pn );
		par_value add( const string& full_name, const prop_node& pn );
		par_value try_get( const string& name, const prop_node& parent_pn, const string& key, const par_value& default_value );

		void set_prefix( const string& s );
		void push_prefix( const string& s );
		void pop_prefix();
		const string& prefix() const { return prefix_; }

	private:
		string prefix_;
		std::vector< size_t > prefixes_sizes;
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

#if defined(_MSC_VER)
#	pragma warning( pop )
#endif
