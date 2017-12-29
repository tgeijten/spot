#define _CRT_SECURE_NO_WARNINGS

#include "cma_optimizer.h"

#include "xo/system/assert.h"
#include <algorithm>
#include "xo/container/container_tools.h"
#include "xo/numerical/math.h"
#include "xo/system/log.h"

#include <cmath>
#include <cstring>
#include <numeric>

namespace spot
{
	typedef std::vector< double > dbl_vec;

	struct cmaes_random_t
	{
		/* Variables for Uniform() */
		long int startseed;
		long int aktseed;
		long int aktrand;
		std::vector< long int > rgrand;

		/* Variables for Gauss() */
		short flgstored;
		double hold;
	};

	struct cmaes_readpara_t
	{
		/* input parameters */
		int N; /* problem dimension, must stay constant, should be unsigned or long? */
		unsigned int seed;
		dbl_vec xstart;
		dbl_vec rgInitialStds;
		double * rgDiffMinChange; // Minimal coordinate wise standard deviation. Not used.

		/* termination parameters */
		double stopMaxFunEvals;
		double facmaxeval;
		double stopMaxIter;
		struct { int flg; double val; } stStopFitness;

		/* internal evolution strategy parameters */
		int lambda;          /* -> mu, <- N */
		int mu;              /* -> weights, (lambda) */
		double mucov, mueff; /* <- weights */
		dbl_vec weights;     /* <- mu, -> mueff, mucov, ccov */
		double damps;        /* <- cs, maxeval, lambda */
		double cs;           /* -> damps, <- N */
		double ccumcov;      /* <- N */
		double ccov;         /* <- mucov, <- N */
		double diagonalCov;  /* number of initial iterations */
		struct { int flgalways; double modulo; double maxtime; } updateCmode;
		double facupdateCmode;

		/* supplementary variables */
		cma_weights weigh_mode;
	};

	struct cmaes_t
	{
		const char *version;
		/* char *signalsFilename; */
		cmaes_readpara_t sp;
		cmaes_random_t rand; /* random number generator */

		double sigma;  /* step size */

		dbl_vec current_mean;  /* mean x vector, "parent" */
		dbl_vec current_best;
		std::vector< dbl_vec > current_pop;   /* range of x-vectors, lambda offspring */
		std::vector< int > index;       /* sorting index of sample pop. */
		dbl_vec arFuncValueHist;

		short flgIniphase; /* not really in use anymore */
		short flgStop;

		double chiN;
		std::vector< dbl_vec > C;  /* lower triangular matrix: i>=j for C[i][j] */
		std::vector< dbl_vec > B;  /* matrix with normalize eigenvectors in columns */
		dbl_vec rgD; /* axis lengths */

		dbl_vec rgpc;
		dbl_vec rgps;
		dbl_vec rgxold;
		dbl_vec rgout;
		dbl_vec rgBDz;   /* for B*D*z */
		dbl_vec rgdTmp;  /* temporary (random) vector used in different places */
		dbl_vec rgFuncValue;
		dbl_vec publicFitness; /* returned by cmaes_init() */

		double gen; /* Generation number */
		double countevals;
		double state; /* 1 == sampled, 2 == not in use anymore, 3 == updated */

		double maxdiagC; /* repeatedly used for output */
		double mindiagC;
		double maxEW;
		double minEW;

		short flgEigensysIsUptodate;
		short flgCheckEigen; /* control via cmaes_signals.par */
		double genOfEigensysUpdate;

		double dMaxSignifKond;
		double dLastMinEWgroesserNull;
	};



	long cmaes_random_init( cmaes_random_t *t, long unsigned inseed )
	{
		t->flgstored = 0;
		t->rgrand.resize( 32 );

		t->flgstored = 0;
		t->startseed = inseed; /* purely for bookkeeping */

		long epoch = clock();
		if ( inseed < 1 ) {
			while ( epoch == (long)clock() );
			inseed = (long unsigned)labs( (long)( 100 * time( NULL ) + clock() ) );
		}

		while ( inseed > 2e9 )
			inseed /= 2; /* prevent infinite loop on 32 bit system */
		t->aktseed = inseed;
		for ( long i = 39; i >= 0; --i )
		{
			long tmp = t->aktseed / 127773;
			t->aktseed = 16807 * ( t->aktseed - tmp * 127773 )
				- 2836 * tmp;
			if ( t->aktseed < 0 ) t->aktseed += 2147483647;
			if ( i < 32 )
				t->rgrand[ i ] = t->aktseed;
		}
		t->aktrand = t->rgrand[ 0 ];
		return inseed;
	}



	void cmaes_readpara_SetWeights( cmaes_readpara_t *t )
	{
		double s1, s2;
		int i;

		t->weights.resize( t->mu );
		switch ( t->weigh_mode )
		{
		case cma_weights::linear:
			for ( i = 0; i < t->mu; ++i )
				t->weights[ i ] = t->mu - i;
			break;
		case cma_weights::equal:
			for ( i = 0; i < t->mu; ++i )
				t->weights[ i ] = 1;
			break;
		case cma_weights::log:
			for ( i = 0; i < t->mu; ++i )
				t->weights[ i ] = std::log( t->mu + 1. ) - std::log( i + 1. );
			break;
		default:
			xo_error( "Invalid weighting mode " + to_str( t->weigh_mode ) );
			break;
		}

		/* normalize weights vector and set mueff */
		for ( i = 0, s1 = 0, s2 = 0; i < t->mu; ++i ) {
			s1 += t->weights[ i ];
			s2 += t->weights[ i ] * t->weights[ i ];
		}
		t->mueff = s1*s1 / s2;
		for ( i = 0; i < t->mu; ++i )
			t->weights[ i ] /= s1;

		if ( t->mu < 1 || t->mu > t->lambda ||
			( t->mu == t->lambda && t->weights[ 0 ] == t->weights[ t->mu - 1 ] ) )
			xo_error( "cmaes_readpara_SetWeights(): invalid setting of mu or lambda" );
	} /* cmaes_readpara_SetWeights() */



	double cmaes_random_Uniform( cmaes_random_t *t )
	{
		long tmp;

		tmp = t->aktseed / 127773;
		t->aktseed = 16807 * ( t->aktseed - tmp * 127773 ) - 2836 * tmp;
		if ( t->aktseed < 0 )
			t->aktseed += 2147483647;
		tmp = t->aktrand / 67108865;
		t->aktrand = t->rgrand[ tmp ];
		t->rgrand[ tmp ] = t->aktseed;
		return (double)( t->aktrand ) / ( 2.147483647e9 );
	}

	double cmaes_random_Gauss( cmaes_random_t *t )
	{
		double x1, x2, rquad, fac;

		if ( t->flgstored )
		{
			t->flgstored = 0;
			return t->hold;
		}
		do
		{
			x1 = 2.0 * cmaes_random_Uniform( t ) - 1.0;
			x2 = 2.0 * cmaes_random_Uniform( t ) - 1.0;
			rquad = x1*x1 + x2*x2;
		}
		while ( rquad >= 1 || rquad <= 0 );
		fac = sqrt( -2.0*std::log( rquad ) / rquad );
		t->flgstored = 1;
		t->hold = fac * x1;
		return fac * x2;
	}



	void cmaes_init( cmaes_t *t,
		int dim,
		const dbl_vec& inxstart,
		const dbl_vec& inrgsigma,
		int inseed,
		int lambda )
	{
		//
		// void cmaes_readpara_init
		//

		t->sp.N = dim;
		t->sp.seed = (unsigned)inseed;
		t->sp.rgDiffMinChange = NULL;
		t->sp.stopMaxFunEvals = -1;
		t->sp.stopMaxIter = -1;
		t->sp.facmaxeval = 1;
		t->sp.stStopFitness.flg = -1;

		t->sp.lambda = lambda;
		t->sp.mu = -1;
		t->sp.mucov = -1;
		t->sp.weigh_mode = cma_weights::log;

		t->sp.cs = -1;
		t->sp.ccumcov = -1;
		t->sp.damps = -1;
		t->sp.ccov = -1;

		t->sp.diagonalCov = 0; /* default is 0, but this might change in future, see below */

		t->sp.updateCmode.modulo = -1;
		t->sp.updateCmode.maxtime = -1;
		t->sp.updateCmode.flgalways = 0;
		t->sp.facupdateCmode = 1;

		int N = t->sp.N;

		if ( t->sp.xstart.empty() ) {
			t->sp.xstart.resize( N );

			/* put inxstart into xstart */
			for ( int i = 0; i < N; ++i )
				t->sp.xstart[ i ] = inxstart[ i ];
		} /* xstart == NULL */

		if ( t->sp.rgInitialStds.empty() ) {
			t->sp.rgInitialStds.resize( N );
			for ( int i = 0; i < N; ++i )
				t->sp.rgInitialStds[ i ] = inrgsigma[ i ];
		}
	}

	void cmaes_readpara_SupplementDefaults( cmaes_t *t )
	{
		int N = t->sp.N;
		double t1, t2;
		if ( t->sp.stStopFitness.flg == -1 )
			t->sp.stStopFitness.flg = 0;

		if ( t->sp.lambda < 2 )
			t->sp.lambda = 4 + (int)( 3 * std::log( (double)N ) );

		if ( t->sp.mu == -1 )
			t->sp.mu = t->sp.lambda / 2;

		if ( t->sp.weights.empty() )
			cmaes_readpara_SetWeights( &t->sp );

		if ( t->sp.cs > 0 ) /* factor was read */
			t->sp.cs *= ( t->sp.mueff + 2. ) / ( N + t->sp.mueff + 3. );
		if ( t->sp.cs <= 0 || t->sp.cs >= 1 )
			t->sp.cs = ( t->sp.mueff + 2. ) / ( N + t->sp.mueff + 3. );

		if ( t->sp.ccumcov <= 0 || t->sp.ccumcov > 1 )
			t->sp.ccumcov = 4. / ( N + 4 );

		if ( t->sp.mucov < 1 ) {
			t->sp.mucov = t->sp.mueff;
		}
		t1 = 2. / ( ( N + 1.4142 )*( N + 1.4142 ) );
		t2 = ( 2.*t->sp.mueff - 1. ) / ( ( N + 2. )*( N + 2. ) + t->sp.mueff );
		t2 = ( t2 > 1 ) ? 1 : t2;
		t2 = ( 1. / t->sp.mucov ) * t1 + ( 1. - 1. / t->sp.mucov ) * t2;
		if ( t->sp.ccov >= 0 ) /* ccov holds the read factor */
			t->sp.ccov *= t2;
		if ( t->sp.ccov < 0 || t->sp.ccov > 1 ) /* set default in case */
			t->sp.ccov = t2;

		if ( t->sp.diagonalCov == -1 )
			t->sp.diagonalCov = 2 + 100. * N / sqrt( (double)t->sp.lambda );

		if ( t->sp.stopMaxFunEvals == -1 )  /* may depend on ccov in near future */
			t->sp.stopMaxFunEvals = t->sp.facmaxeval * 900 * ( N + 3 )*( N + 3 );
		else
			t->sp.stopMaxFunEvals *= t->sp.facmaxeval;

		if ( t->sp.stopMaxIter == -1 )
			t->sp.stopMaxIter = ceil( (double)( t->sp.stopMaxFunEvals / t->sp.lambda ) );

		if ( t->sp.damps < 0 )
			t->sp.damps = 1; /* otherwise a factor was read */
		t->sp.damps = t->sp.damps
			* ( 1 + 2 * std::max( 0., sqrt( ( t->sp.mueff - 1. ) / ( N + 1. ) ) - 1 ) )     /* basic factor */
			* std::max( 0.3, 1. -                                       /* modify for short runs */
			(double)N / ( 1e-6 + std::min( t->sp.stopMaxIter, t->sp.stopMaxFunEvals / t->sp.lambda ) ) )
			+ t->sp.cs;                                                 /* minor increment */

		if ( t->sp.updateCmode.modulo < 0 )
			t->sp.updateCmode.modulo = 1. / t->sp.ccov / (double)( N ) / 10.;
		t->sp.updateCmode.modulo *= t->sp.facupdateCmode;
		if ( t->sp.updateCmode.maxtime < 0 )
			t->sp.updateCmode.maxtime = 0.20; /* maximal 20% of CPU-time */
	}

	void cmaes_init_final( cmaes_t *t )
	{
		int N = t->sp.N;
		int i, j;
		double dtest, trace;

		t->sp.seed = cmaes_random_init( &t->rand, ( long unsigned int ) t->sp.seed );

		N = t->sp.N; /* for convenience */

		/* initialization  */
		for ( i = 0, trace = 0.; i < N; ++i )
			trace += t->sp.rgInitialStds[ i ] * t->sp.rgInitialStds[ i ];
		t->sigma = sqrt( trace / N ); /* t->sp.mueff/(0.2*t->sp.mueff+sqrt(N)) * sqrt(trace/N); */

		t->chiN = sqrt( (double)N ) * ( 1. - 1. / ( 4.*N ) + 1. / ( 21.*N*N ) );
		t->flgEigensysIsUptodate = 1;
		t->flgCheckEigen = 0;
		t->genOfEigensysUpdate = 0;
		t->flgIniphase = 0; /* do not use iniphase, hsig does the job now */
		t->flgStop = 0;

		for ( dtest = 1.; dtest && dtest < 1.1 * dtest; dtest *= 2. )
			if ( dtest == dtest + 1. )
				break;
		t->dMaxSignifKond = dtest / 1000.; /* not sure whether this is really save, 100 does not work well enough */

		t->gen = 0;
		t->countevals = 0;
		t->state = 0;
		t->dLastMinEWgroesserNull = 1.0;

		t->rgpc.resize( N );
		t->rgps.resize( N );
		t->rgdTmp.resize( N + 1 );
		t->rgBDz.resize( N );
		t->current_mean.resize( N ); // WTF? t->rgxmean[ 0 ] = N; ++t->rgxmean; fixed!
		t->rgxold.resize( N + 1 ); // WTF? t->rgxold[ 0 ] = N; ++t->rgxold; fixed!
		t->current_best.resize( N + 2 ); // WTF? t->rgxbestever[ 0 ] = N; ++t->rgxbestever; fixed!
		t->rgout.resize( N + 1 ); // WTF? t->rgout[ 0 ] = N; ++t->rgout; fixed!
		t->rgD.resize( N );
		t->C.resize( N );
		t->B.resize( N );
		t->publicFitness.resize( t->sp.lambda );
		t->rgFuncValue.resize( t->sp.lambda ); // WTF? t->rgFuncValue[ 0 ] = t->sp.lambda; ++t->rgFuncValue; fixed!
		t->arFuncValueHist.resize( 10 + (int)ceil( 3.*10.*N / t->sp.lambda ) );
		// WTF? t->arFuncValueHist[ 0 ] = (double)( 10 + (int)ceil( 3.*10.*N / t->sp.lambda ) ); t->arFuncValueHist++; fixed!

		for ( i = 0; i < N; ++i ) {
			t->C[ i ].resize( i + 1 );
			t->B[ i ].resize( N );
		}
		t->index.resize( t->sp.lambda );
		for ( i = 0; i < t->sp.lambda; ++i )
			t->index[ i ] = i; /* should not be necessary */
		t->current_pop.resize( t->sp.lambda );
		for ( i = 0; i < t->sp.lambda; ++i ) {
			t->current_pop[ i ].resize( N + 1 ); // WTF? t->rgrgx[ i ][ 0 ] = N; t->rgrgx[ i ]++; fixed!
		}

		/* Initialize newed space  */

		for ( i = 0; i < N; ++i )
			for ( j = 0; j < i; ++j )
				t->C[ i ][ j ] = t->B[ i ][ j ] = t->B[ j ][ i ] = 0.;

		for ( i = 0; i < N; ++i )
		{
			t->B[ i ][ i ] = 1.;
			t->C[ i ][ i ] = t->rgD[ i ] = t->sp.rgInitialStds[ i ] * sqrt( N / trace );
			t->C[ i ][ i ] = t->C[ i ][ i ] * t->C[ i ][ i ];
			t->rgpc[ i ] = t->rgps[ i ] = 0.;
		}

		t->minEW = *std::min_element( t->rgD.begin(), t->rgD.end() );
		t->minEW = t->minEW * t->minEW;
		t->maxEW = *std::max_element( t->rgD.begin(), t->rgD.end() );
		t->maxEW = t->maxEW * t->maxEW;

		t->maxdiagC = t->C[ 0 ][ 0 ]; for ( i = 1; i < N; ++i ) if ( t->maxdiagC < t->C[ i ][ i ] ) t->maxdiagC = t->C[ i ][ i ];
		t->mindiagC = t->C[ 0 ][ 0 ]; for ( i = 1; i < N; ++i ) if ( t->mindiagC > t->C[ i ][ i ] ) t->mindiagC = t->C[ i ][ i ];

		/* set xmean */
		for ( i = 0; i < N; ++i )
			t->current_mean[ i ] = t->rgxold[ i ] = t->sp.xstart[ i ];
	}



	static double myhypot( double a, double b )
		/* sqrt(a^2 + b^2) numerically stable. */
	{
		double r = 0;
		if ( fabs( a ) > fabs( b ) ) {
			r = b / a;
			r = fabs( a )*sqrt( 1 + r*r );
		}
		else if ( b != 0 ) {
			r = a / b;
			r = fabs( b )*sqrt( 1 + r*r );
		}
		return r;
	}

	/* ========================================================= */
	static void QLalgo2( int n, dbl_vec& d, dbl_vec& e, std::vector< dbl_vec >& V ) {
		/*
		  -> n     : Dimension.
		  -> d     : Diagonale of tridiagonal matrix.
		  -> e[1..n-1] : off-diagonal, output from Householder
		  -> V     : matrix output von Householder
		  <- d     : eigenvalues
		  <- e     : garbage?
		  <- V     : basis of eigenvectors, according to d

		  Symmetric tridiagonal QL algorithm, iterative
		  Computes the eigensystem from a tridiagonal matrix in roughtly 3N^3 operations

		  code adapted from Java JAMA package, function tql2.
		*/

		int i, k, l, m;
		double f = 0.0;
		double tst1 = 0.0;
		double eps = 2.22e-16; /* Math.pow(2.0,-52.0);  == 2.22e-16 */

			/* shift input e */
		for ( i = 1; i < n; i++ ) {
			e[ i - 1 ] = e[ i ];
		}
		e[ n - 1 ] = 0.0; /* never changed again */

		for ( l = 0; l < n; l++ ) {

			/* Find small subdiagonal element */
			if ( tst1 < fabs( d[ l ] ) + fabs( e[ l ] ) )
				tst1 = fabs( d[ l ] ) + fabs( e[ l ] );
			m = l;
			while ( m < n ) {
				if ( fabs( e[ m ] ) <= eps*tst1 ) {
					break;
				}
				m++;
			}

			/* If m == l, d[l] is an eigenvalue, */
			/* otherwise, iterate. */
			if ( m > l ) {  /* TODO: check the case m == n, should be rejected here!? */
				int iter = 0;
				do { /* while (fabs(e[l]) > eps*tst1); */
					double dl1, h;
					double g = d[ l ];
					double p = ( d[ l + 1 ] - g ) / ( 2.0 * e[ l ] );
					double r = myhypot( p, 1. );

					iter = iter + 1;  /* Could check iteration count here */

					/* Compute implicit shift */
					if ( p < 0 ) {
						r = -r;
					}
					d[ l ] = e[ l ] / ( p + r );
					d[ l + 1 ] = e[ l ] * ( p + r );
					dl1 = d[ l + 1 ];
					h = g - d[ l ];
					for ( i = l + 2; i < n; i++ ) {
						d[ i ] -= h;
					}
					f = f + h;

					/* Implicit QL transformation. */
					p = d[ m ];
					{
						double c = 1.0;
						double c2 = c;
						double c3 = c;
						double el1 = e[ l + 1 ];
						double s = 0.0;
						double s2 = 0.0;
						for ( i = m - 1; i >= l; i-- ) {
							c3 = c2;
							c2 = c;
							s2 = s;
							g = c * e[ i ];
							h = c * p;
							r = myhypot( p, e[ i ] );
							e[ i + 1 ] = s * r;
							s = e[ i ] / r;
							c = p / r;
							p = c * d[ i ] - s * g;
							d[ i + 1 ] = h + s * ( c * g + s * d[ i ] );

							/* Accumulate transformation. */
							for ( k = 0; k < n; k++ ) {
								h = V[ k ][ i + 1 ];
								V[ k ][ i + 1 ] = s * V[ k ][ i ] + c * h;
								V[ k ][ i ] = c * V[ k ][ i ] - s * h;
							}
						}
						p = -s * s2 * c3 * el1 * e[ l ] / dl1;
						e[ l ] = s * p;
						d[ l ] = c * p;
					}

					/* Check for convergence. */
				}
				while ( fabs( e[ l ] ) > eps*tst1 );
			}
			d[ l ] = d[ l ] + f;
			e[ l ] = 0.0;
		}

		/* Sort eigenvalues and corresponding vectors. */
#if 1
		/* TODO: really needed here? So far not, but practical and only O(n^2) */
		{
			int j;
			double p;
			for ( i = 0; i < n - 1; i++ ) {
				k = i;
				p = d[ i ];
				for ( j = i + 1; j < n; j++ ) {
					if ( d[ j ] < p ) {
						k = j;
						p = d[ j ];
					}
				}
				if ( k != i ) {
					d[ k ] = d[ i ];
					d[ i ] = p;
					for ( j = 0; j < n; j++ ) {
						p = V[ j ][ i ];
						V[ j ][ i ] = V[ j ][ k ];
						V[ j ][ k ] = p;
					}
				}
			}
		}
#endif 
	} /* QLalgo2 */

	static void Householder2( int n, std::vector< dbl_vec >& V, dbl_vec& d, dbl_vec& e )
	{
		/*
		   Householder transformation of a symmetric matrix V into tridiagonal form.
		 -> n             : dimension
		 -> V             : symmetric nxn-matrix
		 <- V             : orthogonal transformation matrix:
							tridiag matrix == V * V_in * V^t
		 <- d             : diagonal
		 <- e[0..n-1]     : off diagonal (elements 1..n-1)

		 code slightly adapted from the Java JAMA package, function private tred2()

		*/

		int i, j, k;

		for ( j = 0; j < n; j++ ) {
			d[ j ] = V[ n - 1 ][ j ];
		}

		/* Householder reduction to tridiagonal form */
		for ( i = n - 1; i > 0; i-- ) {

			/* Scale to avoid under/overflow */
			double scale = 0.0;
			double h = 0.0;
			for ( k = 0; k < i; k++ ) {
				scale = scale + fabs( d[ k ] );
			}
			if ( scale == 0.0 ) {
				e[ i ] = d[ i - 1 ];
				for ( j = 0; j < i; j++ ) {
					d[ j ] = V[ i - 1 ][ j ];
					V[ i ][ j ] = 0.0;
					V[ j ][ i ] = 0.0;
				}
			}
			else {

				/* Generate Householder vector */
				double f, g, hh;

				for ( k = 0; k < i; k++ ) {
					d[ k ] /= scale;
					h += d[ k ] * d[ k ];
				}
				f = d[ i - 1 ];
				g = sqrt( h );
				if ( f > 0 ) {
					g = -g;
				}
				e[ i ] = scale * g;
				h = h - f * g;
				d[ i - 1 ] = f - g;
				for ( j = 0; j < i; j++ ) {
					e[ j ] = 0.0;
				}

				/* Apply similarity transformation to remaining columns */
				for ( j = 0; j < i; j++ ) {
					f = d[ j ];
					V[ j ][ i ] = f;
					g = e[ j ] + V[ j ][ j ] * f;
					for ( k = j + 1; k <= i - 1; k++ ) {
						g += V[ k ][ j ] * d[ k ];
						e[ k ] += V[ k ][ j ] * f;
					}
					e[ j ] = g;
				}
				f = 0.0;
				for ( j = 0; j < i; j++ ) {
					e[ j ] /= h;
					f += e[ j ] * d[ j ];
				}
				hh = f / ( h + h );
				for ( j = 0; j < i; j++ ) {
					e[ j ] -= hh * d[ j ];
				}
				for ( j = 0; j < i; j++ ) {
					f = d[ j ];
					g = e[ j ];
					for ( k = j; k <= i - 1; k++ ) {
						V[ k ][ j ] -= ( f * e[ k ] + g * d[ k ] );
					}
					d[ j ] = V[ i - 1 ][ j ];
					V[ i ][ j ] = 0.0;
				}
			}
			d[ i ] = h;
		}

		/* Accumulate transformations */
		for ( i = 0; i < n - 1; i++ ) {
			double h;
			V[ n - 1 ][ i ] = V[ i ][ i ];
			V[ i ][ i ] = 1.0;
			h = d[ i + 1 ];
			if ( h != 0.0 ) {
				for ( k = 0; k <= i; k++ ) {
					d[ k ] = V[ k ][ i + 1 ] / h;
				}
				for ( j = 0; j <= i; j++ ) {
					double g = 0.0;
					for ( k = 0; k <= i; k++ ) {
						g += V[ k ][ i + 1 ] * V[ k ][ j ];
					}
					for ( k = 0; k <= i; k++ ) {
						V[ k ][ j ] -= g * d[ k ];
					}
				}
			}
			for ( k = 0; k <= i; k++ ) {
				V[ k ][ i + 1 ] = 0.0;
			}
		}
		for ( j = 0; j < n; j++ ) {
			d[ j ] = V[ n - 1 ][ j ];
			V[ n - 1 ][ j ] = 0.0;
		}
		V[ n - 1 ][ n - 1 ] = 1.0;
		e[ 0 ] = 0.0;

	} /* Housholder() */

	static void Eigen( int N, std::vector< dbl_vec >& C, dbl_vec& diag, std::vector< dbl_vec >& Q, dbl_vec& rgtmp )
		/*
		   Calculating eigenvalues and vectors.
		   Input:
			 N: dimension.
			 C: symmetric (1:N)xN-matrix, solely used to copy data to Q
			 niter: number of maximal iterations for QL-Algorithm.
			 rgtmp: N+1-dimensional vector for temporal use.
		   Output:
			 diag: N eigenvalues.
			 Q: Columns are normalized eigenvectors.
		 */
	{
		int i, j;

		/* copy C to Q */
		if ( C != Q ) {
			for ( i = 0; i < N; ++i )
				for ( j = 0; j <= i; ++j )
					Q[ i ][ j ] = Q[ j ][ i ] = C[ i ][ j ];
		}

		Householder2( N, Q, diag, rgtmp );
		QLalgo2( N, diag, rgtmp, Q );
	}

	static int Check_Eigen( int N, std::vector< dbl_vec >& C, dbl_vec& diag, std::vector< dbl_vec >& Q )
		/*
		   exhaustive test of the output of the eigendecomposition
		   needs O(n^3) operations

		   writes to error file
		   returns number of detected inaccuracies
		*/
	{
		/* compute Q diag Q^T and Q Q^T to check */
		int i, j, k, res = 0;
		double cc, dd;
		static char s[ 324 ];

		for ( i = 0; i < N; ++i )
			for ( j = 0; j < N; ++j ) {
				for ( cc = 0., dd = 0., k = 0; k < N; ++k ) {
					cc += diag[ k ] * Q[ i ][ k ] * Q[ j ][ k ];
					dd += Q[ i ][ k ] * Q[ j ][ k ];
				}
				/* check here, is the normalization the right one? */
				if ( fabs( cc - C[ i > j ? i : j ][ i > j ? j : i ] ) / sqrt( C[ i ][ i ] * C[ j ][ j ] ) > 1e-10
					&& fabs( cc - C[ i > j ? i : j ][ i > j ? j : i ] ) > 3e-14 ) {
					sprintf( s, "%d %d: %.17e %.17e, %e",
						i, j, cc, C[ i > j ? i : j ][ i > j ? j : i ], cc - C[ i > j ? i : j ][ i > j ? j : i ] );
					log::error( "pimpl_t:Eigen(): imprecise result detected ", s );
					++res;
				}
				if ( fabs( dd - ( i == j ) ) > 1e-10 ) {
					sprintf( s, "%d %d %.17e ", i, j, dd );
					log::error( "pimpl_t:Eigen(): imprecise result detected (Q not orthog.) ", s );
					++res;
				}
			}
		return res;
	}

	void cmaes_UpdateEigensystem( cmaes_t *t, int flgforce )
	{
		int i, N = t->sp.N;

		if ( flgforce == 0 ) {
			if ( t->flgEigensysIsUptodate == 1 )
				return;

			/* return on modulo generation number */
			if ( t->sp.updateCmode.flgalways == 0 /* not implemented, always ==0 */
				&& t->gen < t->genOfEigensysUpdate + t->sp.updateCmode.modulo
				)
				return;
		}

		Eigen( N, t->C, t->rgD, t->B, t->rgdTmp );

		/* find largest and smallest eigenvalue, they are supposed to be sorted anyway */
		t->minEW = *xo::min_element( t->rgD );
		t->maxEW = *xo::max_element( t->rgD );

		if ( t->flgCheckEigen )
			/* needs O(n^3)! writes, in case, error message in error file */
			i = Check_Eigen( N, t->C, t->rgD, t->B );

		for ( i = 0; i < N; ++i )
			t->rgD[ i ] = sqrt( t->rgD[ i ] );

		t->flgEigensysIsUptodate = 1;
		t->genOfEigensysUpdate = t->gen;
	} /* cmaes_UpdateEigensystem() */

	static void TestMinStdDevs( cmaes_t *t )
		/* increases sigma */
	{
		int i, N = t->sp.N;
		if ( t->sp.rgDiffMinChange == NULL )
			return;

		for ( i = 0; i < N; ++i )
			while ( t->sigma * sqrt( t->C[ i ][ i ] ) < t->sp.rgDiffMinChange[ i ] )
				t->sigma *= exp( 0.05 + t->sp.cs / t->sp.damps );

	} /* cmaes_TestMinStdDevs() */

	const std::vector< dbl_vec >& cmaes_SamplePopulation( cmaes_t *t )
	{
		int iNk, i, j, N = t->sp.N;
		int flgdiag = ( ( t->sp.diagonalCov == 1 ) || ( t->sp.diagonalCov >= t->gen ) );
		double sum;
		const auto& xmean = t->current_mean;

		/* cmaes_SetMean(t, xmean); * xmean could be changed at this point */

		/* calculate eigensystem  */
		if ( !t->flgEigensysIsUptodate ) {
			if ( !flgdiag )
				cmaes_UpdateEigensystem( t, 0 );
			else {
				for ( i = 0; i < N; ++i )
					t->rgD[ i ] = sqrt( t->C[ i ][ i ] );
				t->minEW = xo::squared( *xo::min_element( t->rgD ) );
				t->maxEW = xo::squared( *xo::max_element( t->rgD ) );
				t->flgEigensysIsUptodate = 1;
			}
		}

		/* treat minimal standard deviations and numeric problems */
		TestMinStdDevs( t );

		for ( iNk = 0; iNk < t->sp.lambda; ++iNk )
		{ /* generate scaled cmaes_random vector (D * z)    */
			for ( i = 0; i < N; ++i )
				if ( flgdiag )
					t->current_pop[ iNk ][ i ] = xmean[ i ] + t->sigma * t->rgD[ i ] * cmaes_random_Gauss( &t->rand );
				else
					t->rgdTmp[ i ] = t->rgD[ i ] * cmaes_random_Gauss( &t->rand );
			if ( !flgdiag )
				/* add mutation (sigma * B * (D*z)) */
				for ( i = 0; i < N; ++i ) {
					for ( j = 0, sum = 0.; j < N; ++j )
						sum += t->B[ i ][ j ] * t->rgdTmp[ j ];
					t->current_pop[ iNk ][ i ] = xmean[ i ] + t->sigma * sum;
				}
		}
		if ( t->state == 3 || t->gen == 0 )
			++t->gen;
		t->state = 1;

		return( t->current_pop );
	} /* SamplePopulation() */



	static void Sorted_index( const dbl_vec& rgFunVal, std::vector< int >& iindex, int n )
	{
		int i, j;
		for ( i = 1, iindex[ 0 ] = 0; i < n; ++i ) {
			for ( j = i; j > 0; --j ) {
				if ( rgFunVal[ iindex[ j - 1 ] ] < rgFunVal[ i ] )
					break;
				iindex[ j ] = iindex[ j - 1 ]; /* shift up */
			}
			iindex[ j ] = i; /* insert i */
		}
	}

	static void Adapt_C2( cmaes_t *t, int hsig )
	{
		int i, j, k, N = t->sp.N;
		int flgdiag = ( ( t->sp.diagonalCov == 1 ) || ( t->sp.diagonalCov >= t->gen ) );

		if ( t->sp.ccov != 0. && t->flgIniphase == 0 ) {

			/* definitions for speeding up inner-most loop */
			double ccov1 = std::min( t->sp.ccov * ( 1. / t->sp.mucov ) * ( flgdiag ? ( N + 1.5 ) / 3. : 1. ), 1. );
			double ccovmu = std::min( t->sp.ccov * ( 1 - 1. / t->sp.mucov )* ( flgdiag ? ( N + 1.5 ) / 3. : 1. ), 1. - ccov1 );
			double sigmasquare = t->sigma * t->sigma;

			t->flgEigensysIsUptodate = 0;

			/* update covariance matrix */
			for ( i = 0; i < N; ++i )
				for ( j = flgdiag ? i : 0; j <= i; ++j ) {
					t->C[ i ][ j ] = ( 1 - ccov1 - ccovmu ) * t->C[ i ][ j ]
						+ ccov1
						* ( t->rgpc[ i ] * t->rgpc[ j ]
							+ ( 1 - hsig )*t->sp.ccumcov*( 2. - t->sp.ccumcov ) * t->C[ i ][ j ] );
					for ( k = 0; k < t->sp.mu; ++k ) { /* additional rank mu update */
						t->C[ i ][ j ] += ccovmu * t->sp.weights[ k ]
							* ( t->current_pop[ t->index[ k ] ][ i ] - t->rgxold[ i ] )
							* ( t->current_pop[ t->index[ k ] ][ j ] - t->rgxold[ j ] )
							/ sigmasquare;
					}
				}
			/* update maximal and minimal diagonal value */
			t->maxdiagC = t->mindiagC = t->C[ 0 ][ 0 ];
			for ( i = 1; i < N; ++i ) {
				if ( t->maxdiagC < t->C[ i ][ i ] )
					t->maxdiagC = t->C[ i ][ i ];
				else if ( t->mindiagC > t->C[ i ][ i ] )
					t->mindiagC = t->C[ i ][ i ];
			}
		} /* if ccov... */
	}

	dbl_vec& cmaes_UpdateDistribution( cmaes_t *t, const dbl_vec& rgFunVal )
	{
		int i, j, iNk, hsig, N = t->sp.N;
		int flgdiag = ( ( t->sp.diagonalCov == 1 ) || ( t->sp.diagonalCov >= t->gen ) );
		double sum;
		double psxps;

		if ( t->state == 3 )
			xo_error( "cmaes_UpdateDistribution(): You need to call SamplePopulation() before update can take place." );
		if ( rgFunVal.size() != t->sp.lambda )
			xo_error( "cmaes_UpdateDistribution(): Fitness function value array input is missing." );

		if ( t->state == 1 )  /* function values are delivered here */
			t->countevals += t->sp.lambda;
		else
			xo_error( "cmaes_UpdateDistribution(): unexpected state" );

		/* assign function values */
		for ( i = 0; i < t->sp.lambda; ++i )
			t->current_pop[ i ][ N ] = t->rgFuncValue[ i ] = rgFunVal[ i ];


		/* Generate index */
		Sorted_index( rgFunVal, t->index, t->sp.lambda );

		/* Test if function values are identical, escape flat fitness */
		if ( t->rgFuncValue[ t->index[ 0 ] ] ==
			t->rgFuncValue[ t->index[ (int)t->sp.lambda / 2 ] ] ) {
			t->sigma *= exp( 0.2 + t->sp.cs / t->sp.damps );
			log::warning( "Warning: sigma increased due to equal function values. Reconsider the formulation of the objective function" );
		}

		/* update function value history */
		for ( i = (int)t->arFuncValueHist.size() - 1; i > 0; --i )
			t->arFuncValueHist[ i ] = t->arFuncValueHist[ i - 1 ];
		t->arFuncValueHist[ 0 ] = rgFunVal[ t->index[ 0 ] ];

		/* update xbestever */
		if ( t->current_best[ N ] > t->current_pop[ t->index[ 0 ] ][ N ] || t->gen == 1 )
			for ( i = 0; i <= N; ++i ) {
				t->current_best[ i ] = t->current_pop[ t->index[ 0 ] ][ i ];
				t->current_best[ N + 1 ] = t->countevals;
			}

		/* calculate xmean and rgBDz~N(0,C) */
		for ( i = 0; i < N; ++i ) {
			t->rgxold[ i ] = t->current_mean[ i ];
			t->current_mean[ i ] = 0.;
			for ( iNk = 0; iNk < t->sp.mu; ++iNk )
				t->current_mean[ i ] += t->sp.weights[ iNk ] * t->current_pop[ t->index[ iNk ] ][ i ];
			t->rgBDz[ i ] = sqrt( t->sp.mueff )*( t->current_mean[ i ] - t->rgxold[ i ] ) / t->sigma;
		}

		/* calculate z := D^(-1) * B^(-1) * rgBDz into rgdTmp */
		for ( i = 0; i < N; ++i ) {
			if ( !flgdiag )
				for ( j = 0, sum = 0.; j < N; ++j )
					sum += t->B[ j ][ i ] * t->rgBDz[ j ];
			else
				sum = t->rgBDz[ i ];
			t->rgdTmp[ i ] = sum / t->rgD[ i ];
		}

		/* cumulation for sigma (ps) using B*z */
		for ( i = 0; i < N; ++i ) {
			if ( !flgdiag )
				for ( j = 0, sum = 0.; j < N; ++j )
					sum += t->B[ i ][ j ] * t->rgdTmp[ j ];
			else
				sum = t->rgdTmp[ i ];
			t->rgps[ i ] = ( 1. - t->sp.cs ) * t->rgps[ i ] +
				sqrt( t->sp.cs * ( 2. - t->sp.cs ) ) * sum;
		}

		/* calculate norm(ps)^2 */
		for ( i = 0, psxps = 0.; i < N; ++i )
			psxps += t->rgps[ i ] * t->rgps[ i ];

		/* cumulation for covariance matrix (pc) using B*D*z~N(0,C) */
		hsig = sqrt( psxps ) / sqrt( 1. - pow( 1. - t->sp.cs, 2 * t->gen ) ) / t->chiN
			< 1.4 + 2. / ( N + 1 );
		for ( i = 0; i < N; ++i ) {
			t->rgpc[ i ] = ( 1. - t->sp.ccumcov ) * t->rgpc[ i ] +
				hsig * sqrt( t->sp.ccumcov * ( 2. - t->sp.ccumcov ) ) * t->rgBDz[ i ];
		}

		/* stop initial phase */
		if ( t->flgIniphase &&
			t->gen > std::min( 1 / t->sp.cs, 1 + N / t->sp.mucov ) )
		{
			if ( psxps / t->sp.damps / ( 1. - pow( ( 1. - t->sp.cs ), t->gen ) )
				< N * 1.05 )
				t->flgIniphase = 0;
		}

		/* update of C  */
		Adapt_C2( t, hsig );

		/* update of sigma */
		t->sigma *= exp( ( ( sqrt( psxps ) / t->chiN ) - 1. )*t->sp.cs / t->sp.damps );

		t->state = 3;

		return ( t->current_mean );

	} /* cmaes_UpdateDistribution() */

	//
	// Boundary transformation
	//

	typedef struct {
		dbl_vec lower_bounds; /* array of size len_of_bounds */
		dbl_vec upper_bounds; /* array of size len_of_bounds */
		dbl_vec al; /* "add"-on to lower boundary preimage, same length as bounds */
		dbl_vec au; /* add-on to upper boundary preimage, same length as bounds */
	} cmaes_boundary_trans_t;

	void cmaes_boundary_trans_init( cmaes_boundary_trans_t *t, const dbl_vec& lower, const dbl_vec& upper )
	{
		xo_assert( upper.size() == lower.size() );
		auto l = lower.size();

		t->lower_bounds = lower;
		t->upper_bounds = upper;

		/* compute boundaries in pre-image space, al and au */
		t->al.resize( l );
		t->au.resize( l );

		auto& lb = t->lower_bounds;
		auto& ub = t->upper_bounds;
		for ( int i = 0; i < l; ++i ) {
			if ( lb[ i ] == ub[ i ] )
				xo_error( "Lower and upper bounds must be different in all variables" );
			/* between lb+al and ub-au transformation is the identity */
			t->al[ i ] = fmin( ( ub[ i ] - lb[ i ] ) / 2., ( 1. + fabs( lb[ i ] ) ) / 20. );
			t->au[ i ] = fmin( ( ub[ i ] - lb[ i ] ) / 2., ( 1. + fabs( ub[ i ] ) ) / 20. );
		}
	}

	void cmaes_boundary_trans_shift_into_feasible_preimage( cmaes_boundary_trans_t *t, const dbl_vec& x, dbl_vec& y )
	{
		auto len = t->lower_bounds.size();
		for ( size_t i = 0; i < len; ++i ) {
			double lb, ub, al, au, r, xlow, xup;
			lb = t->lower_bounds[ i ];
			ub = t->upper_bounds[ i ];
			al = t->al[ i ];
			au = t->au[ i ];
			xlow = lb - 2 * al - ( ub - lb ) / 2.0;
			xup = ub + 2 * au + ( ub - lb ) / 2.0;
			r = 2 * ( ub - lb + al + au ); /* == xup - xlow == period of the transformation */

			y[ i ] = x[ i ];

			if ( y[ i ] < xlow ) { /* shift up */
				y[ i ] += r * ( 1 + (int)( ( xlow - y[ i ] ) / r ) );
			}
			if ( y[ i ] > xup ) { /* shift down */
				y[ i ] -= r * ( 1 + (int)( ( y[ i ] - xup ) / r ) );
				/* printf(" \n%f\n", fmod(y[i] - ub - au, r)); */
			}
			if ( y[ i ] < lb - al ) /* mirror */
				y[ i ] += 2 * ( lb - al - y[ i ] );
			if ( y[ i ] > ub + au )
				y[ i ] -= 2 * ( y[ i ] - ub - au );

			if ( ( y[ i ] < lb - al - 1e-15 ) || ( y[ i ] > ub + au + 1e-15 ) ) {
				xo_error( stringf( "BUG in cmaes_boundary_transformation_shift_into_feasible_preimage: lb=%f, ub=%f, al=%f au=%f, y=%f\n", lb, ub, al, au, y[ i ] ) );
			}
		}
	}

	void cmaes_boundary_trans( cmaes_boundary_trans_t *t, const dbl_vec& x, dbl_vec& y )
	{
		auto len = t->lower_bounds.size();
		cmaes_boundary_trans_shift_into_feasible_preimage( t, x, y );
		for ( size_t i = 0; i < len; ++i ) {
			double lb, ub, al, au;
			lb = t->lower_bounds[ i ];
			ub = t->upper_bounds[ i ];
			al = t->al[ i ];
			au = t->au[ i ];
			if ( y[ i ] < lb + al )
				y[ i ] = lb + ( y[ i ] - ( lb - al ) ) * ( y[ i ] - ( lb - al ) ) / 4. / al;
			else if ( y[ i ] > ub - au )
				y[ i ] = ub - ( y[ i ] - ( ub + au ) ) * ( y[ i ] - ( ub + au ) ) / 4. / au;
		}
	}

	void cmaes_boundary_trans_inverse( cmaes_boundary_trans_t *t, const dbl_vec& x, dbl_vec& y )
	{
		auto len = t->lower_bounds.size();
		unsigned long i;

		for ( i = 0; i < len; ++i ) {
			double lb, ub, al, au;
			lb = t->lower_bounds[ i ];
			ub = t->upper_bounds[ i ];
			al = t->al[ i ];
			au = t->au[ i ];
			y[ i ] = x[ i ];
			if ( y[ i ] < lb + al )
				y[ i ] = ( lb - al ) + 2 * sqrt( al * ( y[ i ] - lb ) );
			else if ( y[ i ] > ub - au )
				y[ i ] = ( ub + au ) - 2 * sqrt( au * ( ub - y[ i ] ) );
		}
	}

	struct pimpl_t
	{
		cmaes_t cmaes;
		cmaes_boundary_trans_t bounds;
		search_point_vec bounded_pop;

		vector< double > get_bounded( const vector< double >& params )
		{
			xo_assert( cmaes.sp.N == params.size() );
			if ( !bounds.lower_bounds.empty() )
			{
				vector< double > bounded_pars( params.size() );
				cmaes_boundary_trans( &bounds, params, bounded_pars );
				return bounded_pars;
			}
			else return params;
		}
	};

	cma_optimizer::cma_optimizer( const objective& obj, int l, int seed, cma_weights w ) :
	optimizer( obj )
	{
		pimpl = new pimpl_t;
		auto n = objective_.info().dim();

		std::vector< double > mean( n ), std( n ), lb( n ), ub( n );
		for ( index_t i = 0; i < n; ++i )
		{
			auto& p = objective_.info()[ i ];
			mean[ i ] = p.mean;
			std[ i ] = p.std;
			lb[ i ] = p.min;
			ub[ i ] = p.max;
		}

		cmaes_init( &pimpl->cmaes, (int)n, mean, std, seed, l );
		cmaes_readpara_SupplementDefaults( &pimpl->cmaes );
		cmaes_init_final( &pimpl->cmaes );

		pimpl->bounded_pop.resize( lambda(), search_point( objective_.info() ) );
		cmaes_boundary_trans_init( &pimpl->bounds, lb, ub );

		name = obj.name() + stringf( ".R%d", random_seed() );
		add_stop_condition< flat_fitness_condition >( 1e-6 );
	}

	cma_optimizer::~cma_optimizer()
	{
		delete pimpl;
	}

	const search_point_vec& cma_optimizer::sample_population()
	{
		auto& pop = cmaes_SamplePopulation( &pimpl->cmaes );
		for ( index_t pop_idx = 0; pop_idx < pop.size(); ++pop_idx )
		{
			if ( pimpl->bounds.lower_bounds.size() > 0 )
			{
				// apply transform
				dbl_vec bounded_values( dim() );
				cmaes_boundary_trans( &pimpl->bounds, pop[ pop_idx ], bounded_values );
				pimpl->bounded_pop[ pop_idx ].set_values( bounded_values );
			}
			else
			{
				// simply copy
				pimpl->bounded_pop[ pop_idx ].set_values( pop[ pop_idx ] );
			}
		}

		return pimpl->bounded_pop;
	}

	void cma_optimizer::update_distribution( const fitness_vec_t& results )
	{
		if ( objective_.info().maximize() )
		{
			// negate first, since c-cmaes always minimizes
			vector< fitness_t > neg_results( results.size() );
			std::transform( results.begin(), results.end(), neg_results.begin(), [&]( const double& v ) { return -v; } );
			cmaes_UpdateDistribution( &pimpl->cmaes, neg_results );
		}
		else cmaes_UpdateDistribution( &pimpl->cmaes, results );
	}

	vector< double > cma_optimizer::current_mean() const
	{
		return pimpl->get_bounded( pimpl->cmaes.current_mean );
	}

	vector< double > cma_optimizer::current_std() const
	{
		// get from covariance matrix
		vector< double > stds( dim() );
		for ( index_t i = 0; i < dim(); ++i )
			stds[ i ] = pimpl->cmaes.sigma * sqrt( pimpl->cmaes.C[ i ][ i ] );

		return stds;
	}

	vector< par_vec > cma_optimizer::current_covariance() const
	{
		return pimpl->cmaes.C;
	}

	void cma_optimizer::save_state( const path& filename ) const
	{
		XO_NOT_IMPLEMENTED;
	}

	spot::objective_info cma_optimizer::make_updated_objective_info() const
	{
		objective_info inf( info() );
		inf.set_mean_std( current_mean(), current_std() );
		return inf;
	}

	int cma_optimizer::lambda() const
	{
		return pimpl->cmaes.sp.lambda;
	}

	int cma_optimizer::mu() const
	{
		return pimpl->cmaes.sp.mu;
	}

	int cma_optimizer::dim() const
	{
		return pimpl->cmaes.sp.N;
	}

	int cma_optimizer::random_seed() const
	{
		return pimpl->cmaes.sp.seed;
	}

	double cma_optimizer::sigma() const
	{
		return pimpl->cmaes.sigma;
	}

	void cma_optimizer::internal_step()
	{
		auto& pop = sample_population();
		auto results = evaluate( pop );
		update_distribution( results );
	}
}
