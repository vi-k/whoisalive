#ifndef MY_TIME_H
#define MY_TIME_H

#include "my_exception.h"

#include <sstream>
#include <istream>
#include <ostream>
#include <string>
#include <locale> /* facet */

#include <boost/date_time/gregorian/gregorian.hpp>
namespace gregorian=boost::gregorian;
#include <boost/date_time/posix_time/posix_time.hpp>
namespace posix_time=boost::posix_time;
#include "boost/date_time/local_time/local_time.hpp"
namespace local_time=boost::local_time;

#include <boost/functional/hash.hpp>

/* hash_value для boost::unordered_map */
namespace boost {
std::size_t hash_value(const posix_time::ptime &t);
}

namespace my { namespace time {

/* hash для std::unordered_map */
struct ptime_hash : std::unary_function<posix_time::ptime, std::size_t>
{
	std::size_t operator()(const posix_time::ptime &t) const
	{
		int size = sizeof(t) / sizeof(size_t);

		size_t seed = 0;
		size_t *ptr = (size_t*)&t;

		while (size--)
			boost::hash_combine(seed, *ptr);

		return seed;
   	}
};

posix_time::ptime utc_to_local(const posix_time::ptime &utc_time);
posix_time::ptime local_to_utc(const posix_time::ptime &local_time);

/* Функция медленная для частого применения! :( */
template<class Char>
inline void set_output_format(std::basic_ostream<Char> &out,
	const Char *time_fmt, const Char *duration_fmt = 0)
{
	typedef boost::date_time::time_facet<posix_time::ptime, Char> time_facet;
	time_facet *f = new time_facet(time_fmt);
	f->time_duration_format(duration_fmt ? duration_fmt : time_fmt);
	out.imbue( std::locale(out.getloc(), f) );
}

/* Функция медленная для частого применения! :( */
template<class Char>
inline void set_input_format(std::basic_istream<Char> &in,
	const Char *time_fmt, const Char *duration_fmt = 0)
{
	typedef boost::date_time::time_input_facet<posix_time::ptime, Char> time_facet;
	time_facet *f = new time_facet(time_fmt);
	f->time_duration_format(duration_fmt ? duration_fmt : time_fmt);
	in.imbue( std::locale(in.getloc(), f) );
}

template<class Char, class TimeFacet>
class mydef_format : public TimeFacet
{
public:	
	mydef_format() : TimeFacet(get_def_time_fmt())
	{
		static const Char def_duration_fmt[] =
		{
			'%', 'H', ':', '%', 'M', ':', '%', 'S', '%', 'F', '\0'
		};

		time_duration_format(def_duration_fmt);
	}

	inline static const Char* get_def_time_fmt()
	{
		static const Char def_time_fmt[] =
		{
			'%', 'Y', '-', '%', 'm', '-', '%', 'd', ' ',
			'%', 'H', ':', '%', 'M', ':', '%', 'S', '%', 'F', '\0'
		};
		return def_time_fmt;
	}
};

/* Функция быстрая для частого применения на одном и том же потоке */
template<class Char>
inline void set_mydef_output_format(std::basic_ostream<Char> &out)
{
	typedef mydef_format<Char,
		boost::date_time::time_facet<posix_time::ptime, Char> > my_format;
	if (!has_facet<my_format>(out.getloc()))
		out.imbue( std::locale(out.getloc(), new my_format) );
}

/* Функция быстрая для частого применения на одном и том же потоке */
template<class Char>
inline void set_mydef_input_format(std::basic_istream<Char> &in)
{
	typedef mydef_format<Char,
		boost::date_time::time_input_facet<posix_time::ptime, Char> > my_format;
	if (!has_facet<my_format>(in.getloc()))
		in.imbue( std::locale(in.getloc(), new my_format) );
}


template<class Char, class Time>
std::basic_string<Char> format(const Char *fmt,
	const Time &time)
{
	basic_ostringstream<Char> out;
	set_output_format(out, fmt);
	out << time;
	return out.str();
}


template<class Char, class Time>
std::basic_string<Char> to_str(const Time &time)
{
	basic_ostringstream<Char> out;
	set_mydef_output_format(out);
	out << time;
	return out.str();
}

template<class Time>
inline std::string to_string(const Time &time)
	{ return to_str<char>(time); }

template<class Time>
inline std::wstring to_wstring(const Time &time)
	{ return to_str<wchar_t>(time); }


template<class Time>
void throw_if_fail(const Time &time,
	const std::wstring &text = std::wstring(L"Invalid time"))
{
	if (time.is_special())
		throw my::exception(text)
			<< my::param(L"time", time);
}


template<class Time, class Char>
Time format_to(
	const Char *fmt,
	const std::basic_string<Char> &str)
{
    Time time;
	std::basic_istringstream<Char> in(str);
	set_input_format(in, fmt);
	in >> time;
	if (!in)
		time = posix_time::not_a_date_time;
	return time;
}

template<class Char>
inline posix_time::ptime format_to_time(
	const Char *fmt,
	const std::basic_string<Char> &str)
{
    return format_to<posix_time::ptime>(fmt, str);
}

template<class Char>
inline posix_time::time_duration format_to_duration(
	const Char *fmt,
	const std::basic_string<Char> &str)
{
    return format_to<posix_time::time_duration>(fmt, str);
}

template<class Time, class Char>
Time str_to(const std::basic_string<Char> &str)
{
    Time time;
	std::basic_istringstream<Char> in(str);
	set_mydef_input_format(in);
	in >> time;
	if (!in)
		time = posix_time::not_a_date_time;
	return time;
}

template<class Char>
inline posix_time::ptime to_time(const std::basic_string<Char> &str)
{
	return str_to<posix_time::ptime>(str);
}

template<class Char>
inline posix_time::time_duration to_duration(const std::basic_string<Char> &str)
{
	return str_to<posix_time::time_duration>(str);
}

}}

#endif
