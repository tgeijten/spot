#pragma once

#include "objective.h"
#include <functional>
#include "xo/container/storage.h"

namespace spot
{
	using objective_function_t = std::function< fitness_t( const par_vec& ) >;

	class SPOT_API function_objective : public objective
	{
	public:
		function_objective( size_t d, objective_function_t func, const par_vec& mean, const par_vec& stdev, const par_vec& lower, const par_vec& upper );
		function_objective( size_t d, objective_function_t func, par_t mean, par_t stdev, par_t lower, par_t upper );

		virtual fitness_t evaluate( const search_point& point ) const override { return func_( point.values() ); }

	protected:
		objective_function_t func_;
	};
}
