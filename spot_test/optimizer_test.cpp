#include "optimizer_test.h"

#include <thread>
#include <chrono>
#include "test_functions.h"
#include "spot/async_evaluator.h"

namespace spot
{
	void optimizer_test()
	{
		// setup mean / std / N
		int dim = 10;
		int seed = 123;
		int lambda = 0;
		std::vector< double > init_mean( dim, 0.0 );
		std::vector< double > init_std( dim, 0.3 );
		std::vector< double > lower( dim, -1e12 );
		std::vector< double > upper( dim, -1e12 );

		// init c-cmaes
		cmaes_t evo;
		double *arFunvals = cmaes_init( &evo, dim, &init_mean[ 0 ], &init_std[ 0 ], seed, lambda, "no" );

		// init cma_optimizer
		function_objective obj( dim, cigtab, init_mean, init_std, lower, upper );
		async_evaluator eval;
		cma_optimizer cma( obj, eval, cma_options{ lambda, seed } );
#if !SPOT_EVALUATOR_ENABLED
		cma.set_max_threads( 10 );
#endif // !SPOT_EVALUATOR_ENABLED

		/* Iterate until stop criterion holds */
		for ( int gen = 0; gen < 100; ++gen )
		{
			// update C-CMAES
			{
				/* generate lambda new search points, sample population */
				double* const* pop;
				//printf( "C%03d: ", gen );
				pop = cmaes_SamplePopulation( &evo ); /* do not change content of pop */

				/* evaluate the new search points using fitfun */
				for ( int i = 0; i < cmaes_Get( &evo, "lambda" ); ++i ) {
					arFunvals[ i ] = cigtab_c( pop[ i ], (int)cmaes_Get( &evo, "dim" ) );
					//printf( "%.2f ", arFunvals[ i ] );
				}

				/* update the search distribution used for cmaes_SamplePopulation() */
				cmaes_UpdateDistribution( &evo, arFunvals );
				//printf( "\n" );
			}

			// update cma_optimizer
			{
				//printf( "D%03d: ", gen );
				cma.step();
			}
		}

		/* get best estimator for the optimum, xmean */
		double* xfinal = cmaes_GetNew( &evo, "xmean" ); /* "xbestever" might be used as well */
		cmaes_exit( &evo ); /* release memory */

		/* do something with final solution and finally release memory */
		free( xfinal );
	}

	void optimizer_thread_test()
	{
		// setup mean / std / N
		size_t dim = 10;
		int seed = 123;
		int lambda = 0;

		// init cma_optimizer
		function_objective obj( dim, cigtab, 0.0, 0.3, -1e12, 1e12 );
		async_evaluator eval;
		cma_optimizer cma( obj, eval, cma_options{ lambda, seed } );
#if !SPOT_EVALUATOR_ENABLED
		cma.set_max_threads( 3 );
#endif // !SPOT_EVALUATOR_ENABLED

		for ( int gen = 0; gen < 10; ++gen )
		{
			// update cma_optimizer
			{
				cma.step();
				
				auto results = cma.current_step_fitnesses();
				printf( "D%03d: ", gen );
				for ( int i = 0; i < cma.lambda(); ++i ) {
					printf( "%.2f ", results[ i ] );
				}
				printf( "\n" );
			}
		}

	}

}
