#include "mes_optimizer.h"
#include "math_tools.h"
#include "xo/numerical/math.h"
#include "xo/container/container_algorithms.h"

namespace spot
{
	mes_optimizer::mes_optimizer( const objective& o, evaluator& e, const mes_options& options ) :
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
		mom_.resize( o.dim(), 0 );
		for ( auto& pi : o.info() )
		{
			mean_.emplace_back( pi.mean );
			var_.emplace_back( xo::squared( pi.std ) );
		}

		// add flat fitness condition
		add_stop_condition( std::make_unique< flat_fitness_condition >( 1e-9 ) );
	}

	par_vec mes_optimizer::current_std() const
	{
		par_vec std_vec;
		std_vec.reserve( var_.size() );
		for ( auto& v : var_ )
			std_vec.emplace_back( std::sqrt( v ) );
		return std_vec;
	}

	objective_info mes_optimizer::make_updated_objective_info() const
	{
		objective_info inf( info() );
		inf.set_mean_std( current_mean(), current_std() );
		return inf;
	}

	vector< string > mes_optimizer::optimizer_state_labels() const
	{
		vector<string> labels;
		for ( auto& pi : info() ) {
			labels.emplace_back( pi.name + ".mean" );
			labels.emplace_back( pi.name + ".std" );
			labels.emplace_back( pi.name + ".mom" );
		}
		return labels;
	}

	vector< par_t > mes_optimizer::optimizer_state_values() const
	{
		vector<par_t> result;
		result.reserve( 3 * info().dim() );
		for ( index_t i = 0; i < info().dim(); ++i )
			result.insert( result.end(), { mean_[i], std::sqrt( var_[i] ), mom_[i] } );
		return result;
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

		auto mom_dist = std::normal_distribution( options_.mom_offset, options_.mom_offset_stdev );
		const auto n = info().dim();
		par_vec vec( n );
		for ( int ind_idx = 0; ind_idx < lambda_; ++ind_idx )
		{
			auto mom_ofs = mom_dist( random_engine_ );
			for ( index_t i = 0; i < n; ++i )
			{
				auto var = var_[i] + xo::squared( mom_[i] );
				vec[i] = sample_parameter( mean_[i] + mom_ofs * mom_[i], std::sqrt( var ), info()[i] );
			}
			population_[ind_idx].set_values( vec );
		}
	}

	void mes_optimizer::update_distribution()
	{
		XO_PROFILE_FUNCTION( profiler_ );

		const auto n = info().dim();
		auto order = xo::sorted_indices( current_step_fitnesses_, [&]( auto a, auto b ) { return info().is_better( a, b ); } );

		std::vector<par_vec> projected_pop;
		for ( auto& sp : population_ ) {
			projected_pop.push_back( projected( mean_, mom_, sp.values() ) );
		}

		par_vec new_mean( n ), new_var( n );
		for ( index_t pi = 0; pi < n; ++pi ) {
			par_t mean = 0, var = 0;
			for ( index_t ui = 0; ui < mu_; ++ui ) {
				auto& individual = population_[order[ui]];
				auto& proj_ind = projected_pop[order[ui]];
				mean += individual[pi];
				//var += xo::squared( individual[ pi ] - ( mean_[ pi ] + options_.mom_offset * mom_[ pi ] ) );
				var += xo::squared( individual[pi] - proj_ind[pi] ); // distance to projected point
			}
			mean /= mu_;
			var /= mu_;

			if ( options_.mom_sigma > 0 )
			{
				// update momentum, then mean
				auto delta_mean = mean - mean_[pi];
				update( mom_[pi], delta_mean, options_.mom_sigma );
				mean_[pi] += options_.mean_sigma * mom_[pi];
				update( var_[pi], var, options_.var_sigma );
			}
			else
			{
				// update mean directly
				update( mean_[pi], mean, options_.mean_sigma );
				update( var_[pi], var, options_.var_sigma );
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
