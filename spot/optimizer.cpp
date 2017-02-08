#include "optimizer.h"

#include "ss/builder/model_builder.h"
#include "ss/core/arena.h"
#include "ss/core/simulator.h"

namespace ss
{
	optimizer::optimizer( const simulator_factory& f, const prop_node& pn ) : factory_( f )
	{
		INIT_PROP( pn, maximize, true );
	}

	std::vector< double > optimizer::evaluate( const prop_node& builder_pn, std::vector<par_set>& ps_vec )
	{
		std::vector< double > results;

		for ( auto& ps : ps_vec )
		{
			auto sim = factory_.create_simulator( prop_node() );
			prop_node ap;
			ap[ "settings" ].set( "simulation_frequency", 720 );

			arena a( *sim, nullptr, ap );
			model_builder mb( builder_pn, ps );
			a.register_model_info( mb.create_model_info() );
			auto m = a.create_model( mb.get_name(), transformf( vec3f( 0, 1, 0 ) ) );
			a.simulate_to( 10.0 );
			m->compute_com();
			results.push_back( m->com_pos.length() );
		}

		return results;
	}
}
