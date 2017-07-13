#pragma once

#include "flut/system/test_framework.hpp"
#include "c-cmaes/cmaes_interface.h"
#include "spot/cma_optimizer.h"

namespace spot
{
	void optimizer_test();
	void optimizer_thread_test();
}
