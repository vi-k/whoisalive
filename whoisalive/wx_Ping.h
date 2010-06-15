#ifndef WX_PING_H
#define WX_PING_H

#include "server.h"
#include "../pinger/ping_result.h"

#include "../common/my_inet.h"
#include "../common/my_http.h"
#include "../common/my_thread.h"
#include "../common/my_mru.h"
#include "../common/my_time.h"

#include <memory>

#include <boost/config/warning_disable.hpp> /* против unsafe */
#include <boost/smart_ptr/enable_shared_from_this2.hpp>

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

class wx_Ping: public wxFrame, public boost::enable_shared_from_this2<wx_Ping>
{
private:
	typedef my::mru::list<unsigned short, pinger::ping_result> pings_list;
	typedef std::map<posix_time::ptime, pinger::host_state> states_list;

	int terminate_;
	who::server &server_;
	who::object *object_;

	pings_list pings_;
	tcp::socket pings_socket_;
	my::http::reply pings_reply_;
	mutex pings_read_mutex_;
	posix_time::ptime first_ping_time_;
	posix_time::ptime last_ping_time_;
	mutex pings_mutex_;
	wxBitmap pings_bitmap_;
	mutex pings_bitmap_mutex_;
	int pings_active_index_;

	states_list states_;
	tcp::socket states_socket_;
	my::http::reply states_reply_;
	mutex states_read_mutex_;
	posix_time::ptime first_state_time_;
	posix_time::ptime last_state_time_;
	mutex states_mutex_;
	wxBitmap states_bitmap_;
	mutex states_bitmap_mutex_;
	int states_active_index_;

	void states_handle_read(const boost::system::error_code& error,
		size_t bytes_transferred);

	void states_repaint();

	void pings_handle_read(const boost::system::error_code& error,
		size_t bytes_transferred);

	void pings_repaint();

	//(*Handlers(wx_Ping)
	void OnClose(wxCloseEvent& event);
	void on_pingpanel_mousemove(wxMouseEvent& event);
	void on_pingpanel_mouseleave(wxMouseEvent& event);
	void OnStatePanelPaint(wxPaintEvent& event);
	void OnStatePanelEraseBackground(wxEraseEvent& event);
	void OnPingPanelPaint(wxPaintEvent& event);
	void OnPingPanelEraseBackground(wxEraseEvent& event);
	void OnPanelsEraseBackground(wxEraseEvent& event);
	void OnPingPanelMouseMove(wxMouseEvent& event);
	void OnPingPanelMouseLeave(wxMouseEvent& event);
	void OnStatePanelMouseMove(wxMouseEvent& event);
	void OnStatePanelMouseLeave(wxMouseEvent& event);
	//*)

	DECLARE_EVENT_TABLE()

protected:

	//(*Identifiers(wx_Ping)
	static const long ID_STATEPANEL;
	static const long ID_STATICTEXT1;
	static const long ID_STATICTEXT2;
	static const long ID_PINGPANEL;
	static const long ID_STATICTEXT3;
	static const long ID_STATICTEXT4;
	static const long ID_PINGTEXTCTRL;
	//*)

	void start();

public:
	wx_Ping(wxWindow* parent, who::server &server, who::object *object);
	virtual ~wx_Ping();

	static void Start(wxWindow* parent, who::server &server, who::object *object);

	//(*Declarations(wx_Ping)
	wxPanel* StatePanel;
	wxStaticText* StateFirstText;
	wxStaticText* StateLastText;
	wxStaticText* PingFirstText;
	wxStaticText* PingLastText;
	wxTextCtrl* PingTextCtrl;
	wxPanel* PingPanel;
	//*)
};

#endif
