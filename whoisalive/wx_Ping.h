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

class wx_Ping: public wxFrame
{
private:
	typedef my::mru::list<unsigned short, pinger::ping_result> pings_list;

	bool terminate_;
	who::server &server_;
	tcp::socket socket_;
	who::object *object_;

	my::http::reply reply_;
	mutex read_mutex_;
	pings_list pings_;
	posix_time::ptime first_ping_;
	posix_time::ptime last_ping_;
	mutex pings_mutex_;
	wxBitmap bitmap_;
	mutex bitmap_mutex_;
	int active_index_;

	void handle_read(const boost::system::error_code& error,
		size_t bytes_transferred);

	void repaint();

	//(*Handlers(wx_Ping)
	void OnClose(wxCloseEvent& event);
	void on_pingpanel_paint(wxPaintEvent& event);
	void on_pingpanel_mousemove(wxMouseEvent& event);
	void on_pingpanel_erasebackground(wxEraseEvent& event);
	void on_pingpanel_mouseleave(wxMouseEvent& event);
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
	
public:
	wx_Ping(wxWindow* parent, who::server &server, who::object *object);
	virtual ~wx_Ping();

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
