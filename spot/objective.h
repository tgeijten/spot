#pragma once

#include "xo/xo_types.h"
#include "search_point.h"
#include <functional>
#include <future>
#include "xo/system/system_tools.h"

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
		virtual prop_node to_prop_node() const { return prop_node(); }

		virtual fitness_t evaluate( const search_point& point ) const = 0;
		fitness_t evaluate_noexcept( const search_point& point ) const noexcept;
		std::future< fitness_t > evaluate_async( const search_point& point, xo::thread_priority prio ) const;
		fitness_vec evaluate_async( const search_point_vec& pop, size_t max_threads, xo::thread_priority prio ) const;

	protected:
		objective_info info_;
		fitness_t evaluate_in_thread_noexcept( const search_point& point, xo::thread_priority prio ) const;
	};

	using objective_function_t = std::function< fitness_t( const par_vec& ) >;

	class SPOT_API function_objective : public objective
	{
	public:
		function_objective( size_t d, objective_function_t func, const par_vec& start, const par_vec& start_std, const par_vec& lower, const par_vec& upper );
		function_objective( size_t d, objective_function_t func, par_t start, par_t std, par_t lower, par_t upper );

		virtual fitness_t evaluate( const search_point& point ) const override { return func_( point.values() ); }

	private:
		objective_function_t func_;
	};
}
