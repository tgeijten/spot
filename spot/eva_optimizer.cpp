#include "eva_optimizer.h"
#include "math_tools.h"
#include "xo/numerical/math.h"
#include "xo/container/container_algorithms.h"

namespace spot
{
	eva_optimizer::eva_optimizer( const objective& o, evaluator& e, const eva_options& options ) :
		optimizer( o, e ),
		lambda_( options.lambda > 1 ? options.lambda : 4 + int( 3 * std::log( double( o.dim() ) ) ) ),
		mu_( options.mu ? options.mu : lambda_ / 2 ),
		max_resample_count( 100 ),
		options_( options ),
		random_engine_( options.random_seed ),
		population_( lambda_, search_point( o.info() ) )
	{
		mean_.reserve( o.dim() );
		var_.reserve( o.dim() );
		ev_.resize( o.dim(), 0 );
		for ( auto& pi : o.info() )
		{
			mean_.emplace_back( pi.mean );
			var_.emplace_back( xo::squared( pi.std ) );
		}

		// add flat fitness condition
		add_stop_condition( std::make_unique< flat_fitness_condition >( 1e-9 ) );
	}

	par_vec eva_optimizer::current_std() const
	{
		par_vec std_vec;
		std_vec.reserve( var_.size() );
		for ( auto& v : var_ )
			std_vec.emplace_back( std::sqrt( v ) );
		return std_vec;
	}

	objective_info eva_optimizer::make_updated_objective_info() const
	{
		objective_info inf( info() );
		inf.set_mean_std( current_mean(), current_std() );
		return inf;
	}

	vector< string > eva_optimizer::optimizer_state_labels() const
	{
		vector<string> labels;
		for ( auto& pi : info() ) {
			labels.emplace_back( pi.name + ".mean" );
			labels.emplace_back( pi.name + ".std" );
			labels.emplace_back( pi.name + ".ev" );
		}
		return labels;
	}

	vector< par_t > eva_optimizer::optimizer_state_values() const
	{
		vector<par_t> result;
		result.reserve( 3 * info().dim() );
		for ( index_t i = 0; i < info().dim(); ++i )
			result.insert( result.end(), { mean_[ i ], std::sqrt( var_[ i ] ), ev_[ i ] } );
		return result;
	}

	par_t eva_optimizer::sample_parameter( par_t mean, par_t stdev, const par_info& pi )
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

	void eva_optimizer::sample_population()
	{
		XO_PROFILE_FUNCTION( profiler_ );

		auto ev_distrib = std::normal_distribution( options_.ev_offset, options_.ev_stdev );
		const auto n = info().dim();
		par_vec x( n );
		for ( int ind_idx = 0; ind_idx < lambda_; ++ind_idx )
		{
			auto ev_ofs = ev_distrib( random_engine_ );
			for ( index_t i = 0; i < n; ++i ) {
				auto sample_mean = mean_[ i ] + ev_ofs * ev_[ i ];
				//auto sample_var = var_[ i ];
				auto sample_var = var_[ i ] + xo::squared( ev_[ i ] );
				x[ i ] = sample_parameter( sample_mean, std::sqrt( sample_var ), info()[ i ] );
			}

			population_[ ind_idx ].set_values( x );
		}
	}

	void eva_optimizer::update_distribution()
	{
		XO_PROFILE_FUNCTION( profiler_ );

		const auto n = info().dim();
		auto order = xo::sorted_indices( current_step_fitnesses_, [&]( auto a, auto b ) { return info().is_better( a, b ); } );

		std::vector<par_vec> projected_pop;
		for ( auto& individual : population_ ) {
			projected_pop.push_back( projected( mean_, ev_, individual.values() ) );
		}

		par_vec new_mean( n ), new_var( n );
		for ( index_t pi = 0; pi < n; ++pi ) {
			par_t mean = 0, var = 0;
			for ( index_t ui = 0; ui < mu_; ++ui ) {
				auto& individual = population_[ order[ ui ] ];
				auto& proj_ind = projected_pop[ order[ ui ] ];
				mean += individual[ pi ];
				var += xo::squared( individual[ pi ] - proj_ind[ pi ] ); // distance to projected point
				//var += xo::squared( individual[ pi ] - mean_[ pi ] ); // distance to previous mean
			}
			mean /= mu_;
			var /= mu_;
			//var /= ( mu_ - 1 );

			// update ev, mean and var
			auto delta_mean = mean - mean_[ pi ];
			update( ev_[ pi ], delta_mean, options_.ev_update );
			mean_[ pi ] += options_.step_size * delta_mean;
			update( var_[ pi ], var, options_.var_update );
		}
	}

	bool eva_optimizer::internal_step()
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
