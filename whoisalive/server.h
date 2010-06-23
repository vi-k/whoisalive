#ifndef WHO_SERVER_H
#define WHO_SERVER_H

#include "ipgui.h"
#include "obj_class.h"
#include "window.h"
#include "tiler.h"

#include "../pinger/host_state.h"

#include "../common/my_inet.h"
#include "../common/my_http.h"
#include "../common/my_thread.h"
#include "../common/my_xml.h"
#include "../common/my_ptr.h"
#include "../common/my_time.h"
#include "../common/my_employer.h"

#include <map>
#include <memory>

#include <boost/ptr_container/ptr_list.hpp>
#include <boost/unordered_map.hpp>

namespace who
{

/* Сервер (по отношению к модулям приложения,
	а не в терминах клиент-серверной технологии) */
class server : my::employer
{
public:
	typedef shared_ptr<server> ptr;
	typedef boost::unordered_map<std::wstring, pinger::host_state> hosts_list;
	typedef boost::function<void ()> anim_handler;
	typedef std::map<int, anim_handler> anim_handlers_list;

private:
	/* io */
	asio::io_service io_service_;
	my::worker::ptr io_worker_;

	void io_thread_proc(my::worker::ptr worker);

	/* Анимация */
	my::worker::ptr anim_worker_;
	anim_handlers_list anim_handlers_;
	int anim_handlers_counter_;

	void anim_thread_proc(my::worker::ptr this_worker);

	/* Мигание */
	int flash_step_;
	bool flash_pause_;
	double flash_alpha_;
	double flash_new_alpha_;

	tcp::endpoint server_endpoint_;
	tcp::socket state_log_socket_;
	ULONG_PTR gdiplus_token_;
	boost::unordered_map<std::wstring, obj_class::ptr> classes_;
	boost::ptr_list<window> windows_;
	posix_time::time_duration anim_period_;
	int def_anim_steps_;
	hosts_list hosts_;
	tiler::server tiler_;
	int active_map_id_;

	void load_classes_();
	void load_maps_();

	void state_log_thread_proc(my::worker::ptr worker);

public:
	server(const xml::wptree &config);
	~server();

	inline void io_wake_up()
		{ wake_up(io_worker_); }

	int add_anim_handler(anim_handler handler);
	void remove_anim_handler(int index);

	inline int def_anim_steps()
		{ return def_anim_steps_; }
	inline posix_time::time_duration anim_period()
		{ return anim_period_; }
		
	inline double flash_alpha()
		{ return flash_alpha_; }

	inline obj_class::ptr obj_class(const std::wstring &class_name)
		{ return classes_[class_name]; }

	window* add_window(HWND hwnd);

	void get(my::http::reply &reply, const std::wstring &request);
	void get_header(tcp::socket &socket, my::http::reply &reply,
		const std::wstring &request);

	unsigned int load_file(const std::wstring &file,
		const std::wstring &file_local, bool throw_if_fail = true);

	bool cmd(const wstring &request);

	/* Состояние ip-адресов, квитирование */
	pinger::host_state host_state(const std::wstring &host)
		{ return hosts_[host]; }
	
	void acknowledge(const std::wstring &host);
	void unacknowledge(const std::wstring &host);
	void acknowledge_all();
	void unacknowledge_all();

	void check_state_notify();

	inline tiler::tile::ptr get_tile(int z, int x, int y)
		{ return tiler_.get_tile(active_map_id_, z, x, y); }

	inline int active_map_id()
		{ return active_map_id_; }
	inline void set_active_map(int id)
		{ active_map_id_ = id; }

	void paint_tile(Gdiplus::Graphics *canvas,
		int canvas_x, int canvas_y, int z, int x, int y, int level = 0);
	void on_tiler_update();

	inline asio::io_service& io_service()
		{ return io_service_; }
};

}

#endif
