#include "objective.h"

#include <future>

#include "xo/system/system_tools.h"
#include "xo/system/log.h"

namespace spot
{
	result<fitness_t> objective::evaluate_noexcept( const search_point& point, const xo::stop_token& st ) const noexcept
	{
		try
		{
			return evaluate( point, st );
		}
		catch ( std::exception& e )
		{
			return xo::error_message( e.what() );
		}
		catch ( ... )
		{
			return xo::error_message( "Unknown exception while evaluating objective" );
		}
	}
}
