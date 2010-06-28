#ifndef WX_PING_H
#define WX_PING_H

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
class wxBoxSizer;
//*)

#include <wx/textctrl.h>
#include <wx/bitmap.h>

class wx_Ping : public wxFrame, my::employer
{
private:

	typedef my::mru::list<unsigned short, pinger::ping_result> pings_list;
	typedef std::map<posix_time::ptime, pinger::host_state> states_list;

	who::server &server_;
	who::object *object_;

	/* Асинронные операции */
	asio::io_service io_service_;
	void io_thread_proc(my::worker::ptr this_worker);

	/* Анимация состояний */
	int anim_handler_index_;
	void anim_handler(my::worker::ptr this_worker);

	/* Состояния */
	states_list states_;
	shared_mutex states_list_mutex_;
	tcp::socket states_socket_;
	my::http::reply states_reply_;
	posix_time::ptime states_start_time_;
	posix_time::ptime states_cursor_time_;
	double states_z_;
	double new_states_z_;
	int states_z_step_;
	wxBitmap states_bitmap_;
	wxBitmap states_background_;
	wxBitmap states_caption_bitmap_;
	mutex states_bitmap_mutex_;
	std::size_t states_hash_;
	wxCoord states_move_x_;
	posix_time::ptime states_move_time_;
	shared_mutex states_params_mutex_;

	posix_time::ptime states_start_time();
	posix_time::ptime states_cursor_time();
	void states_handle_read( my::worker::ptr worker,
		const boost::system::error_code& error, size_t bytes_transferred );
	void states_paint();
	void states_paint_background();

	static posix_time::time_duration states_resolution(double z)
	{
		int iz = (int)z; /* Отбрасываем дробную часть */

		/* 250ms, 500ms, 1s, 2s, 4s, 8s, 16s, 32s, ... */
		posix_time::time_duration res
			= posix_time::milliseconds(250) * (2 << iz);

		return res + my::time::mul(res, z - (double)iz);
	}

	pinger::host_state get_state_by_offset(int offset);


	/* Пинги */
	pings_list pings_;
	tcp::socket pings_socket_;
	my::http::reply pings_reply_;
	posix_time::ptime first_ping_time_;
	posix_time::ptime last_ping_time_;
	shared_mutex pings_mutex_;
	wxBitmap pings_bitmap_;
	mutex pings_bitmap_mutex_;
	int pings_active_index_;

	void pings_handle_read( my::worker::ptr worker,
		const boost::system::error_code& error, size_t bytes_transferred );
	void pings_repaint();


	inline wxDouble time_to_x(
		const posix_time::ptime &time,
		const posix_time::ptime &start_time,
		const posix_time::time_duration &resolution,
		wxDouble width);

	inline posix_time::ptime x_to_time(
		wxDouble x,
		const posix_time::ptime &start_time,
		const posix_time::time_duration &resolution,
		wxDouble width);

	wxDouble states_time_to_x(const posix_time::ptime &time);
	posix_time::ptime states_x_to_time(wxDouble x);

	void prepare_buffer(wxWindow *win, wxBitmap *bmp,
		wxDouble *pw, wxDouble *ph);

	//(*Handlers(wx_Ping)
	void OnStatePanelPaint(wxPaintEvent& event);
	void OnPingPanelPaint(wxPaintEvent& event);
	void OnPanelsEraseBackground(wxEraseEvent& event);
	void OnPingPanelMouseMove(wxMouseEvent& event);
	void OnPingPanelMouseLeave(wxMouseEvent& event);
	void OnStatePanelMouseMove(wxMouseEvent& event);
	void OnStatePanelLeftDown(wxMouseEvent& event);
	void OnStatePanelRightDown(wxMouseEvent& event);
	void OnStatePanelLeftUp(wxMouseEvent& event);
	void OnStatePanelMouseWheel(wxMouseEvent& event);
	//*)

	DECLARE_EVENT_TABLE()

protected:

	//(*Identifiers(wx_Ping)
	static const long ID_STATICTEXT5;
	static const long ID_STATEPANEL;
	static const long ID_STATICTEXT1;
	static const long ID_PINGPANEL;
	static const long ID_STATICTEXT3;
	static const long ID_STATICTEXT4;
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
	wxStaticText* PingFirstText;
	wxStaticText* PingLastText;
	wxTextCtrl* PingTextCtrl;
	wxPanel* PingPanel;
	//*)
};

#endif
