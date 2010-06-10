#ifndef MY_STOPWATCH_H
#define MY_STOPWATCH_H

#include <ostream>
#include <string>

#include "my_time.h"

#define MY_SW_COUNT my::stopwatch::show_count
#define MY_SW_TOTAL my::stopwatch::show_total
#define MY_SW_AVG   my::stopwatch::show_avg
#define MY_SW_MIN   my::stopwatch::show_min
#define MY_SW_MAX   my::stopwatch::show_max
#define MY_SW_ALL   my::stopwatch::show_all

namespace my {

class stopwatch
{
public:
	enum {show_count=1, show_total=2, show_avg=4,
		show_min=8, show_max=16, show_all=31};

private:
	posix_time::ptime start_;
	int show_;

public:
	int count;
	posix_time::time_duration total;
	posix_time::time_duration min;
	posix_time::time_duration max;

	stopwatch(int show = show_all)
		: count(0)
		, show_(show) {}

	inline void reset()
	{
		*this = stopwatch(show_);
	}

	inline void restart()
	{
		reset();
		start();
	}

	inline void start()
	{
		start_ = posix_time::microsec_clock::local_time();
	}

	inline void finish()
	{
		posix_time::time_duration time
			= posix_time::microsec_clock::local_time() - start_;

		if (time < min || count == 0)
			min = time;
		if (time > max || count == 0)
			max = time;

		total += time;
		count++;
	}

	inline posix_time::time_duration avg() const
	{
		return count ? (total / count)
			: posix_time::time_duration();
	}

	template<class Char>
	friend std::basic_ostream<Char>& operator <<(std::basic_ostream<Char>& out,
		const stopwatch &sw)
	{
		if (sw.count == 0)
			out << "null";
		else if (sw.count == 1 && (sw.show_ & show_total))
			out << "total=" << sw.total;
		else
		{
			int index = 0;
			if (sw.show_ & show_count)
				out	<< (index++ ? " " : "") << "count=" << sw.count;
			if (sw.show_ & show_total)
				out << (index++ ? " " : "") << "total=" << sw.total;
			if (sw.show_ & show_avg)
				out << (index++ ? " " : "") << "avg=" << sw.avg();
			if (sw.show_ & show_min)
				out << (index++ ? " " : "") << "min=" << sw.min;
			if (sw.show_ & show_max)
				out << (index++ ? " " : "") << "max=" << sw.max;
		}

		return out;
	}	

	template<class Char>
	std::basic_string<Char> to_str()
	{
		basic_ostringstream<Char> out;
		out << *this;
		return out.str();
	}

	inline std::string to_string()
		{ return to_str<char>(); }

	inline std::wstring to_wstring()
		{ return to_str<wchar_t>(); }

};

}

#endif
