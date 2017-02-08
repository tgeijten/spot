#pragma once

#include "flut/prop_node.hpp"
#include "ss/core/types.h"
#include "flut/system/path.hpp"
#include "ss/optimizer/objectives/objective.h"
#include "ss/optimizer/par_set.h"
#include "ss/core/simulator.h"

namespace ss
{
	class SS_API optimizer
	{
	public:
		optimizer( const simulator_factory& f, const prop_node& pn  );
		virtual ~optimizer() {}

		u_ptr< objective > objective_;

		std::vector< double > evaluate( const prop_node& builder_pn, std::vector< par_set >& ps_vec );
		virtual void optimize( const prop_node& builder_pn ) = 0;

	protected:
		path output_path;
		bool maximize;
		bool is_better( double a, double b ) { return maximize ? a > b : a < b; }
		
	private:
		const simulator_factory& factory_;
	};
}
