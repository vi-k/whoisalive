#ifndef HOST_STATE_H
#define HOST_STATE_H

#include "../common/my_time.h"

#include <string>
#include <sstream>
#include <ostream>
#include <istream>
#include <boost/io/ios_state.hpp>

#define HOST_STATE_VER 1

namespace pinger {

/* Состояние хоста */
class host_state
{
public:
	enum state_t {unknown=0, ok, warn, fail};

private:
	enum ack_t {ack, unack};

	state_t state_;
	posix_time::ptime time_;
	ack_t acknowledged_;

public:
	host_state()
		: state_(unknown)
		, acknowledged_(ack) {}

	template<class Char>
	host_state(const std::basic_string<Char> &str)
	{
		std::basic_istringstream<Char> in(str);
		in >> *this;
		if (!in)
			*this = host_state();
	}

	template<class Char>
	inline std::basic_string<Char> to_str() const
	{
		std::basic_ostringstream<Char> out;
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

	inline bool acknowledged() const
		{ return acknowledged_ == ack ? true : false; }
	inline void set_acknowledged(bool acknowledged)
		{ acknowledged_ = acknowledged ? ack : unack; }

	template<class Char>
	std::basic_string<Char> brief() const
	{
		static const Char time_fmt[] =
		{
			'%', 'Y', '-', '%', 'm', '-', '%', 'd', ' ',
			'%', 'H', ':', '%', 'M', ':', '%', 'S', '\0'
		};

		std::basic_ostringstream<Char> out;

		out << state_
			<< ' ' << acknowledged_
			<< ' ' << my::time::to_str<Char>(time_, time_fmt);
		return out.str();
	}

	inline bool operator==(state_t st) const
		{ return state_ == st; }

	inline bool operator!=(state_t st) const
		{ return state_ != st; }

	static inline bool eq(const host_state &first,
		const host_state &second)
	{
		return first.state_ == second.state_
			&& first.acknowledged_ == second.acknowledged_
			&& first.state_ != unknown;
	}

	template<class Char>
	friend std::basic_ostream<Char>& operator<<(
		std::basic_ostream<Char>& out, state_t st)
	{
		static const Char ok_s[] = { 'o', 'k', 0 };
		static const Char warn_s[] = { 'w', 'a', 'r', 'n', 0 };
		static const Char fail_s[] = { 'f', 'a', 'i', 'l', 0};
		static const Char unknown_s[] = { 'u', 'n', 'k', 'n', 'o', 'w', 'n', 0};

		out << (st == ok ? ok_s
			: st == warn ? warn_s
			: st == fail ? fail_s
			: unknown_s);

		return out;
	}

	template<class Char>
	friend std::basic_ostream<Char>& operator<<(
		std::basic_ostream<Char>& out, ack_t ac)
	{
		static const Char ack_s[] = { 'a', 'c', 'k', 0 };
		static const Char unack_s[] = { 'u', 'n', 'a', 'c', 'k', 0 };
		
		out << (ac == ack ? ack_s : unack_s);

		return out;
	}

	template<class Char>
	friend std::basic_istream<Char>& operator>>(
		std::basic_istream<Char>& in, state_t &st)
	{
		static const Char ok_s[] = { 'o', 'k', 0 };
		static const Char warn_s[] = { 'w', 'a', 'r', 'n', 0 };
		static const Char fail_s[] = { 'f', 'a', 'i', 'l', 0};
		static const Char unknown_s[] = { 'u', 'n', 'k', 'n', 'o', 'w', 'n', 0};

		basic_string<Char> word;
		in >> word;

		if (word.compare(ok_s) == 0)
			st = ok;
		else if (word.compare(warn_s) == 0)
			st = warn;
		else if (word.compare(fail_s) == 0)
			st = fail;
		else if (word.compare(unknown_s) == 0)
			st = unknown;
		else
			in.setstate(std::ios::failbit);

		return in;
	}

	template<class Char>
	friend std::basic_istream<Char>& operator>>(
		std::basic_istream<Char>& in, ack_t &ac)
	{
		static const Char ack_s[] = { 'a', 'c', 'k', 0 };
		static const Char unack_s[] = { 'u', 'n', 'a', 'c', 'k', 0 };

		basic_string<Char> word;
		in >> word;

		if (word.compare(ack_s) == 0)
			ac = ack;
		else if (word.compare(unack_s) == 0)
			ac = unack;
		else
			in.setstate(std::ios::failbit);

		return in;
	}

	template<class Char>
	friend std::basic_ostream<Char>& operator<<(
		std::basic_ostream<Char>& out, const host_state &hs)
	{
		out << HOST_STATE_VER
			<< ' ' << hs.state_
			<< ' ' << hs.acknowledged_
			<< ' ' << my::time::to_str<Char>(hs.time_);

		return out;
	}

	template<class Char>
	friend std::basic_istream<Char>& operator>>(
		std::basic_istream<Char>& in, host_state &hs)
	{
		basic_string<Char> t1, t2;

		int ver = 0;

		in >> ver
			>> hs.state_
			>> hs.acknowledged_
			>> t1 >> t2;

		hs.time_ = my::time::to_time(t1 + Char(' ') + t2);

		if (ver != HOST_STATE_VER)
			in.setstate(std::ios::failbit);

		return in;
	}

};

}

#endif
