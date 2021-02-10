#pragma once

#include "objective.h"
#include <functional>
#include "xo/container/storage.h"
#include "par_info.h"

namespace spot
{
	template< typename InputT = double, typename OutputT = double>
	class data_objective : public objective
	{
	public:
		using function_t = typename std::function< OutputT( const par_vec&, InputT ) >;

		using pair_t = typename std::pair<InputT, OutputT>;
		using container_t = typename std::vector<pair_t>;

		data_objective( const container_t& data, function_t func, const par_vec& mean, const par_vec& stdev, const par_vec& lower, const par_vec& upper ) :
			objective( make_objective_info( mean, stdev, lower, upper ) ),
			data_( data ),
			func_( func )
		{}

		data_objective( const container_t& data, function_t func, size_t d, par_t mean, par_t stdev, par_t lower = default_lower_boundaray, par_t upper = default_upper_boundaray ) :
			objective( make_objective_info( d, mean, stdev, lower, upper ) ),
			data_( data ),
			func_( func )
		{}

		virtual fitness_t evaluate( const search_point& point ) const override {
			fitness_t result = 0;
			for ( const auto& [x, y] : data_ )
				result += xo::squared( func_( point.values(), x ) - y );
			return result;
		}

	private:
		const container_t& data_;
		function_t func_;
	};
}
