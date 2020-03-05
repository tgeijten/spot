#include "boundary_transformer.h"

#include "xo/system/assert.h"
#include "xo/system/log.h"
#include "xo/numerical/math.h"
#include "xo/string/string_tools.h"

namespace spot
{

	soft_limit_boundary_transformer::soft_limit_boundary_transformer( const objective_info& i, par_t threshold ) :
	boundary_transformer( i ),
	boundary_limit_threshold_( threshold )
	{
	}

	void soft_limit_boundary_transformer::apply( par_vec& v )
	{
		xo_assert( v.size() == info_.dim() );
		for ( index_t i = 0; i < v.size(); ++i )
			xo::soft_clamp( v[ i ], info_[ i ].min, info_[ i ].max, boundary_limit_threshold_ );
	}

	void soft_limit_boundary_transformer::apply_inverse( par_vec& v )
	{
		XO_NOT_IMPLEMENTED;
	}

	cmaes_boundary_transformer::cmaes_boundary_transformer( const objective_info& info ) :
	boundary_transformer( info )
	{
		auto len = info_.dim();
		lb_.resize( len );
		ub_.resize( len );
		al_.resize( len );
		au_.resize( len );

		for ( index_t i = 0; i < len; ++i )
		{
			lb_[ i ] = info_[ i ].min;
			ub_[ i ] = info_[ i ].max;

			if ( lb_[ i ] == ub_[ i ] || ub_[ i ] < lb_[ i ] )
				xo_error( "Invalid upper and lower bounds for parameter " + info[ i ].name );

			/* between lb+al and ub-au transformation is the identity */
			al_[ i ] = fmin( ( ub_[ i ] - lb_[ i ] ) / 2., ( 1. + fabs( lb_[ i ] ) ) / 20. );
			au_[ i ] = fmin( ( ub_[ i ] - lb_[ i ] ) / 2., ( 1. + fabs( ub_[ i ] ) ) / 20. );
		}
	}

	void cmaes_boundary_transformer::apply( par_vec& x )
	{
		auto len = lb_.size();

		// cmaes_boundary_trans_shift_into_feasible_preimage
		for ( size_t i = 0; i < len; ++i ) {
			double lb, ub, al, au, r, xlow, xup;
			lb = lb_[ i ];
			ub = ub_[ i ];
			al = al_[ i ];
			au = au_[ i ];
			xlow = lb - 2 * al - ( ub - lb ) / 2.0;
			xup = ub + 2 * au + ( ub - lb ) / 2.0;
			r = 2 * ( ub - lb + al + au ); /* == xup - xlow == period of the transformation */

			auto y = x[ i ];

			if ( y < xlow ) { /* shift up */
				y += r * ( 1 + (int)( ( xlow - y ) / r ) );
			}
			if ( y > xup ) { /* shift down */
				y -= r * ( 1 + (int)( ( y - xup ) / r ) );
				/* printf(" \n%f\n", fmod(y[i] - ub - au, r)); */
			}
			if ( y < lb - al ) /* mirror */
				y += 2 * ( lb - al - y );
			if ( y > ub + au )
				y -= 2 * ( y - ub - au );

			if ( ( y < lb - al - 1e-15 ) || ( y > ub + au + 1e-15 ) ) {
				xo::log::error( xo::stringf( "BUG in cmaes_boundary_transformation_shift_into_feasible_preimage: lb=%f, ub=%f, al=%f au=%f, x=%f, y=%f, i=%d\n", lb, ub, al, au, x[ i ], y, i ) );
			}
			else x[ i ] = y;
		}

		for ( size_t i = 0; i < len; ++i ) {
			double lb, ub, al, au;
			lb = lb_[ i ];
			ub = ub_[ i ];
			al = al_[ i ];
			au = au_[ i ];
			auto y = x[ i ];
			if ( y < lb + al )
				y = lb + ( y - ( lb - al ) ) * ( y - ( lb - al ) ) / 4. / al;
			else if ( y > ub - au )
				y = ub - ( y - ( ub + au ) ) * ( y - ( ub + au ) ) / 4. / au;
			x[ i ] = y;
		}
	}

	void cmaes_boundary_transformer::apply_inverse( par_vec& v )
	{
		XO_NOT_IMPLEMENTED;
	}
}
