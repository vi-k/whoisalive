﻿#include "stdafx.h"

#include "server.h"

#include "../common/my_inet.h"
#include "../common/my_http.h"
#include "../common/my_utf8.h"
#include "../common/my_fs.h"
#include "../common/my_log.h"
extern my::log main_log;

//#define MY_STOPWATCH_DEBUG
#include "../common/my_debug.h"
MY_STOPWATCH( __load_file_sw1(MY_SW_ALL & ~MY_SW_TOTAL) )
MY_STOPWATCH( __load_file_sw2(MY_SW_ALL & ~MY_SW_TOTAL) )

#include <sstream>
#include <iostream>
#include <map>
#include <utility> /* std::pair */
using namespace std;

#include <boost/config/warning_disable.hpp> /* против unsafe */
#include <boost/bind.hpp>
#include <boost/foreach.hpp>

#pragma warning(disable:4355) /* 'this' : used in base member initializer list */

namespace who {

server::server(const xml::wptree &config)
	: gdiplus_token_(0)
	, io_service_()
	, anim_handlers_counter_(0)
	, anim_period_( posix_time::milliseconds(60) )  /* 20 40 50 100 200 */
	, def_anim_steps_(4)                            /* 10  5  4   2   1 */
	, anim_speed_(0)
	, anim_freq_(0)
	, flash_step_(1)
	, flash_pause_(false)
	, flash_alpha_(0)
	, flash_new_alpha_(0)
	, state_log_socket_(io_service_)
	, active_map_id_(1)
	, tiler_(*this, 1000, boost::bind(&server::on_tiler_update, this))
{
	/* Инициализируем GDI+ */
	Gdiplus::GdiplusStartupInput gs;
	Gdiplus::GdiplusStartup(&gdiplus_token_, &gs, NULL);

	tcp::resolver resolver(io_service_);
	tcp::resolver::query query(
		my::ip::punycode_encode(config.get<wstring>(L"address",L"")),
		my::str::to_string(config.get<wstring>(L"port",L"")) );
	server_endpoint_ = *resolver.resolve(query);

	load_classes_();
	load_maps_();

	/* Поток для приёма журнала состояний хостов */
	boost::thread( boost::bind(
		&server::state_log_thread_proc, this,
		new_worker("state_log_thread", boost::bind(
            &tcp::socket::close, boost::ref(state_log_socket_))) ) );

	/* Поток работы с io_service */
    io_worker_ = new_worker("io_thread");
    boost::thread( boost::bind(
		&server::io_thread_proc, this, io_worker_) );

	/* Анимация */
	
	/* Чтобы скорость и частота анимации не скакали слишком быстро,
		будем хранить 10 последних расчётов и вычислять общее среднее */
	for (int i = 0; i < 10; i++)
	{
		anim_speed_sw_.push();
		anim_freq_sw_.push();
	}
	anim_worker_ = new_worker("anim_thread");
	boost::thread( boost::bind(
		&server::anim_thread_proc, this, anim_worker_) );
}

server::~server()
{
	/* Оповещаем о завершении работы */
	lets_finish();

	/* "Увольняем" все ссылки на "работников" */
	dismiss(io_worker_);
	dismiss(anim_worker_);

    /* Ждём завершения */
   	#if 0
    while (!check_for_finish())
    {
    	vector<std::string> v;
    	workers_state(v);
    	size_t n = v.size();
    }
    #endif

	wait_for_finish();

	//TODO: (ptr_list-объекты будут очищены после, так что здесь нельзя закрывать Gdi+)
	//Gdiplus::GdiplusShutdown(gdiplus_token_);

	windows_.clear();
	classes_.clear();
}

void server::io_thread_proc(my::worker::ptr worker)
{
	while (!finish())
	{
		io_service_.run(); /* Запускаем */
		io_service_.reset();

		/* Если нет задач, засыпаем */
		sleep(worker);
	}
}

void server::anim_thread_proc(my::worker::ptr this_worker)
{
	asio::io_service io_service;
	asio::deadline_timer timer(io_service, my::time::utc_now());
	
	while (!finish())
	{
		anim_speed_sw_.start();

		/* Мигание для "мигающих" объектов */
		flash_alpha_ += (flash_new_alpha_ - flash_alpha_) / flash_step_;
		if (--flash_step_ == 0)
		{
			flash_step_ = def_anim_steps_;
			/* При выходе из паузы, меняем направление мигания */
			if ((flash_pause_ = !flash_pause_) == false)
				flash_new_alpha_ = (flash_new_alpha_ == 0 ? 1 : 0);
		}

		BOOST_FOREACH(window &win, windows_)
			win.anim_handler();

		{
			/* Используем внутренний mutex worker'а для блокировки */
			unique_lock<mutex> lock = this_worker->create_lock();

			for (anim_handlers_list::iterator iter = anim_handlers_.begin();
				iter != anim_handlers_.end(); ++iter)
			{
				iter->second();
			}
		}

		anim_speed_sw_.finish();
		anim_freq_sw_.finish();

		if (anim_speed_sw_.total().total_milliseconds() >= 300)
		{
			anim_speed_sw_.push();
			anim_speed_sw_.pop_back();

			anim_freq_sw_.push();
			anim_freq_sw_.pop_back();

			anim_speed_ = my::time::div(
				anim_speed_sw_.full_avg(), posix_time::milliseconds(1) );
			anim_freq_ = my::time::div(
				anim_freq_sw_.full_avg(), posix_time::milliseconds(1) );
		}

		anim_freq_sw_.start();

		boost::posix_time::ptime time = timer.expires_at() + anim_period_;
		boost::posix_time::ptime now = my::time::utc_now();

		/* Теоретически время следующей прорисовки должно быть относительным
			от времени предыдущей, но на практике могут возникнуть торможения,
			и, тогда, программа будет пытаться запустить прорисовку в прошлом.
			В этом случае следующий запуск делаем относительно текущего времени */ 
		timer.expires_at( now > time ? now : time );
		timer.wait();
	}
}

int server::add_anim_handler(anim_handler handler)
{
	/* Пока жива копия, жив и worker */
	my::worker::ptr anim_worker_copy = anim_worker_;

	if (!anim_worker_copy || finish())
		return 0;
	
	/* Используем внутренний mutex worker'а для блокировки */
	unique_lock<mutex> lock = anim_worker_copy->create_lock();
	
	/* Добавляем */
	anim_handlers_[++anim_handlers_counter_] = handler;
	
	return anim_handlers_counter_;
}

void server::remove_anim_handler(int handler_index)
{
	/* Пока жива копия, жив и worker */
	my::worker::ptr anim_worker_copy = anim_worker_;

	if (!anim_worker_copy || finish())
		return;

	unique_lock<mutex> lock = anim_worker_copy->create_lock();
	anim_handlers_.erase(handler_index);
}

void server::state_log_thread_proc(my::worker::ptr worker)
{
	try
	{
		my::http::reply reply;
		get_header(state_log_socket_, reply, L"/pinger/state.log");

		if (reply.status_code != 200)
			throw my::exception(L"Сервер не вернул состояние хостов")
				<< my::param(L"http-status-code", reply.status_code)
				<< my::param(L"http-status-message", reply.status_message);
	
		while (!finish())
		{
			size_t n = asio::read_until(state_log_socket_, reply.buf_, "\r\n");

			if (finish())
				break;

			reply.body.resize(n);
			reply.buf_.sgetn((char*)reply.body.c_str(), n);

			if (reply.body == "START_ARCHIVE\r\n")
			{
				//
			}
			else if (reply.body == "END_ARCHIVE\r\n")
			{
				//
			}
			else
			{
				wistringstream ss( my::utf8::decode(reply.body) );
				wstring host;
				ss >> host;

				pinger::host_state state;
				ss >> state;

				if (ss)
					hosts_[host] = state;

				check_state_notify();
			}
		}
	}
	catch (my::exception &e)
	{
		if (!finish())
			throw e;
	}
	catch(exception &e)
	{
		if (!finish())
			throw my::exception(e)
				<< my::param(L"where", L"who::server::state_log_thread_proc()");
	}
}

/******************************************************************************
* Загрузка классов
*/
void server::load_classes_()
{
	try
	{
		my::http::reply reply;
		get(reply, L"/classes/classes.xml");

		xml::wptree pt;
		reply.to_xml(pt);
	
		pair<xml::wptree::assoc_iterator,
			xml::wptree::assoc_iterator> p
				= pt.get_child(L"classes").equal_range(L"class");

		/* Создаём классы */
		while (p.first != p.second)
		{
			wstring class_name = p.first->second.get<wstring>(L"<xmlattr>.name");
			try
			{
				classes_[class_name].reset(
					new who::obj_class(*this, p.first->second) );
			}
			catch(my::exception &e)
			{
				throw e << my::param(L"class-name", class_name);
			}

			p.first++;
		}
	}
	catch(my::exception &e)
	{
		throw my::exception(L"Ошибка загрузки классов")
			<< my::param(L"file", L"/classes/classes.xml")
			<< e;
	}
}

void server::load_maps_()
{
	try
	{
		my::http::reply reply;
		get(reply, L"/maps/maps.xml");

		xml::wptree pt;
		reply.to_xml(pt);
	
		pair<xml::wptree::assoc_iterator,
			xml::wptree::assoc_iterator> p
				= pt.get_child(L"maps").equal_range(L"map");

		while (p.first != p.second)
		{
			tiler::map map;
			map.id = p.first->second.get<wstring>(L"id");
			map.name = p.first->second.get<wstring>(L"name", L"");
			map.is_layer = p.first->second.get<bool>(L"layer", 0);
			
			map.tile_type = p.first->second.get<wstring>(L"tile-type");
			if (map.tile_type == L"image/jpeg")
				map.ext = L"jpg";
			else if (map.tile_type == L"image/png")
				map.ext = L"png";
			else
				throw my::exception(L"Неизвестный тип тайла")
					<< my::param(L"tile-type", map.tile_type);

			wstring projection = p.first->second.get<wstring>(L"projection");
			if (projection == L"spheroid")
				map.projection = tiler::map::spheroid;
			else if (projection == L"ellipsoid")
				map.projection = tiler::map::ellipsoid;
			else
				throw my::exception(L"Неизвестный тип проекции")
					<< my::param(L"projection", projection);

			tiler_.add_map(map);
			p.first++;
		}
	}
	catch(my::exception &e)
	{
		throw my::exception(L"Ошибка загрузки описания карт")
			<< my::param(L"file", L"/maps/maps.xml")
			<< e;
	}
}

/******************************************************************************
* Добавление окна
*/
window* server::add_window(HWND parent_wnd)
{
	window *win = new window(*this, parent_wnd);
	windows_.push_back(win);

	check_state_notify();

	return win;
}

void server::acknowledge(const std::wstring &host)
{
	cmd(L"/pinger/acknowledge?host=" + host);
}
	
void server::unacknowledge(const std::wstring &host)
{
	cmd(L"/pinger/unacknowledge?host=" + host);
}

void server::acknowledge_all()
{
	for( hosts_list::iterator it = hosts_.begin();
		it != hosts_.end(); it++)
	{
		acknowledge(it->first);
	}

	check_state_notify();
}

void server::unacknowledge_all()
{
	for( hosts_list::iterator it = hosts_.begin();
		it != hosts_.end(); it++)
	{
		unacknowledge(it->first);
	}

	check_state_notify();
}

void server::check_state_notify()
{
	/*TODO: sync */
	
	/* Отсылаем всем окнам сообщение об необходимости проверки
		состояния. Через сообщения, потому что можем находиться
		не в том потоке */
	BOOST_FOREACH(window &win, windows_)
		PostMessage( win.hwnd(), MY_WM_CHECK_STATE, 0, 0);
}

void server::get(my::http::reply &reply, const wstring &request)
{
	tcp::socket socket(io_service_);
	socket.connect(server_endpoint_);
	
	string full_request = "GET "
		+ my::http::percent_encode(my::utf8::encode(request))
		+ " HTTP/1.1\r\n\r\n";

	reply.get(socket, full_request);
}

void server::get_header(tcp::socket &socket, my::http::reply &reply,
	const std::wstring &request)
{
	socket.connect(server_endpoint_);
	
	string full_request = "GET "
		+ my::http::percent_encode(my::utf8::encode(request))
		+ " HTTP/1.1\r\n\r\n";

	reply.get(socket, full_request, false);
}

unsigned int server::load_file(const wstring &file,
	const wstring &file_local, bool throw_if_fail)
{
	my::http::reply reply;

	MY_STOPWATCH_START(__load_file_sw1)
    get(reply, file);
	MY_STOPWATCH_FINISH(__load_file_sw1)

	if (reply.status_code == 200)
	{
		MY_STOPWATCH_START(__load_file_sw2)
		reply.save(file_local);
		MY_STOPWATCH_FINISH(__load_file_sw2)
	
		MY_STOPWATCH_OUT(main_log, L"load_file " << file)
		MY_STOPWATCH_OUT(main_log, L"\nload " << __load_file_sw1)
		MY_STOPWATCH_OUT(main_log, L"\nsave " << __load_file_sw2 << main_log)
	}
	else if (throw_if_fail)
		throw my::exception(L"Запрашиваемый файл не найден")
			<< my::param(L"request", file);
	
	return reply.status_code;
}

bool server::cmd(const wstring &request)
{
	my::http::reply reply;

    get(reply, request);

	return reply.status_code == 200;
}

void server::paint_tile(Gdiplus::Graphics *canvas,
	int canvas_x, int canvas_y, int z, int x, int y, int level)
{
	int mask = 0;
	int l = level;
	while (l--) mask = (mask << 1) | 1;

	if (z)
	{
		tiler::tile::ptr tile = get_tile(z, x >> level, y >> level);

		if (!tile || !tile->loaded)
			paint_tile(canvas, canvas_x, canvas_y, z - 1, x, y, level + 1);
		else
		{
			int new_w = 256 >> level;

			Gdiplus::Rect rect(canvas_x, canvas_y, 256, 256);
			canvas->DrawImage(&tile->image, rect,
				(x & mask) * new_w, (y & mask) * new_w, new_w, new_w,
				Gdiplus::UnitPixel, NULL, NULL, NULL );
		}
					
		/* Рамка вокруг тайла, если родного нет */
		/*-
		if (level) {
			Gdiplus::Pen pen(Gdiplus::Color(160, 160, 160), 1);
			canvas->DrawRectangle(&pen, canvas_x, canvas_y, 255, 255);
		}
		-*/
	}
}

void server::on_tiler_update()
{
	BOOST_FOREACH(window &win, windows_)
		PostMessage(win.hwnd(), MY_WM_UPDATE, 0, 0);
}

}
