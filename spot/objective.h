#pragma once

#include "flut/system/types.hpp"
#include "flut/system/assert.hpp"
#include "search_point.h"
#include <functional>

#if defined(_MSC_VER)
#	pragma warning( push )
#	pragma warning( disable: 4251 )
#endif

namespace spot
{
	class SPOT_API objective
	{
	public:
		objective() {}
		virtual ~objective() {}

		const objective_info& info() const { return info_; }
		objective_info& info() { return info_; }
		size_t dim() const { return info_.dim(); }

		virtual string name() const { return string( "nameless" ); }
		virtual fitness_t evaluate( const search_point& point ) const = 0;
		virtual prop_node to_prop_node() const { return prop_node(); }

	protected:
		objective_info info_;
	};

	class SPOT_API function_objective : public objective
	{
	public:
		typedef std::function< fitness_t( const par_vec& ) > function_t;

		function_objective( size_t d, function_t func, bool minimize = true,
			const par_vec& start = par_vec(), const par_vec& start_std = par_vec(),
			const par_vec& upper = par_vec(), const par_vec& lower = par_vec() );

		virtual fitness_t evaluate( const search_point& point ) const override { return func_( point.values() ); }

		function_t func_;
	};
}

#if defined(_MSC_VER)
#	pragma warning( pop )
#endif
