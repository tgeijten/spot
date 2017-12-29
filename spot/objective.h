#pragma once

#include "xo/utility/types.h"
#include "xo/system/assert.h"
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

		virtual string name() const { return string( "" ); }
		virtual fitness_t evaluate( const search_point& point ) const { xo_error( "Cannot evaluate undefined objective" ); }
		virtual prop_node to_prop_node() const { return prop_node(); }

	protected:
		objective_info info_;
	};

	using objective_function_t = std::function< fitness_t( const par_vec& ) >;

	class SPOT_API function_objective : public objective
	{
	public:
		function_objective( size_t d, objective_function_t func, const par_vec& start, const par_vec& start_std, const par_vec& lower, const par_vec& upper );
		function_objective( size_t d, objective_function_t func, par_value start, par_value std, par_value lower, par_value upper );

		virtual fitness_t evaluate( const search_point& point ) const override { return func_( point.values() ); }

	private:
		objective_function_t func_;
	};
}

#if defined(_MSC_VER)
#	pragma warning( pop )
#endif
