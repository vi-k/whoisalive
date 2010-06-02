#ifndef MY_TIME_H
#define MY_TIME_H

#include <string>

#include <boost/date_time/posix_time/posix_time.hpp>
namespace posix_time=boost::posix_time;

#include "boost/date_time/local_time/local_time.hpp"
namespace local_time=boost::local_time;

namespace my { namespace time
{

posix_time::ptime utc_to_local(const posix_time::ptime &utc_time);
posix_time::ptime local_to_utc(const posix_time::ptime &local_time);

template<class Char>
inline void set_format(std::basic_ostream<Char> &out,
	const Char *time_fmt = 0, const Char *duration_fmt = 0)
{
	static const Char def_time_fmt[] =
	{
		'%', 'Y', '-', '%', 'm', '-', '%', 'd', ' ',
		'%', 'H', ':', '%', 'M', ':', '%', 'S', '%', 'F', '\0'
	};

	static const Char def_duration_fmt[] =
	{
		'%', 'H', ':', '%', 'M', ':', '%', 'S', '%', 'F', '\0'
	};

	typedef boost::date_time::time_facet<posix_time::ptime, Char> time_facet;

	time_facet *f = time_fmt ? new time_facet(time_fmt)
		: new time_facet(def_time_fmt);

	f->time_duration_format(duration_fmt ? duration_fmt : def_duration_fmt);

	out.imbue( std::locale(out.getloc(), f) );
}

template<class Char>
inline void set_input_format(std::basic_istream<Char> &in,
	const Char *time_fmt = 0, const Char *duration_fmt = 0)
{
	static const Char def_time_fmt[] =
	{
		'%', 'Y', '-', '%', 'm', '-', '%', 'd', ' ',
		'%', 'H', ':', '%', 'M', ':', '%', 'S', '%', 'F', '\0'
	};

	static const Char def_duration_fmt[] =
	{
		'%', 'H', ':', '%', 'M', ':', '%', 'S', '%', 'F', '\0'
	};

	typedef boost::date_time::time_input_facet<posix_time::ptime, Char> time_facet;

	time_facet *f = time_fmt ? new time_facet(time_fmt)
		: new time_facet(def_time_fmt);

	f->time_duration_format(duration_fmt ? duration_fmt : def_duration_fmt);

	in.imbue( std::locale(in.getloc(), f) );
}

template<class Char>
std::basic_string<Char> format(const Char *fmt,
	const posix_time::ptime &time)
{
	basic_stringstream<Char> out;
	set_format(out, fmt);
	out << time;
	return out.str();
}

template<class Char>
std::basic_string<Char> format(const std::basic_string<Char> &fmt,
	const posix_time::ptime &time)
{
	basic_stringstream<Char> out;
	set_format(out, fmt.c_str());
	out << time;
	return out.str();
}

template<class Char>
std::basic_string<Char> format(const Char *fmt,
	const posix_time::time_duration &time)
{
	basic_stringstream<Char> out;
	set_format(out, (const Char*)0, fmt);
	out << time;
	return out.str();
}

template<class Char>
std::basic_string<Char> format(const std::basic_string<Char> &fmt,
	const posix_time::time_duration &time)
{
	basic_stringstream<Char> out;
	set_format(out, (const Char*)0, fmt.c_str());
	out << time;
	return out.str();
}

std::wstring to_fmt_wstring(const wchar_t *fmt,
	const posix_time::ptime &time);
std::wstring to_fmt_wstring(const wchar_t *fmt,
	const posix_time::time_duration &time);

std::string to_fmt_string(const char *fmt,
	const posix_time::ptime &time);
std::string to_fmt_string(const char *fmt,
	const posix_time::time_duration &time);

inline std::wstring to_wstring(const posix_time::ptime &time)
	{ return my::time::to_fmt_wstring(L"%Y-%m-%d %H:%M:%S%F", time); }
inline std::wstring to_wstring(const posix_time::time_duration &time)
	{ return to_fmt_wstring(L"%-%H:%M:%S%F", time); }

inline std::string to_string(const posix_time::ptime &time)
	{ return my::time::to_fmt_string("%Y-%m-%d %H:%M:%S%F", time); }
inline std::string to_string(const posix_time::time_duration &time)
	{ return to_fmt_string("%-%H:%M:%S%F", time); }

void throw_if_fail(const posix_time::ptime &time);
void throw_if_fail(const posix_time::time_duration &time);

posix_time::time_duration to_duration(const std::string &str);
posix_time::time_duration to_duration(const std::wstring &str);

posix_time::ptime to_time_fmt(const wchar_t *str, const wchar_t *fmt);
inline posix_time::ptime to_time_fmt(const std::wstring &str, const wchar_t *fmt)
	{ return to_time_fmt(str.c_str(), fmt); }
inline posix_time::ptime to_time(const wchar_t *str)
	{ return to_time_fmt(str, L"%Y-%m-%d %H:%M:%S%F"); }
inline posix_time::ptime to_time(const std::wstring &str)
	{ return to_time_fmt(str.c_str(), L"%Y-%m-%d %H:%M:%S%F"); }

} }

#endif
