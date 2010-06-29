#ifndef WX_PING_H
#define WX_PING_H

#include "time_grid.h"

#include "server.h"
#include "../pinger/ping_result.h"

#include "../common/my_inet.h"
#include "../common/my_http.h"
#include "../common/my_thread.h"
#include "../common/my_mru.h"
#include "../common/my_time.h"
#include "../common/my_employer.h"

#include <memory>
#include <cmath>

#include <boost/config/warning_disable.hpp> /* против unsafe */

#include <wx/msw/setup.h>

//(*Headers(wx_Ping)
#include <wx/frame.h>
class wxPanel;
class wxTextCtrl;
class wxStaticText;
class wxFlexGridSizer;
//*)

#include <wx/textctrl.h>
#include <wx/bitmap.h>

class wx_Ping : public wxFrame, my::employer
{
private:

	typedef std::map<posix_time::ptime, pinger::ping_result> pings_list;
	typedef std::map<posix_time::ptime, pinger::host_state> states_list;

	who::server &server_;
	who::object *object_;

	/* Асинронные операции */
	asio::io_service io_service_;
	void io_thread_proc(my::worker::ptr this_worker);

	/* Анимация */
	int anim_handler_index_;
	void anim_handler(my::worker::ptr this_worker);

	/* Состояния */
	states_list states_;
	shared_mutex states_list_mutex_;
	tcp::socket states_socket_;
	my::http::reply states_reply_;
	scoped_ptr<my::time_grid> states_grid_;

	void states_handle_read( my::worker::ptr worker,
		const boost::system::error_code& error, size_t bytes_transferred );

	void states_on_before_paint(wxGraphicsContext *gc,
		wxDouble width, wxDouble height, posix_time::ptime right_bound,
		posix_time::time_duration resolution);

	void states_on_after_paint(wxGraphicsContext *gc,
		wxDouble width, wxDouble height, posix_time::ptime right_bound,
		posix_time::time_duration resolution);

	//pinger::host_state get_state_by_offset(int offset);


	/* Пинги */
	pings_list pings_;
	shared_mutex pings_list_mutex_;
	tcp::socket pings_socket_;
	my::http::reply pings_reply_;
	scoped_ptr<my::time_grid> pings_grid_;

	void pings_handle_read( my::worker::ptr worker,
		const boost::system::error_code& error, size_t bytes_transferred );

	void pings_on_before_paint(wxGraphicsContext *gc,
		wxDouble width, wxDouble height, posix_time::ptime right_bound,
		posix_time::time_duration resolution);

	void pings_on_after_paint(wxGraphicsContext *gc,
		wxDouble width, wxDouble height, posix_time::ptime right_bound,
		posix_time::time_duration resolution);

	//(*Handlers(wx_Ping)
	void OnStatePanelMouseMove(wxMouseEvent& event);
	void OnPingPanelMouseMove(wxMouseEvent& event);
	void OnStatePanelMouseEnter(wxMouseEvent& event);
	void OnPingPanelMouseEnter(wxMouseEvent& event);
	//*)

	DECLARE_EVENT_TABLE()

protected:

	//(*Identifiers(wx_Ping)
	static const long ID_STATICTEXT5;
	static const long ID_STATEPANEL;
	static const long ID_STATICTEXT1;
	static const long ID_PINGPANEL;
	static const long ID_PINGTEXTCTRL;
	//*)

public:
	wx_Ping(wxWindow* parent, who::server &server, who::object *object);
	virtual ~wx_Ping();

	static void Open(wxWindow* parent, who::server &server, who::object *object);

	//(*Declarations(wx_Ping)
	wxStaticText* StaticText2;
	wxPanel* StatePanel;
	wxStaticText* StaticText1;
	wxTextCtrl* PingTextCtrl;
	wxPanel* PingPanel;
	//*)
};

#endif
