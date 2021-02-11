#pragma once

#include "objective.h"
#include <functional>
#include "xo/container/storage.h"
#include "par_info.h"

namespace spot
{
	template< int Dim, typename InputT = double, typename OutputT = double >
	struct data_model {
		static constexpr size_t size() { return Dim; }
		static par_vec mean() { return par_vec( size(), 0.0 ); }
		static par_vec stdev() { return par_vec( size(), 0.1 ); }
		static par_vec lower() { return par_vec( size(), default_lower_boundaray ); }
		static par_vec upper() { return par_vec( size(), default_upper_boundaray ); }
		using input_t = InputT;
		using output_t = OutputT;
		data_model( const spot::search_point& sp ) : p( sp ) {
			xo_error_if( size() != sp.size(), xo::stringf( "Model dimension mismatch: %d != %d", size(), sp.dim() ) );
		}
		const spot::search_point& p;
	};

	template< typename ModelT >
	class data_objective : public objective
	{
	public:
		using input_t = typename ModelT::input_t;
		using output_t = typename ModelT::output_t;
		using pair_t = typename std::pair<input_t, output_t>;
		using container_t = typename std::vector<pair_t>;

		data_objective( const container_t& data ) :
			objective( make_objective_info( ModelT::mean(), ModelT::stdev(), ModelT::lower(), ModelT::upper() ) ),
			data_( data )
		{}

		virtual fitness_t evaluate( const search_point& point ) const override {
			ModelT model( point );
			fitness_t result = 0;
			for ( const auto& [x, y] : data_ )
				result += xo::squared( model( x ) - y );
			return result / data_.size();
		}

	private:
		const container_t& data_;
	};
}
