#include "mes_optimizer.h"

namespace spot
{
	mes_optimizer::mes_optimizer( const objective& o, evaluator& e, const mes_options& options ) :
		optimizer( o, e ),
		lambda_( options.lambda > 1 ? options.lambda : 4 + int( 3 * std::log( double( o.dim() ) ) ) ),
		mu_( options.mu ? options.mu : lambda_ / 2 ),
		max_resample_count( 100 ),
		mean_sigma( options.mean_sigma ),
		var_sigma( options.var_sigma ),
		mom_sigma( options.mom_sigma ),
		random_engine_( options.random_seed ),
		population_( std::make_unique<search_point_vec>( lambda_, search_point( o.info() ) ) )
	{
		mean_.reserve( o.dim() );
		var_.reserve( o.dim() );
		mom_.resize( o.dim(), 0 );
		for ( auto& pi : o.info() )
		{
			mean_.emplace_back( pi.mean );
			var_.emplace_back( xo::squared( pi.std ) );
		}
	}

	par_t mes_optimizer::sample_parameter( par_t mean, par_t stdev, const par_info& pi )
	{
		auto dist = std::normal_distribution( mean, stdev );
		for ( int i = 0; i < max_resample_count; ++i ) {
			auto v = dist( random_engine_ );
			if ( pi.is_within_range( v ) )
				return v;
		}
		xo::log::warning( "Could not sample parameter after ", max_resample_count, " attempts. Clamping value instead (this should not happen)." );
		return xo::clamped( dist( random_engine_ ), pi.min, pi.max );
	}

	void mes_optimizer::sample_population()
	{
		XO_PROFILE_FUNCTION( profiler_ );

		const auto n = info().dim();
		for ( int ind_idx = 0; ind_idx < lambda_; ++ind_idx )
		{
			auto& ind = population()[ ind_idx ].values();
			ind.resize( n );
			for ( index_t i = 0; i < n; ++i )
			{
				auto var = var_[ i ] + xo::squared( mom_[ i ] );
				ind[ i ] = sample_parameter( mean_[ i ], std::sqrt( var ), info()[ i ] );
			}
		}
	}

	inline void update( par_t& v, const par_t& nv, const par_t& rate ) { v = ( 1 - rate ) * v + rate * nv; }

	void mes_optimizer::update_distribution()
	{
		XO_PROFILE_FUNCTION( profiler_ );

		const auto n = info().dim();
		auto order = xo::sorted_indices( current_step_fitnesses_, [&]( auto a, auto b ) { return info().is_better( a, b ); } );
		par_vec new_mean( n ), new_var( n );
		for ( index_t pi = 0; pi < n; ++pi ) {
			par_t mean = 0, var = 0;
			for ( index_t ui = 0; ui < mu_; ++ui ) {
				auto oi = order[ ui ];
				mean += population()[ oi ][ pi ];
				var += xo::squared( population()[ oi ][ pi ] - mean_[ pi ] );
			}
			mean /= mu_;
			var /= mu_;

			if ( mom_sigma > 0 )
			{
				// update momentum, then mean
				auto delta_mean = mean - mean_[ pi ];
				update( mom_[ pi ], delta_mean, mom_sigma );
				mean_[ pi ] += mean_sigma * mom_[ pi ];
				update( var_[ pi ], var, var_sigma );
			}
			else
			{
				// update mean directly
				update( mean_[ pi ], mean, mean_sigma );
				update( var_[ pi ], var, var_sigma );
			}
		}
	}

	bool mes_optimizer::internal_step()
	{
		XO_PROFILE_FUNCTION( profiler_ );

		sample_population();

		if ( evaluate_step( population() ) )
		{
			update_distribution();
			return true;
		}
		else return false;
	}
}
