#pragma once

#include "spot_types.h"
#include "search_point.h"
#include "xo/thread/stop_token.h"
#include <functional>

#if !SPOT_EVALUATOR_ENABLED
#include "xo/system/system_tools.h"
#include <future>
#endif // !SPOT_EVALUATOR_ENABLED

namespace spot
{
	class SPOT_API objective
	{
	public:
		objective() = default;
		virtual ~objective() = default;

		const objective_info& info() const { return info_; }
		objective_info& info() { return info_; }
		size_t dim() const { return info_.dim(); }

		virtual string name() const { return string( "" ); }
		virtual prop_node to_prop_node() const { return prop_node(); }

		virtual result<fitness_t> evaluate( const search_point& point, const xo::stop_token& ) const { return evaluate( point ); }
#if !SPOT_EVALUATOR_ENABLED
		fitness_t evaluate_noexcept( const search_point& point ) const noexcept;
		std::future< fitness_t > evaluate_async( const search_point& point, xo::thread_priority prio ) const;
		vector< result<fitness_t> > evaluate_async( const search_point_vec& pop, size_t max_threads, xo::thread_priority prio ) const;
#else
		result<fitness_t> evaluate_noexcept( const search_point& point, const xo::stop_token& st ) const noexcept;
#endif

	protected:
		virtual fitness_t evaluate( const search_point& point ) const { xo_error( "Implement either objective::evaluate(search_point,stop_token) or objective::evaluate(search_point)" ); }
		objective_info info_;
#if !SPOT_EVALUATOR_ENABLED
		fitness_t evaluate_in_thread_noexcept( const search_point& point, xo::thread_priority prio ) const;
#endif
	};
}
