#pragma once

#include "objective_info.h"

namespace spot
{
	class boundary_transformer
	{
	public:
		boundary_transformer( const objective_info& i ) : info_( i ) {}
		virtual ~boundary_transformer() {}
		virtual void apply( par_vec& v ) {};
		virtual void apply_inverse( par_vec& v ) {};

	protected:
		const objective_info& info_;
	};

	class soft_limit_boundary_transformer : public boundary_transformer
	{
	public:
		soft_limit_boundary_transformer( const objective_info& i, par_value threshold = 0.1 );
		virtual void apply( par_vec& v );
		virtual void apply_inverse( par_vec& v );
	private:
		par_value boundary_limit_threshold_;
	};

	class cmaes_boundary_transformer : public boundary_transformer
	{
	public:
		cmaes_boundary_transformer( const objective_info& i );
		virtual void apply( par_vec& v );;
		virtual void apply_inverse( par_vec& v );;
	private:
		par_vec lb_; /* array of size len_of_bounds */
		par_vec ub_; /* array of size len_of_bounds */
		par_vec al_; /* "add"-on to lower boundary preimage, same length as bounds */
		par_vec au_; /* add-on to upper boundary preimage, same length as bounds */

	};
}
