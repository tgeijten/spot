#include "optimizer_test.h"
#include <thread>
#include <chrono>

double fitfun_c( double const *x, int N )
{
	/* function "cigtab" */
	int i;
	double sum = 1e4*x[ 0 ] * x[ 0 ] + 1e-4*x[ 1 ] * x[ 1 ];
	for ( i = 2; i < N; ++i )
		sum += x[ i ] * x[ i ];
	return sum;
}

double fitfun( const std::vector< double >& x )
{
	/* function "cigtab" */
	double sum = 1e4*x[ 0 ] * x[ 0 ] + 1e-4*x[ 1 ] * x[ 1 ];
	for ( int i = 2; i < x.size(); ++i )
		sum += x[ i ] * x[ i ];
	return sum;
}

double slow_func( const std::vector< double >& x )
{
	/* function "cigtab" */
	double sum = 1e4*x[ 0 ] * x[ 0 ] + 1e-4*x[ 1 ] * x[ 1 ];
	for ( int i = 2; i < x.size(); ++i )
		sum += x[ i ] * x[ i ];

	std::this_thread::sleep_for( std::chrono::milliseconds( rand() % 2000 ) );

	return sum;
}

namespace flut
{
	void optimizer_test()
	{
		// setup mean / std / N
		int dim = 10;
		int seed = 123;
		int lambda = 0;
		std::vector< double > init_mean( dim, 0.0 );
		std::vector< double > init_std( dim, 0.3 );

		// init c-cmaes
		cmaes_t evo;
		double *arFunvals = cmaes_init( &evo, dim, &init_mean[ 0 ], &init_std[ 0 ], seed, lambda, "no" );

		// init cma_optimizer
		function_objective obj( dim, fitfun, true, init_mean, init_std );
		cma_optimizer cma( obj, lambda, seed );
		cma.set_max_threads( 10 );

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
					arFunvals[ i ] = fitfun_c( pop[ i ], (int)cmaes_Get( &evo, "dim" ) );
					//printf( "%.2f ", arFunvals[ i ] );
				}

				/* update the search distribution used for cmaes_SamplePopulation() */
				cmaes_UpdateDistribution( &evo, arFunvals );
				//printf( "\n" );
			}

			// update cma_optimizer
			{
				//printf( "D%03d: ", gen );
				auto& cma_pop = cma.sample_population();
				auto results = cma.evaluate( cma_pop );
				for ( int i = 0; i < cma.lambda(); ++i ) {
					//printf( "%.2f ", results[ i ] );
				}

				/* update the search distribution used for cmaes_SamplePopulation() */
				cma.update_distribution( results );
				//printf( "\n" );
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
		function_objective obj( dim, slow_func, true, par_vec( dim, 0.0 ), par_vec( dim, 0.3 ) );
		cma_optimizer cma( obj, lambda, seed );
		cma.set_max_threads( 3 );

		for ( int gen = 0; gen < 10; ++gen )
		{
			// update cma_optimizer
			{
				auto& cma_pop = cma.sample_population();
				auto results = cma.evaluate( cma_pop );

				printf( "D%03d: ", gen );
				for ( int i = 0; i < cma.lambda(); ++i ) {
					printf( "%.2f ", results[ i ] );
				}
				printf( "\n" );

				/* update the search distribution used for cmaes_SamplePopulation() */
				cma.update_distribution( results );
			}
		}

	}

}
