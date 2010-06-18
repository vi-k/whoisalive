/*
	Код сделан на основе примера для boost.asio:

	ping.cpp
	~~~~~~~~
	
	Copyright (c) 2003-2010 Christopher M. Kohlhoff (chris at kohlhoff dot com)
	
	Distributed under the Boost Software License, Version 1.0.
	(http://www.boost.org/LICENSE_1_0.txt)
*/

#include "pinger.h"
using namespace pinger;

#include "../common/my_time.h"
#include "../common/my_xml.h"
#include "../common/my_utf8.h"
#include "../common/my_http.h"
#include "../common/my_exception.h"

#include <istream>
#include <iostream>
#include <ostream>
#include <utility> /* std::pair */
using namespace std;

#include <boost/foreach.hpp>

posix_time::ptime pinger::now()
{
	return posix_time::microsec_clock::universal_time();
}

unsigned short pinger::get_id()
{
#if defined(BOOST_WINDOWS)
	return static_cast<unsigned short>(::GetCurrentProcessId());
#else
	return static_cast<unsigned short>(::getpid());
#endif
}

host_pinger::host_pinger(server &parent,
	asio::io_service &io_service,
	const std::wstring &hostname,
	icmp::endpoint endpoint,
	icmp::socket &socket,
	posix_time::time_duration timeout,
	posix_time::time_duration request_period,
	unsigned short max_states,
	unsigned short max_results)
		: parent_(parent)
		, socket_(socket)
		, hostname_(hostname)
		, endpoint_(endpoint)
		, states_(max_states)
		, fails_(0)
		, results_(max_results)
		, sequence_number_(0)
		, timeout_(timeout)
		, request_period_(request_period)
		, timer_(io_service)
{
}

host_pinger_copy host_pinger::copy()
{
	unique_lock<recursive_mutex> l(pinger_mutex_); /* Блокируем пингер */
	return host_pinger_copy(*this);
}

void host_pinger::states_copy(vector<host_state> &v)
{
	unique_lock<recursive_mutex> l(pinger_mutex_);

	for (states_list::iterator iter = states_.begin();
		iter != states_.end(); iter++)
	{
		v.push_back(*iter);
	}
}

void host_pinger::results_copy(vector<ping_result> &v)
{
	unique_lock<recursive_mutex> l(pinger_mutex_);

	for (results_list::iterator iter = results_.begin();
		iter != results_.end(); iter++)
	{
		v.push_back(iter->value());
	}
}

void host_pinger::run()
{
	static const string body("'Hello!' from Asio ping.");

	/* Создаём пакет */
	icmp_header echo_request;
	echo_request.type(icmp_header::echo_request);
	echo_request.code(0);
	echo_request.identifier(pinger::get_id());
	echo_request.sequence_number(++sequence_number_);
	compute_checksum(echo_request, body.begin(), body.end());

	asio::streambuf request_buffer;
	ostream os(&request_buffer);
	os << echo_request << body;

	/* Отправляем пакет */	
	last_ping_time_ = now();
	socket_.send_to(request_buffer.data(), endpoint_);

	timer_.expires_at( last_ping_time_ + timeout_ );
	timer_.async_wait( boost::bind(&pinger::host_pinger::handle_timeout_,
		this, sequence_number_) );
}

void host_pinger::acknowledge(bool ack)
{
	/* Сразу вычисляем время - избегаем лишних погрешностей */
	posix_time::ptime time = now();

	host_state prev_state;
	host_state new_state;
	bool changed = false;

	/* Блокируем пингер */
	{
		unique_lock<recursive_mutex> l(pinger_mutex_);

		if (!states_.empty())
			prev_state = states_.front();

		new_state = prev_state;
		new_state.set_time(time);
		new_state.set_acknowledged(ack);

		if ( !host_state::eq(new_state, prev_state) )
		{
			changed = true;
			states_.push_front(new_state);
		}
	}

	/* Мы имеем копии результата и состояния, поэтому
		дальнейшая блокировка не требуется */

	/* Оповещаем об изменении состояния */
	if (changed)
		parent_.change_state_notify(*this, new_state);
}

void host_pinger::handle_timeout_(unsigned short sequence_number)
{
	/* Сразу вычисляем время - избегаем лишних погрешностей */
	posix_time::ptime time = now();

	/* Результат пинга */
	ping_result result;
	result.set_sequence_number(sequence_number);
	result.set_state(ping_result::timeout);
	result.set_time(last_ping_time_);
	result.set_duration(time - last_ping_time_);

	/* Cостояние хоста */
	host_state prev_state;
	host_state new_state;
	bool state_changed = false;

	/* Блокируем пингер */
	{
		unique_lock<recursive_mutex> l(pinger_mutex_);

		/* Вполне может случиться так, что сначала прийдёт отклик,
			но за время его обработки сработает таймаут и функция запустится.
			В этом случае ничего не делаем: результат ping'а уже в списке,
			таймер уже установлен */
		results_list::iterator iter = results_.find(sequence_number);
		if (iter != results_.end())
			return;

		/* Сохраняем результат (отрицательный результат - тоже результат */
		results_[sequence_number] = result;

		if (!states_.empty())
			prev_state = states_.front();

		new_state = prev_state;
		new_state.set_time(time);

		fails_++;

		/* Первые 4 таймаута - warn. Далее - fail */
		if (fails_ == 1)
			new_state.set_state(host_state::warn);
		else if (fails_ == 5)
		{
			new_state.set_state(host_state::fail);
			new_state.set_acknowledged(false);
		}

		if ( !host_state::eq(new_state, prev_state) )
		{
			state_changed = true;
			states_.push_front(new_state);
		}
	}

	/* Мы имеем копии результата и состояния, поэтому
		дальнейшая блокировка не требуется */

	if (fails_ <= 4)
		/* После первого таймаута сразу делаем 3 дополнительных
			запроса, чтоб7ы удостовериться, что это сбой */
		timer_.expires_at(last_ping_time_ + timeout_ + posix_time::seconds(1));
	else
		/* После них торопиться уже некуда */
		timer_.expires_at(last_ping_time_ + request_period_);

	timer_.async_wait( boost::bind(&host_pinger::run, this) );

	/* Оповещаем о таймауте */
	parent_.ping_notify(*this, result);

	/* Оповещаем об изменении состояния */
	if (state_changed)
		parent_.change_state_notify(*this, new_state);
}

void host_pinger::on_receive(posix_time::ptime time,
	const ipv4_header &ipv4_hdr, const icmp_header &icmp_hdr)
{
	/* Результат пинга */
	ping_result result;
	result.set_state(ping_result::ok);
	result.set_ipv4_hdr(ipv4_hdr);
	result.set_icmp_hdr(icmp_hdr);

	/* Cостояние хоста */
	host_state prev_state;
	host_state new_state;
	bool state_changed = false;

	/* Блокируем пингер */
	{
		unique_lock<recursive_mutex> l(pinger_mutex_);

		/* При получении отклика, результат уже может быть в списке,
			если таймаут уже сработал. В этом случае исправляем старый
			результат на новый */
		unsigned short n = result.sequence_number();

		results_list::iterator iter = results_.find(n);

		if (iter != results_.end())
		{
			result.set_time( iter->value().time() );
			result.set_duration( time - result.time() );
		}
		else
		{
			result.set_time(last_ping_time_);
			result.set_duration(time - last_ping_time_);
		}

		/* Сохраняем результат */
		results_[n] = result;

		if (!states_.empty())
			prev_state = states_.front();

		new_state = prev_state;

		/* Может прийти ping из прошлого. Не обращаем на него внимания */
		if (sequence_number_ == result.sequence_number())
		{
			fails_ = 0;
			new_state.set_state(host_state::ok);
			new_state.set_time(time);
		}

		if ( !host_state::eq(new_state, prev_state) )
		{
			state_changed = true;
			states_.push_front(new_state);
		}

		if (iter == results_.end())
		{
			timer_.cancel();
			timer_.expires_at( last_ping_time_ + request_period_ );
			timer_.async_wait( boost::bind(&host_pinger::run, this) );
		}
	}

	/* Мы имеем копии результата и состояния, поэтому
		дальнейшая блокировка не требуется */

	/* Оповещаем об ответе хоста */
	parent_.ping_notify(*this, result);

	/* Оповещаем об изменении состояния */
	if (state_changed)
		parent_.change_state_notify(*this, new_state);
}

server::server()
	: io_service_()
	, resolver_(io_service_)
	, socket_(io_service_, icmp::v4())
	, def_timeout_(0,0,1)
	, def_request_period_(0,0,5)
{
}

void server::run()
{
	receive_();
	io_service_.run();
}

void server::match(xml::wptree &pt)
{
	try
	{
		pair<xml::wptree::const_assoc_iterator,
			xml::wptree::const_assoc_iterator> p
				= pt.equal_range(L"host");

		while (p.first != p.second)
		{
			const xml::wptree &node = p.first->second;

			/* Адрес - обязательно! */
			wstring hostname = node.get_value<wstring>();

			posix_time::time_duration timeout
				= my::time::to_duration(
					node.get<wstring>(L"<xmlattr>.timeout", L"") );
		
			posix_time::time_duration request_period
				= my::time::to_duration(
					node.get<wstring>(L"<xmlattr>.request_period", L"") );

			add_pinger(hostname, timeout, request_period);

			p.first++;
		}
	}
	catch(exception &e)
	{
		throw my::exception(e);
	}
}

icmp::endpoint server::resolve(const std::string &hostname)
{
	icmp::endpoint endpoint;

	try
	{
		icmp::resolver::query query(icmp::v4(), hostname, "");
		endpoint = *resolver_.resolve(query);
	}
	catch(std::exception &e)
	{
		throw my::exception(e)
			<< my::param(L"hostname", my::ip::punycode_decode(hostname));
	}

	return endpoint;
}

icmp::endpoint server::resolve(const std::wstring &hostname)
{
	icmp::endpoint endpoint;

	try
	{
		icmp::resolver::query query(icmp::v4(), my::ip::punycode_encode(hostname), "");
		endpoint = *resolver_.resolve(query);
	}
	catch(std::exception &e)
	{
		throw my::exception(e)
			<< my::param(L"hostname", hostname);
	}

	return endpoint;
}

bool server::add_pinger(const std::wstring &hostname,
	posix_time::time_duration timeout,
	posix_time::time_duration request_period)
{
	unique_lock<recursive_mutex> l(server_mutex_); /* Блокируем сервер */

	icmp::endpoint endpoint = resolve(hostname);

	/* Если уже существует - не добавляем */
	if ( find_pinger_( endpoint.address().to_v4() ) )
		return false;

	if (timeout.is_special())
		timeout = def_timeout_;
	
	if (request_period.is_special())
		request_period = def_request_period_;

	host_pinger *pinger = new host_pinger(*this, io_service_,
		hostname, endpoint, socket_, timeout, request_period);

	pingers_.push_back(pinger);

	pinger->run();

	return true;
}

host_pinger* server::find_pinger_(const ip::address_v4 &address,
	bool throw_if_not_found)
{
	unique_lock<recursive_mutex> l(server_mutex_); /* Блокируем сервер */

	BOOST_FOREACH(host_pinger &pinger, pingers_)
		if (pinger.address() == address)
			return &pinger;

	if (throw_if_not_found)
		throw my::exception(L"Адрес не найден")
			<< my::param(L"address", address);

	return NULL;
}

void server::receive_()
{
	/* Discard any data already in the buffer */
	reply_buffer_.consume(reply_buffer_.size());

	/* Wait for a reply. We prepare the buffer to receive up to 64KB */
	socket_.async_receive(reply_buffer_.prepare(65536),
		boost::bind(&server::handle_receive_, this, _2));
}

void server::handle_receive_(size_t length)
{
	/* Сразу вычисляем время - избегаем лишних погрешностей */
	posix_time::ptime time = now();

	/* The actual number of bytes received is committed to the buffer
		so that we can extract it using a istream object */
	reply_buffer_.commit(length);

	/* Decode the reply packet */
	istream is(&reply_buffer_);
	ipv4_header ipv4_hdr;
	icmp_header icmp_hdr;
	is >> ipv4_hdr >> icmp_hdr;

	/* We can receive all ICMP packets received by the host, so we need to
		filter out only the echo replies that match the our identifier */
	if ( is && icmp_hdr.type() == icmp_header::echo_reply
		&& icmp_hdr.identifier() == pinger::get_id() )
	{
		unique_lock<recursive_mutex> l(server_mutex_); /* Блокируем сервер */

		/* Проверяем, что адрес есть в списке */
		host_pinger *host_pinger =
			find_pinger_(ipv4_hdr.source_address());

		if (host_pinger)
			host_pinger->on_receive(time, ipv4_hdr, icmp_hdr);
	}

	receive_();
}

void server::pingers_copy(std::vector<host_pinger_copy> &v)
{
	unique_lock<recursive_mutex> l(server_mutex_); /* Блокируем сервер */

	BOOST_FOREACH(host_pinger &pinger, pingers_)
		v.push_back( pinger.copy() );
}
