#ifndef PING_RESULT_H
#define PING_RESULT_H

#include "../common/my_time.h"
#include "../common/my_http.h"
#include "../common/my_str.h"

#include "icmp_header.hpp"
#include "ipv4_header.hpp"

#include "string.h"

#include <string>
#include <sstream>
#include <ostream>
#include <istream>
#include <boost/io/ios_state.hpp>

#define PING_RESULT_VER 1

namespace pinger {

/* Результат отдельного ping'а */
class ping_result
{
public:
	enum state_t {unknown, ok, timeout};

private:
	state_t state_;
	posix_time::ptime time_;
	posix_time::time_duration duration_;
	ipv4_header ipv4_hdr_;
	icmp_header icmp_hdr_;

public:
	ping_result()
		: state_(unknown) {}

	template<class Char>
	ping_result(const std::basic_string<Char> &str)
	{
		std::basic_stringstream<Char> in(str);
		in >> *this;
		if (!in)
			*this = ping_result();
	}

	template<class Char>
	inline std::basic_string<Char> to_str() const
	{
		std::basic_stringstream<Char> out;
		out << *this;
		return out.str();
	}

	inline std::string to_string() const
		{ return to_str<char>(); }

	inline std::wstring to_wstring() const
		{ return to_str<wchar_t>(); }


	inline state_t state() const
		{ return state_; }
	inline void set_state(state_t st)
		{ state_ = st; }

	inline posix_time::ptime time() const
		{ return time_; }
	inline void set_time(posix_time::ptime t)
		{ time_ = t; }

	inline posix_time::time_duration duration() const
		{ return duration_; }
	inline void set_duration(posix_time::time_duration d)
		{ duration_ = d; }

	inline unsigned short sequence_number() const
		{ return icmp_hdr_.sequence_number(); }
	inline void set_sequence_number(unsigned short n)
		{ icmp_hdr_.sequence_number(n); }

	inline const ipv4_header& ipv4_hdr() const
		{ return ipv4_hdr_; }
	inline void set_ipv4_hdr(const ipv4_header &ipv4_hdr)
		{ ipv4_hdr_ = ipv4_hdr; }

	inline const icmp_header& icmp_hdr() const
		{ return icmp_hdr_; }
	inline void set_icmp_hdr(const icmp_header &icmp_hdr)
		{ icmp_hdr_ = icmp_hdr; }

	template<class Char>
	std::basic_string<Char> info() const
	{
		static const Char time_fmt[] =
		{
			'%', 'Y', '-', '%', 'm', '-', '%', 'd', ' ',
			'%', 'H', ':', '%', 'M', ':', '%', 'S', '\0'
		};

		std::basic_stringstream<Char> out;
	
		my::time::set_output_format(out, time_fmt);
		
		out << sequence_number()
			<< ' ' << state_
			<< ' ' << time_
			<< ' ' << duration_.total_milliseconds() << 'm' << 's';
		return out.str();
	}

	inline bool operator==(state_t st) const
		{ return state_ == st; }

	inline bool operator!=(state_t st) const
		{ return state_ != st; }

	template<class Char>
	friend std::basic_ostream<Char>& operator<<(
		std::basic_ostream<Char>& out, state_t st)
	{
		static const Char ok_s[] = { 'o', 'k', 0 };
		static const Char timeout_s[] = { 't', 'i', 'm', 'e', 'o', 'u', 't', 0};
		static const Char unknown_s[] = { 'u', 'n', 'k', 'n', 'o', 'w', 'n', 0};

		out << (st == ok ? ok_s
			: st == timeout ? timeout_s : unknown_s);

		return out;
	}

	template<class Char>
	friend std::basic_istream<Char>& operator>>(
		std::basic_istream<Char>& in, state_t &st)
	{
		static const Char ok_s[] = { 'o', 'k', 0 };
		static const Char timeout_s[] = { 't', 'i', 'm', 'e', 'o', 'u', 't', 0};
		static const Char unknown_s[] = { 'u', 'n', 'k', 'n', 'o', 'w', 'n', 0};

		basic_string<Char> word;
		in >> word;

		if (word.compare(ok_s) == 0)
			st = ok;
		else if (word.compare(timeout_s) == 0)
			st = timeout;
		else if (word.compare(unknown_s) == 0)
			st = unknown;
		else
			in.setstate(std::ios::failbit);

		return in;
	}

	template<class Char>
	friend std::basic_ostream<Char>& operator<<(
		std::basic_ostream<Char>& out, const ping_result &pr)
	{
		boost::io::basic_ios_all_saver<Char> ios_saver(out);
		
		my::time::set_mydef_output_format(out);

		out << PING_RESULT_VER
			<< ' ' << pr.state_
			<< ' ' << pr.time_
			<< ' ' << pr.duration_
			<< ' ' << my::str::to_hex( (const char*)pr.ipv4_hdr_.rep_,
				sizeof(pr.ipv4_hdr_.rep_) ).c_str()
			<< ' ' << my::str::to_hex( (const char*)pr.icmp_hdr_.rep_,
				sizeof(pr.icmp_hdr_.rep_) ).c_str();

		return out;
	}

	template<class Char>
	friend std::basic_istream<Char>& operator>>(
		std::basic_istream<Char>& in, ping_result &pr)
	{
		boost::io::basic_ios_all_saver<Char> ios_saver(in);

		my::time::set_mydef_input_format(in);

		int ver = 0;
		in >> ver;
		if (ver != PING_RESULT_VER)
			in.setstate(std::ios::failbit);

		in >> pr.state_
			>> pr.time_
			>> pr.duration_;

		std::basic_string<Char> ipv4_s;
		in >> ipv4_s;
		std::string ipv4_s2 = my::str::from_hex(ipv4_s);
		if (ipv4_s2.size() != sizeof(pr.ipv4_hdr_.rep_))
			in.setstate(std::ios::failbit);
		else
			memcpy(pr.ipv4_hdr_.rep_, ipv4_s2.c_str(), sizeof(pr.ipv4_hdr_.rep_));

		std::basic_string<Char> icmp_s;
		in >> icmp_s;
		std::string icmp_s2( my::str::from_hex(icmp_s) );
		if (icmp_s2.size() != sizeof(pr.icmp_hdr_.rep_))
			in.setstate(std::ios::failbit);
		else
			memcpy(pr.icmp_hdr_.rep_, icmp_s2.c_str(), sizeof(pr.icmp_hdr_.rep_));

		return in;
	}

};

}

#endif
