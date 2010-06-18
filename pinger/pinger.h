#ifndef PINGER_H
#define PINGER_H

/*
	Код создан на основе примера для boost.asio:

	ping.cpp, ipv4_header.hpp, icmp_header.hpp
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	
	Copyright (c) 2003-2010 Christopher M. Kohlhoff (chris at kohlhoff dot com)
	
	Distributed under the Boost Software License, Version 1.0. (See accompanying
	file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
*/

#include "ping_result.h"
#include "host_state.h"

#include "../common/my_thread.h"
#include "../common/my_inet.h"
#include "../common/my_time.h"
#include "../common/my_xml.h"
#include "../common/my_mru.h"

#include <vector>

#include <boost/bind.hpp>
#include <boost/ptr_container/ptr_list.hpp>
#include <boost/circular_buffer.hpp>

#include "icmp_header.hpp"
#include "ipv4_header.hpp"

namespace pinger {

inline posix_time::ptime now();
inline unsigned short get_id();

/* Пингер отдельного хоста */
class server;
class host_pinger
{
	friend class host_pinger_copy;

	typedef boost::circular_buffer<host_state> states_list;
	typedef my::mru::list<unsigned short, ping_result> results_list;

private:
	server &parent_;
	std::wstring hostname_; /* Имя хоста */
	icmp::endpoint endpoint_; /* ip-адрес */
	states_list states_; /* Состояния адреса */
	int fails_; /* Кол-во таймаутов подряд */
	results_list results_; /* Результаты пингов */
	unsigned short sequence_number_; /* Номер последнего пинга */
	posix_time::ptime last_ping_time_; /* Время отправки последнего пинга */
	icmp::socket &socket_;
	asio::deadline_timer timer_;
	posix_time::time_duration timeout_; /* Время ожидания ответа */
	posix_time::time_duration request_period_; /* Период опроса */
	recursive_mutex pinger_mutex_; /* Блокировка всего пингера */

	void handle_timeout_(unsigned short sequence_number);
	
public:
	host_pinger(server &parent,
		asio::io_service &io_service /* Для таймера */,
		const std::wstring &hostname,
		icmp::endpoint endpoint,
		icmp::socket &socket,
		posix_time::time_duration timeout,
		posix_time::time_duration request_period,
		unsigned short max_states = 100,
		unsigned short max_results = 100);

	void run();

	void on_receive(posix_time::ptime time,
		const ipv4_header &ipv4_hdr, const icmp_header &icmp_hdr);

	
	/* Чтение параметров пингера */
	ip::address_v4 address()
	{
		unique_lock<recursive_mutex> l(pinger_mutex_); /* Блокируем пингер */
		return endpoint_.address().to_v4();
	}

	class host_pinger_copy copy();

	void acknowledge(bool ack);

	host_state last_state()
	{
		unique_lock<recursive_mutex> l(pinger_mutex_);
		return states_.empty() ? host_state() : states_.front();
	}

	void states_copy(std::vector<host_state> &v);

	ping_result last_result()
	{
		unique_lock<recursive_mutex> l(pinger_mutex_);
		return results_.empty() ? ping_result() : results_.front().value();
	}
	
	void results_copy(std::vector<ping_result> &v);

	/* Изменение параметров пингера */
	void set_request_period(posix_time::time_duration request_period)
	{
		unique_lock<recursive_mutex> l(pinger_mutex_);
		request_period_ = request_period;
	}

	void set_timeout(posix_time::time_duration timeout)
	{
		unique_lock<recursive_mutex> l(pinger_mutex_);
		timeout_ = timeout;
	}
};

/* Структура для копирования и передачи сведений о host_pinger'е */
class host_pinger_copy
{
public:
	std::wstring hostname;
	ip::address_v4 address;
	int fails;
	posix_time::time_duration timeout;
	posix_time::time_duration request_period;

	host_pinger_copy(host_pinger &pinger)
		: hostname(pinger.hostname_)
		, address(pinger.endpoint_.address().to_v4())
		, fails(pinger.fails_)
		, timeout(pinger.timeout_)
		, request_period(pinger.request_period_) {}
};

/* Пингер-сервер */
class server
{
private:
	asio::io_service io_service_;
	icmp::resolver resolver_;
	boost::ptr_list<host_pinger> pingers_;
	icmp::socket socket_;
	asio::streambuf reply_buffer_;
	posix_time::time_duration def_timeout_;
	posix_time::time_duration def_request_period_;
	recursive_mutex server_mutex_;

	void receive_();
	void handle_receive_(std::size_t length);
	host_pinger* find_pinger_(const ip::address_v4 &address,
		bool throw_if_not_found = false);

public:
	server();

	boost::function<void (const host_pinger_copy &pinger,
		const host_state &state)> on_change_state;
	boost::function<void (const host_pinger_copy &pinger,
		const ping_result &result)> on_ping;

	void run();

	/* Разобрать настройки */
	void match(xml::wptree &pt);

	icmp::endpoint resolve(const std::string &hostname);
	icmp::endpoint resolve(const std::wstring &hostname);

	bool add_pinger(const std::wstring &hostname,
		posix_time::time_duration timeout = posix_time::not_a_date_time,
		posix_time::time_duration request_period = posix_time::not_a_date_time);

	inline void change_state_notify(host_pinger &pinger,
		const host_state &state)
	{
		/*TODO: Сохранение результата в БД */
		if (on_change_state)
			on_change_state(pinger.copy(), state);
	}

	inline void ping_notify(host_pinger &pinger,
		const ping_result &result)
	{
		if (on_ping)
			on_ping(pinger.copy(), result);
	}

	void pingers_copy(std::vector<host_pinger_copy> &v);
	
	host_pinger_copy pinger_copy(const ip::address_v4 &address)
	{
		unique_lock<recursive_mutex> l(server_mutex_); /* Блокируем сервер */
		host_pinger *pinger = find_pinger_(address, true);
		return host_pinger_copy(*pinger);
	}

	void acknowledge(const ip::address_v4 &address, bool ack)
	{
		unique_lock<recursive_mutex> l(server_mutex_); /* Блокируем сервер */
		host_pinger *pinger = find_pinger_(address, true);
		if (pinger)
			pinger->acknowledge(ack);
	}

	host_state last_state(const ip::address_v4 &address)
	{
		unique_lock<recursive_mutex> l(server_mutex_); /* Блокируем сервер */
		host_pinger *pinger = find_pinger_(address, true);
		return pinger->last_state();
	}

	void states_copy(const ip::address_v4 &address,
		std::vector<host_state> &v)
	{
		unique_lock<recursive_mutex> l(server_mutex_); /* Блокируем сервер */
		host_pinger *pinger = find_pinger_(address, true);
		pinger->states_copy(v);
	}

	ping_result last_result(const ip::address_v4 &address)
	{
		unique_lock<recursive_mutex> l(server_mutex_); /* Блокируем сервер */
		host_pinger *pinger = find_pinger_(address, true);
		return pinger->last_result();
	}

	void results_copy(const ip::address_v4 &address,
		std::vector<ping_result> &v)
	{
		unique_lock<recursive_mutex> l(server_mutex_); /* Блокируем сервер */
		host_pinger *pinger = find_pinger_(address, true);
		pinger->results_copy(v);
	}
	
	/* Чтение параметров сервера */
	
	inline posix_time::time_duration def_timeout()
	{
		unique_lock<recursive_mutex> l(server_mutex_); /* Блокируем сервер */
		return def_timeout_;
	}

	inline posix_time::time_duration def_request_period()
	{
		unique_lock<recursive_mutex> l(server_mutex_); /* Блокируем сервер */
		return def_request_period_;
	}

	
	/* Изменение параметров сервера */

	void set_def_timeout(posix_time::time_duration time)
	{
		my::time::throw_if_fail(time);
		unique_lock<recursive_mutex> l(server_mutex_); /* Блокируем сервер */
		def_timeout_ = time;
	}

	void set_def_request_period(posix_time::time_duration time)
	{
		my::time::throw_if_fail(time);
		unique_lock<recursive_mutex> l(server_mutex_); /* Блокируем сервер */
		def_request_period_ = time;
	}


	/* Чтение параметров пингеров */
#if 0
	posix_time::time_duration timeout(const ip::address_v4 &address)
	{
		unique_lock<recursive_mutex> l(server_mutex_); /* Блокируем сервер */
		host_pinger *pinger = find_pinger_(address, true /*throw_if_not_found*/);
		return pinger->timeout();
	}

	posix_time::time_duration request_period(const ip::address_v4 &address)
	{
		unique_lock<recursive_mutex> l(server_mutex_); /* Блокируем сервер */
		host_pinger *pinger = find_pinger_(address, true /*throw_if_not_found*/);
		return pinger->request_period();
	}
#endif

	/* Изменение параметров пингеров */

	void set_timeout(const ip::address_v4 &address,
		posix_time::time_duration time)
	{
		my::time::throw_if_fail(time);
		unique_lock<recursive_mutex> l(server_mutex_); /* Блокируем сервер */
		host_pinger *pinger = find_pinger_(address, true /*throw_if_not_found*/);
		pinger->set_timeout(time);
	}

	void set_request_period(const ip::address_v4 &address,
		posix_time::time_duration time)
	{
		my::time::throw_if_fail(time);
		unique_lock<recursive_mutex> l(server_mutex_); /* Блокируем сервер */
		host_pinger *pinger = find_pinger_(address, true /*throw_if_not_found*/);
		pinger->set_request_period(time);
	}
};

}

#endif
