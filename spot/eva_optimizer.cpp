#include "eva_optimizer.h"
#include "xo/utility/irange.h"

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

	inline par_t length( const par_vec& vec ) {
		par_t sum = 0;
		for ( const auto& v : vec )
			sum += v * v;
		return std::sqrt( sum );
	}

	inline par_t dot_product( const par_vec& v1, const par_vec& v2 ) {
		xo_assert( v1.size() == v2.size() );
		par_t sum = 0;
		for ( auto i : xo::size_range( v1 ) )
			sum += v1[ i ] * v2[ i ];
		return sum;
	}

	inline par_vec vec_sub( par_vec a, const par_vec& b ) {
		xo_assert( a.size() == b.size() );
		for ( index_t i = 0; i < a.size(); ++i )
			a[ i ] -= b[ i ];
		return a;
	}

	inline par_vec vec_add( par_vec a, const par_vec& b ) {
		xo_assert( a.size() == b.size() );
		for ( index_t i = 0; i < a.size(); ++i )
			a[ i ] += b[ i ];
		return a;
	}

	inline par_vec vec_mul( par_t s, par_vec a ) {
		for ( index_t i = 0; i < a.size(); ++i )
			a[ i ] *= s;
		return a;
	}

	inline void update( par_t& v, const par_t& nv, const par_t& rate ) {
		v = ( 1 - rate ) * v + rate * nv;
	}

	inline par_vec projected( const par_vec& a, const par_vec& ab, const par_vec& p ) {
		auto ap = vec_sub( p, a );
		auto dot_ab = dot_product( ab, ab );
		if ( dot_ab > xo::num<par_t>::ample_epsilon ) {
			auto t = dot_product( ap, ab ) / dot_ab;
			return vec_add( a, vec_mul( t, ab ) );
		}
		else return a;
	}

	void eva_optimizer::sample_population()
	{
		XO_PROFILE_FUNCTION( profiler_ );

		auto mom_dist = std::normal_distribution( options_.ev_offset, options_.ev_stdev );
		const auto n = info().dim();
		par_vec vec( n );
		for ( int ind_idx = 0; ind_idx < lambda_; ++ind_idx )
		{
			auto mom_ofs = mom_dist( random_engine_ );
			for ( index_t i = 0; i < n; ++i )
			{
				auto var = var_[ i ] + xo::squared( ev_[ i ] );
				vec[ i ] = sample_parameter( mean_[ i ] + mom_ofs * ev_[ i ], std::sqrt( var ), info()[ i ] );
			}
			population_[ ind_idx ].set_values( vec );
		}
	}

	void eva_optimizer::update_distribution()
	{
		XO_PROFILE_FUNCTION( profiler_ );

		const auto n = info().dim();
		auto order = xo::sorted_indices( current_step_fitnesses_, [&]( auto a, auto b ) { return info().is_better( a, b ); } );

		std::vector<par_vec> projected_pop;
		for ( auto& sp : population_ ) {
			projected_pop.push_back( projected( mean_, ev_, sp.values() ) );
		}

		par_vec new_mean( n ), new_var( n );
		for ( index_t pi = 0; pi < n; ++pi ) {
			par_t mean = 0, var = 0;
			for ( index_t ui = 0; ui < mu_; ++ui ) {
				auto& individual = population_[ order[ ui ] ];
				auto& proj_ind = projected_pop[ order[ ui ] ];
				mean += individual[ pi ];
				var += xo::squared( individual[ pi ] - proj_ind[ pi ] ); // distance to projected point
			}
			mean /= mu_;
			var /= mu_;

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
