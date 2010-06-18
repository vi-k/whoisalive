#include "eventer.h"

#include "../common/my_utf8.h"

#include <string>
#include <sstream>
#include <locale>
#include <exception>
using namespace std;

#include <boost/foreach.hpp>
#include <boost/bind.hpp>

using namespace eventer;

server::server()
{
}

void server::run()
{
	/* Локальный мьютекс - используем только для засыпания/просыпания потока */
	mutex server_mutex;

	while (true)
	{
		unique_lock<mutex> lock(server_mutex);

		/* Копируем события и сразу освобождаем список, чтобы не останавливать
			работу pingera'а пока передаём данные */
		std::list<event> events_copy;

		{
			/* Блокируем список событий на время копирования */
			unique_lock<recursive_mutex> l(events_mutex_);
			events_copy = events_;
			events_.clear();
		}

		/* Ждём, пока не появятся события */
		if (events_copy.empty())
		{
			/* А пока засыпаем */
			cond_.wait(lock);
			continue;
		}

		unique_lock<recursive_mutex> l(handlers_mutex_); /* Нельзя удалять и добавлять обработчики */

		BOOST_FOREACH(handler &h, handlers_)
		{
			wostringstream out;

			BOOST_FOREACH(event &ev, events_copy)
			{
				if (ev.address == h.address
					|| h.address == ip::address_v4::any())
				{
					out << ev.message << L"\r\n";
				}
			}

			/* Для данного обработчика список событий может оказаться пустым */
			if (!out.str().empty())
			try
			{
				h.connection->send( my::utf8::encode(out.str()) );
			}
			catch(...)
			{
				/* Ошибка отправки. Закрываем соединение */
				h.connection->socket().close();
			}
		}

		/* Удаляем закрытые соединения */
		if (!handlers_.empty())
			handlers_.erase_if(handler_failed);
	
	} /* while (true) */
}

void server::start()
{
	cond_.notify_all();
}

void server::add_handler(acceptor::connection *connection,
	ip::address_v4 for_whom)
{
	unique_lock<recursive_mutex> l(handlers_mutex_); /* Блокируем список обработчиков */
	handlers_.push_back( new handler(connection, for_whom) );
	start();
}

void server::add_event(ip::address_v4 who,
	const wstring &what)
{
	unique_lock<recursive_mutex> l(events_mutex_); /* Блокируем список событий */
	events_.push_back( event(who, what) );
	start();
}
