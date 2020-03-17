#pragma once

#include "objective.h"
#include <functional>

namespace spot
{
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
