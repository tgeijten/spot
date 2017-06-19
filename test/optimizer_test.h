#pragma once

#include "flut/system/test_framework.hpp"
#include "c-cmaes/cmaes_interface.h"
#include "flut/optimization/cma_optimizer.hpp"

/* the objective (fitness) function to be minimized */
double fitfun_c( double const *x, int N );

double fitfun( const std::vector< double >& x );

namespace flut
{
	void optimizer_test();
	void optimizer_thread_test();
}
