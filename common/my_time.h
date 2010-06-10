/*
	Смысл всех этих самописных функций преобразования в том, что
	стандартные функции из boost::date_time грандиозно медленны
	
	Тест (преобразование 1000 строк, avg - среднее время):

	1) my::time::to_duration: avg=00:00:00.000250
	2) duration_from_string:  avg=00:00:00.014063 (в 56 раз)
	3) istringstream >> (без создания потока на каждой итерации):
	                          avg=00:00:00.013594 (в 54 раза)
	4) + создание потока:     avg=00:00:00.921929 (в 3690 раз)
	5) + форматирование:      avg=00:00:00.945355 (в 3780 раз)
*/

#ifndef MY_TIME_H
#define MY_TIME_H

#include "my_exception.h"
#include "my_num.h"
#include "my_str.h"

#include <cstddef> /* std::size_t */
#include <sstream>
#include <istream>
#include <ostream>
#include <string>
#include <locale> /* facet */

#include <boost/date_time/special_defs.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "boost/date_time/local_time/local_time.hpp"

namespace gregorian=boost::gregorian;
namespace posix_time=boost::posix_time;
namespace local_time=boost::local_time;
namespace date_time=boost::date_time;

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
		int size = sizeof(t) / sizeof(std::size_t);

		std::size_t seed = 0;
		std::size_t *ptr = (std::size_t*)&t;

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
	int a_;

	mydef_format() : a_(123), TimeFacet(get_def_time_fmt())
	{
		static const Char def_duration_fmt[]
			= { '%','H',':','%','M',':','%','S','%','F', 0 };

		time_duration_format(def_duration_fmt);
	}

	inline static const Char* get_def_time_fmt()
	{
		static const Char def_time_fmt[]
			= { '%','Y','-','%','m','-','%','d',' ',
			'%','H',':','%','M',':','%','S','%','F', 0 };
		return def_time_fmt;
	}
};

/* Функция быстрая для частого применения на одном и том же потоке */
template<class Char>
inline void set_mydef_output_format(std::basic_ostream<Char> &out)
{
	typedef mydef_format<Char,
		boost::date_time::time_facet<posix_time::ptime, Char> > my_format;
	//std::cout << "<<<" << endl;
	if (has_facet<my_format>(out.getloc()))
	{
		//std::cout << "has_facet" << endl;
		//std::cout << *(int*)&use_facet<my_format>(out.getloc()) << endl;
		//std::cout << use_facet<my_format>(out.getloc()).a_ << endl;
	}
	else
	{
		//std::cout << "!has_facet" << endl;
		out.imbue( std::locale(out.getloc(), new my_format) );
	}
	//std::cout << ">>>" << endl;
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

/*-
template<class Char, class Time>
std::basic_string<Char> to_str(const Time &time)
{
	basic_ostringstream<Char> out;
	set_mydef_output_format(out);
	out << time;
	return out.str();
}
-*/

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

/*
	Вывод даты/времени в строку
*/

template<class Time>
date_time::special_values as_special(const Time &t)
{
	if (t.is_not_a_date_time())
		return date_time::not_a_date_time;
	else if (t.is_neg_infinity())
		return date_time::neg_infin;
	else if (t.is_pos_infinity())
		return date_time::pos_infin;

	return date_time::not_special;
}

template<class Char>
std::size_t put(Char *buf, std::size_t buf_sz,
	const date_time::special_values &sv)
{
	static const Char neg_infinity[]
		= { '-','i','n','f','i','n','i','t','y', 0 };
	static const Char pos_infinity[]
		= { '+','i','n','f','i','n','i','t','y', 0 };
	static const Char not_a_date_time[]
		= { 'n','o','t','-','a','-','d','a','t','e',
			'-','t','i','m','e', 0 };
	static const Char not_special[]
		= { 'n','o','t','-','s','p','e','c','i','a','l', 0 };

	Char *ptr = buf;
	Char *end = buf + buf_sz;

	switch (sv)
	{
		case date_time::not_a_date_time:
			ptr += my::str::put(ptr, end - ptr, not_a_date_time);
			break;

		case date_time::neg_infin:
			ptr += my::str::put(ptr, end - ptr, neg_infinity);
			break;
		
		case date_time::pos_infin:
			ptr += my::str::put(ptr, end - ptr, pos_infinity);
			break;
		
		default:
			ptr += my::str::put(ptr, end - ptr, not_special);
	}

	return ptr - buf;
}

/* Преобразование даты (boost::gregorian::date) в строку */
template<class Char>
std::size_t put(Char *buf, std::size_t buf_sz,
	const gregorian::date &date)
{
	if (date.is_special())
		return put(buf, buf_sz, date.as_special());

	Char *ptr = buf;
	Char *end = buf + buf_sz;

	gregorian::date::ymd_type ymd = date.year_month_day();

	ptr += my::num::put(ptr, end - ptr, ymd.year, 4);
	ptr += my::str::put(ptr, end - ptr, Char('-'));
	ptr += my::num::put(ptr, end - ptr, ymd.month, 2);
	ptr += my::str::put(ptr, end - ptr, Char('-'));
	ptr += my::num::put(ptr, end - ptr, ymd.day, 2);

	return ptr - buf;
}

/* Преобразование даты/времени (boost::posix_time::ptime) в строку */
template<class Char>
std::size_t put(Char *buf, std::size_t buf_sz,
	const posix_time::ptime &time)
{
	if (time.is_special())
		return put(buf, buf_sz, my::time::as_special(time));

	Char *ptr = buf;
	Char *end = buf + buf_sz;

	ptr += put(ptr, end - ptr, time.date());
	ptr += my::str::put(ptr, end - ptr, Char(' '));
	ptr += put(ptr, end - ptr, time.time_of_day());

	return ptr - buf;
}

/* Преобразование длительности (boost::posix_time::time_duration) в строку */
template<class Char>
std::size_t put(Char *buf, std::size_t buf_sz,
	const posix_time::time_duration &dur)
{
	if (dur.is_special())
		return put(buf, buf_sz, my::time::as_special(dur));
	
	Char *ptr = buf;
	Char *end = buf + buf_sz;

	long long ticks = dur.ticks();

	if (ticks < 0)
	{
		ptr += my::str::put(ptr, end - ptr, Char('-'));
		ticks = -ticks;
	}
		
	long long total_seconds = ticks / dur.ticks_per_second();
	long long hours = total_seconds / 3600;

	long fseconds = static_cast<long>(ticks
		- total_seconds * dur.ticks_per_second());
	long seconds = static_cast<long>(total_seconds - hours * 3600);
	long minutes = seconds / 60;
	seconds -= minutes * 60;
		
	ptr += my::num::put(ptr, end - ptr, hours, 2);
	ptr += my::str::put(ptr, end - ptr, Char(':'));
	ptr += my::num::put(ptr, end - ptr, minutes, 2);
	ptr += my::str::put(ptr, end - ptr, Char(':'));
	ptr += my::num::put(ptr, end - ptr, seconds, 2);

	if (fseconds)
	{
		ptr += my::str::put(ptr, end - ptr, Char('.'));
		ptr += my::num::put(ptr, end - ptr, fseconds,
			posix_time::time_duration::num_fractional_digits());
	}

	return ptr - buf;
}

template<class Char,class Time>
inline std::basic_string<Char> to_str(const Time &t)
{
	Char buf[30]; /* 2010-06-10 16:02:14.123456[789] - 26/29 */
	my::time::put(buf, sizeof(buf) / sizeof(*buf), t);
	return std::basic_string<Char>(buf);
}

template<class Time>
inline std::string to_string(const Time &time)
	{ return my::time::to_str<char>(time); }

template<class Time>
inline std::wstring to_wstring(const Time &time)
	{ return my::time::to_str<wchar_t>(time); }


/* Преобразование строки в дату (boost::gregorian::date) */
template<class Char>
std::size_t to_date_s(const Char *str, gregorian::date &date,
	std::size_t size = -1)
{
	static const Char sep_list[] = { '-','.','/',' ',',', 0 };

	unsigned short y(0), m(0), d(0);
	const Char *ptr = str;
	const Char *end = (std::size_t)-1 ? my::str::end(str) : str + size;
	Char sep;

	date = gregorian::date(gregorian::not_a_date_time);

	std::size_t n;

	ptr += (n = my::num::to_ushort(ptr, y, end - ptr));

	if (n != 0 && ptr != end &&
		(sep = *std::find(sep_list, sep_list
			+ sizeof(sep_list) / sizeof(*sep_list) - 1, *ptr)) != 0)
	{	
		if (++ptr != end)
		{
			ptr += (n = my::num::to_ushort(ptr, m, end - ptr));

			if (n != 0 && ptr != end && *ptr == sep)
			{	
				if (++ptr != end)
				{
					ptr += (n = my::num::to_ushort(ptr, d, end - ptr));

					if (y < d)
					{
						unsigned short tmp = d;
						d = y;
						y = tmp;
					}

					try { date = gregorian::date(y, m, d); }
					catch(...) {}
				}
			}
		}
	}

	return ptr - str;
}

template<class Char>
inline std::size_t to_date_s(const std::basic_string<Char> &str,
	gregorian::date &date)
{
	return to_date_s(str.c_str(), date, str.size());
}

template<class Char>
inline gregorian::date to_date(const Char *str, std::size_t size = -1)
{
	gregorian::date date;
	
	if (size == (std::size_t)-1)
		size = my::str::length(str);

	if (size != to_date_s(str, date, size))
		date = gregorian::date(gregorian::not_a_date_time);

	return date;
}

template<class Char>
inline gregorian::date to_date(const std::basic_string<Char> &str)
{
	return to_date(str.c_str(), str.size());
}


/* Преобразование строки в дату/время (boost::posix_time::ptime) */
template<class Char>
std::size_t to_time_s(const Char *str, posix_time::ptime &time,
	std::size_t size = -1)
{
	const Char *ptr = str;
	const Char *end = (std::size_t)-1 ? my::str::end(str) : str + size;

	time = posix_time::ptime(posix_time::not_a_date_time);

	gregorian::date date;
	ptr += to_date_s(str, date, end - ptr);

	if (!date.is_special() && ptr != end && *ptr == ' ' && ++ptr != end)
	{
		posix_time::time_duration dur;
		ptr += to_duration_s(ptr, dur, end - ptr);

		if (!dur.is_special() && !dur.is_negative() && dur.hours() < 24)
			time = posix_time::ptime(date, dur);
	}

	return ptr - str;
}

template<class Char>
inline std::size_t to_time_s(const std::basic_string<Char> &str,
	posix_time::ptime &time)
{
	return to_time_s(str.c_str(), time, str.size());
}

template<class Char>
inline posix_time::ptime to_time(const Char *str,
	std::size_t size = -1)
{
	posix_time::ptime time;
	
	if (size == (std::size_t)-1)
		size = my::str::length(str);

	if (size != to_time_s(str, time, size))
		time = posix_time::ptime(posix_time::not_a_date_time);

	return time;
}

template<class Char>
inline posix_time::ptime to_time(const std::basic_string<Char> &str)
{
	return to_time(str.c_str(), str.size());
}


/* Преобразование строки в длительность (boost::posix_time::time_duration) */
template<class Char>
std::size_t to_duration_s(const Char *str, posix_time::time_duration &dur,
	std::size_t size = -1)
{
	unsigned long h(0), m(0), s(0), f(0);
	const Char *ptr = str;
	const Char *end = (std::size_t)-1 ? my::str::end(str) : str + size;
	bool negative = false;

	dur = posix_time::time_duration(gregorian::not_a_date_time);

	if (ptr != end && *ptr == '-')
	{
		ptr++;
		negative = true;
	}

	std::size_t n;

	/* Часы */
	ptr += (n = my::num::to_ulong(ptr, h, end - ptr));

	if (n != 0 && ptr != end && *ptr == ':' && ++ptr != end)
	{	
		/* Минуты */
		ptr += (n = my::num::to_ulong(ptr, m, end - ptr));

		if (n != 0 && m < 60 && ptr != end && *ptr == ':' && ++ptr != end)
		{
			/* Секунды */
			ptr += (n = my::num::to_ulong(ptr, s, end - ptr));

			if (n != 0 && s < 60)
			{
				/* Доли секунд (могут отсутствовать) */
				if (ptr != end && *ptr == '.' && ++ptr != end)
				{
					ptr += (n = my::num::to_ulong(ptr, f, end - ptr));

					/* Но если есть точка, должно быть и число */
					if (n == 0)
						return ptr - str;
				}

				try {
					
					dur = posix_time::time_duration(h, m, s, f);

					if (negative)
						dur = -dur;
				}
				catch(...) {}
			}
		}
	}

	return ptr - str;
}

template<class Char>
inline std::size_t to_duration_s(const std::basic_string<Char> &str,
	posix_time::time_duration &dur)
{
	return to_duration_s(str.c_str(), dur, str.size());
}

template<class Char>
inline posix_time::time_duration to_duration(const Char *str,
	std::size_t size = -1)
{
	posix_time::time_duration dur;
	
	if (size == (std::size_t)-1)
		size = my::str::length(str);

	if (size != to_duration_s(str, dur, size))
		dur = posix_time::time_duration(posix_time::not_a_date_time);

	return dur;
}

template<class Char>
inline posix_time::time_duration to_duration(const std::basic_string<Char> &str)
{
	return to_duration(str.c_str(), str.size());
}


}}

#endif
