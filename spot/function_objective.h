#pragma once

#include "objective.h"
#include <functional>
#include "xo/container/storage.h"
#include <string>

namespace spot
{
	using objective_function_t = std::function< fitness_t( const par_vec& ) >;

	class function_objective : public objective
	{
	public:
		function_objective( objective_function_t func, const par_vec& mean, const par_vec& stdev, const par_vec& lower, const par_vec& upper, const std::string& name = "" ) :
			objective( make_objective_info( mean, stdev, lower, upper ) ),
			func_( func )
		{
			info().set_name( name );
		}

		function_objective( objective_function_t func, size_t d, par_t mean, par_t stdev, par_t lower, par_t upper, const std::string& name = "" ) :
			objective( make_objective_info( d, mean, stdev, lower, upper ) ),
			func_( func )
		{
			info().set_name( name );
		}

		virtual fitness_t evaluate( const search_point& point ) const override { return func_( point.values() ); }

	protected:
		objective_function_t func_;
	};
}
