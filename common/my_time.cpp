#include "my_time.h"
#include "my_exception.h"

using namespace std;

#include <boost/date_time/time_parsing.hpp>
#include "boost/date_time/c_local_time_adjustor.hpp"

namespace my { namespace time {

posix_time::ptime utc_to_local(const posix_time::ptime &utc_time)
{
	return boost::date_time::c_local_adjustor<posix_time::ptime>
		::utc_to_local(utc_time);
}

posix_time::ptime local_to_utc(const posix_time::ptime &local_time)
{
	/* Прямой функции для c_local_adjustor почему-то нет */
	posix_time::time_duration td = utc_to_local(local_time) - local_time;
	return local_time - td;
}

}}
