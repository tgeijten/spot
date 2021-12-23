#pragma once

#include "spot_types.h"
#include "search_point.h"
#include "xo/thread/stop_token.h"
#include <functional>

namespace spot
{
	class SPOT_API objective
	{
	public:
		objective() = default;
		objective( objective_info info ) : info_( std::move( info ) ) {}

		virtual ~objective() = default;

		const objective_info& info() const { return info_; }
		objective_info& info() { return info_; }
		size_t dim() const { return info_.dim(); }

		virtual string name() const { return info_.name(); }
		virtual prop_node to_prop_node() const { return prop_node(); }

		virtual result<fitness_t> evaluate( const search_point& point, const xo::stop_token& ) const { return evaluate( point ); }
		result<fitness_t> evaluate_noexcept( const search_point& point, const xo::stop_token& st ) const noexcept;

	protected:
		virtual fitness_t evaluate( const search_point& point ) const { xo_error( "Implement either objective::evaluate(search_point,stop_token) or objective::evaluate(search_point)" ); }
		objective_info info_;
	};
}
