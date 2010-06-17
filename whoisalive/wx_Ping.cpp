#include "wx_Ping.h"

#include "../common/my_exception.h"
#include "../common/my_str.h"
#include "../common/my_num.h"
#include "../common/my_utf8.h"

#include <sstream>
#include <istream>
using namespace std;

#include <boost/bind.hpp>
#include <boost/system/system_error.hpp>

#include <wx/msgdlg.h>
#include <wx/dcclient.h>
#include <wx/dcmemory.h>

//(*InternalHeaders(wx_Ping)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/panel.h>
#include <wx/settings.h>
#include <wx/font.h>
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(wx_Ping)
const long wx_Ping::ID_STATEPANEL = wxNewId();
const long wx_Ping::ID_STATICTEXT1 = wxNewId();
const long wx_Ping::ID_STATICTEXT2 = wxNewId();
const long wx_Ping::ID_PINGPANEL = wxNewId();
const long wx_Ping::ID_STATICTEXT3 = wxNewId();
const long wx_Ping::ID_STATICTEXT4 = wxNewId();
const long wx_Ping::ID_PINGTEXTCTRL = wxNewId();
//*)

BEGIN_EVENT_TABLE(wx_Ping,wxFrame)
	//(*EventTable(wx_Ping)
	//*)
END_EVENT_TABLE()

#define BLOCK_W 4

void destroy(wxFrame *frame)
{
	frame->Destroy();
}

wx_Ping::wx_Ping(wxWindow* parent, who::server &server, who::object *object)
	: stop_(false)
	, server_(server)
	, object_(object)
	, pings_socket_(server.io_service())
	, pings_(1000)
	, pings_active_index_(-1)
	, states_socket_(server.io_service())
	, states_(/*1000*/)
	, states_active_index_(-1)
{
	wxWindowID id = -1;

	//(*Initialize(wx_Ping)
	wxBoxSizer* BoxSizer2;
	wxBoxSizer* BoxSizer1;
	wxFlexGridSizer* FlexGridSizer1;

	Create(parent, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE|wxFRAME_TOOL_WINDOW|wxFRAME_FLOAT_ON_PARENT, _T("wxID_ANY"));
	SetClientSize(wxSize(544,333));
	SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
	FlexGridSizer1 = new wxFlexGridSizer(5, 1, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	FlexGridSizer1->AddGrowableRow(2);
	StatePanel = new wxPanel(this, ID_STATEPANEL, wxDefaultPosition, wxSize(400,61), wxTAB_TRAVERSAL, _T("ID_STATEPANEL"));
	FlexGridSizer1->Add(StatePanel, 1, wxTOP|wxLEFT|wxRIGHT|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BoxSizer1 = new wxBoxSizer(wxHORIZONTAL);
	StateFirstText = new wxStaticText(this, ID_STATICTEXT1, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT, _T("ID_STATICTEXT1"));
	wxFont StateFirstTextFont(6,wxDEFAULT,wxFONTSTYLE_NORMAL,wxNORMAL,false,wxEmptyString,wxFONTENCODING_DEFAULT);
	StateFirstText->SetFont(StateFirstTextFont);
	BoxSizer1->Add(StateFirstText, 1, wxRIGHT|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StateLastText = new wxStaticText(this, ID_STATICTEXT2, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxST_NO_AUTORESIZE|wxALIGN_RIGHT, _T("ID_STATICTEXT2"));
	wxFont StateLastTextFont(6,wxDEFAULT,wxFONTSTYLE_NORMAL,wxNORMAL,false,wxEmptyString,wxFONTENCODING_DEFAULT);
	StateLastText->SetFont(StateLastTextFont);
	BoxSizer1->Add(StateLastText, 1, wxLEFT|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(BoxSizer1, 1, wxBOTTOM|wxLEFT|wxRIGHT|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	PingPanel = new wxPanel(this, ID_PINGPANEL, wxDefaultPosition, wxSize(400,61), wxTAB_TRAVERSAL, _T("ID_PINGPANEL"));
	FlexGridSizer1->Add(PingPanel, 1, wxTOP|wxLEFT|wxRIGHT|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BoxSizer2 = new wxBoxSizer(wxHORIZONTAL);
	PingFirstText = new wxStaticText(this, ID_STATICTEXT3, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT, _T("ID_STATICTEXT3"));
	wxFont PingFirstTextFont(6,wxDEFAULT,wxFONTSTYLE_NORMAL,wxNORMAL,false,wxEmptyString,wxFONTENCODING_DEFAULT);
	PingFirstText->SetFont(PingFirstTextFont);
	BoxSizer2->Add(PingFirstText, 1, wxRIGHT|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	PingLastText = new wxStaticText(this, ID_STATICTEXT4, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxST_NO_AUTORESIZE|wxALIGN_RIGHT, _T("ID_STATICTEXT4"));
	wxFont PingLastTextFont(6,wxDEFAULT,wxFONTSTYLE_NORMAL,wxNORMAL,false,wxEmptyString,wxFONTENCODING_DEFAULT);
	PingLastText->SetFont(PingLastTextFont);
	BoxSizer2->Add(PingLastText, 1, wxLEFT|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(BoxSizer2, 1, wxBOTTOM|wxLEFT|wxRIGHT|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	PingTextCtrl = new wxTextCtrl(this, ID_PINGTEXTCTRL, wxEmptyString, wxDefaultPosition, wxSize(400,97), wxTE_AUTO_SCROLL|wxTE_MULTILINE|wxTE_READONLY|wxTE_RICH2|wxTE_NOHIDESEL, wxDefaultValidator, _T("ID_PINGTEXTCTRL"));
	PingTextCtrl->SetForegroundColour(wxColour(192,192,192));
	PingTextCtrl->SetBackgroundColour(wxColour(0,0,0));
	FlexGridSizer1->Add(PingTextCtrl, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->SetSizeHints(this);

	StatePanel->Connect(ID_STATEPANEL,wxEVT_PAINT,(wxObjectEventFunction)&wx_Ping::OnStatePanelPaint,0,this);
	StatePanel->Connect(ID_STATEPANEL,wxEVT_ERASE_BACKGROUND,(wxObjectEventFunction)&wx_Ping::OnPanelsEraseBackground,0,this);
	StatePanel->Connect(ID_STATEPANEL,wxEVT_MOTION,(wxObjectEventFunction)&wx_Ping::OnStatePanelMouseMove,0,this);
	StatePanel->Connect(ID_STATEPANEL,wxEVT_LEAVE_WINDOW,(wxObjectEventFunction)&wx_Ping::OnStatePanelMouseLeave,0,this);
	PingPanel->Connect(ID_PINGPANEL,wxEVT_PAINT,(wxObjectEventFunction)&wx_Ping::OnPingPanelPaint,0,this);
	PingPanel->Connect(ID_PINGPANEL,wxEVT_ERASE_BACKGROUND,(wxObjectEventFunction)&wx_Ping::OnPanelsEraseBackground,0,this);
	PingPanel->Connect(ID_PINGPANEL,wxEVT_MOTION,(wxObjectEventFunction)&wx_Ping::OnPingPanelMouseMove,0,this);
	PingPanel->Connect(ID_PINGPANEL,wxEVT_LEAVE_WINDOW,(wxObjectEventFunction)&wx_Ping::OnPingPanelMouseLeave,0,this);
	Connect(wxID_ANY,wxEVT_CLOSE_WINDOW,(wxObjectEventFunction)&wx_Ping::OnClose);
	//*)

	if (object->hosts().empty())
		throw my::exception(L"Объект не содержит ни один адрес");

	if (object->hosts().size() > 1)
		throw my::exception(L"Объект содержит больше одного адреса");

	wstring host = object->hosts().front();
	wstring name = object->name() + L" / " + host;
	SetLabel(name);

	Show();
}

void wx_Ping::Start(wxWindow* parent, who::server &server, who::object *object)
{
	try
	{
		//shared_ptr<wx_Ping> ptr
		//	= shared_ptr<wx_Ping>(new wx_Ping(parent, server, object), destroy);
		wx_Ping *ptr = new wx_Ping(parent, server, object);
		ptr->start();
	}
	catch(my::exception &e)
	{
		wxMessageBox(e.message(), L"Ошибка", wxOK | wxICON_ERROR, parent);
	}
}

void wx_Ping::start()
{
	wstring host = object_->hosts().front();

	/* states */
	server_.get_header(states_socket_, states_reply_,
		L"/pinger/state.log?address=" + host);
	
	states_reply_.buf_.consume(states_reply_.buf_.size());
	states_reply_.buf_.prepare(65536);

	shared_lock_ptr lock_ptr(new shared_lock<shared_mutex>(i_work_mutex_));

	asio::async_read_until(
		states_socket_, states_reply_.buf_, "\r\n",
		boost::bind(&wx_Ping::states_handle_read, this,
            lock_ptr,
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred) );

	/* pings */
	server_.get_header(pings_socket_, pings_reply_,
		L"/pinger/ping.log?address=" + host);

	pings_reply_.buf_.consume(pings_reply_.buf_.size());
	pings_reply_.buf_.prepare(65536);

	asio::async_read_until(
		pings_socket_, pings_reply_.buf_, "\r\n",
		boost::bind(&wx_Ping::pings_handle_read, this,
            lock_ptr,
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred) );

	server_.io_wake_up();
}

wx_Ping::~wx_Ping()
{
	//(*Destroy(wx_Ping)
	//*)

	stop_ = true;

	states_socket_.close();
	pings_socket_.close();

	unique_lock<shared_mutex> l(i_work_mutex_);
}

void wx_Ping::OnClose(wxCloseEvent& event)
{
	//event.Veto();
	Destroy();
}

/* Асинхронное чтение состояний */
void wx_Ping::states_handle_read( shared_lock_ptr lock_ptr,
	const boost::system::error_code& error, size_t bytes_transferred )
{
	if (stop_) return;

	if (!error)
	{
		Refresh();

		states_reply_.body.resize(bytes_transferred);
		states_reply_.buf_.sgetn((char*)states_reply_.body.c_str(), bytes_transferred);

		wstringstream out;
		wxTextAttr style;

		if (states_reply_.body == "START_ARCHIVE\r\n")
		{
			asio::async_read_until(
				states_socket_, states_reply_.buf_, "END_ARCHIVE\r\n",
				boost::bind(&wx_Ping::states_handle_read, this,
					lock_ptr,
					boost::asio::placeholders::error,
					boost::asio::placeholders::bytes_transferred) );
		}
		else
		{
			wistringstream ss( my::utf8::decode(states_reply_.body) );

			/* Если загружаем архив, то это будет больше, чем одна строка */
			while (true)
			{
				/* Первым идёт название хоста */
				wstring host;
				ss >> host;

				if (host == L"END_ARCHIVE")
					continue;

				/* Вторым само состояние */
				pinger::host_state state;
				ss >> state;

				if (!ss)
					break;

				posix_time::ptime utc_time = state.time();
				state.set_time( my::time::utc_to_local(utc_time) );

				wostringstream out;

				{
					scoped_lock l(states_mutex_);
					states_[utc_time] = state;
					states_active_index_ = -1;
				}
			} /* while (ss) */

			states_repaint();

			asio::async_read_until(
				states_socket_, states_reply_.buf_, "\r\n",
				boost::bind(&wx_Ping::states_handle_read, this,
					lock_ptr,
					boost::asio::placeholders::error,
					boost::asio::placeholders::bytes_transferred) );

		} /* else // if (states_reply_.body == "START_ARCHIVE\r\n") */

		if (first_state_time_.is_special())
			StateFirstText->SetLabel(L"");
		else
			StateFirstText->SetLabel(
				my::time::to_wstring(first_state_time_, L"%d-%m-%Y\n%H:%M:%S") );

		if (last_state_time_.is_special())
			StateLastText->SetLabel(L"");
		else
			StateLastText->SetLabel(
				my::time::to_wstring(last_state_time_, L"%d-%m-%Y\n%H:%M:%S") );
	}
	else
	{
		boost::system::system_error se(error);
		wstring str = my::str::to_wstring( se.what() );

		wxMessageBox(str, L"Ошибка чтения данных",
			wxOK | wxICON_ERROR, this);
	}
}

/* Асинхронное чтение пингов */
void wx_Ping::pings_handle_read( shared_lock_ptr lock_ptr,
	const boost::system::error_code& error, size_t bytes_transferred )
{
	if (stop_) return;

	if (!error)
	{
		Refresh();

		pings_reply_.body.resize(bytes_transferred);
		pings_reply_.buf_.sgetn((char*)pings_reply_.body.c_str(), bytes_transferred);

		wstringstream out;
		wxTextAttr style;

		if (pings_reply_.body == "START_ARCHIVE\r\n")
		{
			asio::async_read_until(
				pings_socket_, pings_reply_.buf_, "END_ARCHIVE\r\n",
				boost::bind(&wx_Ping::pings_handle_read, this,
					lock_ptr,
					boost::asio::placeholders::error,
					boost::asio::placeholders::bytes_transferred) );
		}
		else
		{
			wistringstream ss( my::utf8::decode(pings_reply_.body) );

			PingTextCtrl->Freeze();

			/* Если загружаем архив, то это будет больше, чем одна строка */
			while (true)
			{
				/* Первым идёт название хоста */
				wstring host;
				ss >> host;

				if (host == L"END_ARCHIVE")
					continue;

				/* Вторым сам пинг */
				pinger::ping_result ping;
				ss >> ping;

				if (!ss)
					break;

				ping.set_time( my::time::utc_to_local(ping.time()) );

				wostringstream out;

				wxTextAttr style;

				unsigned short num = ping.sequence_number();

				if (ping.state() == pinger::ping_result::unknown)
				{
					style = wxTextAttr(*wxLIGHT_GREY);
					out << L"unknown message\n";
				}
				else
				{
					{
						scoped_lock l(pings_mutex_);
						pings_[num] = ping;
						pings_active_index_ = -1;
					}

					switch (ping.state())
					{
						case pinger::ping_result::ok:
							style = wxTextAttr(*wxGREEN);
							out << my::time::to_wstring(ping.time(), L"[%H:%M:%S]")
								<< L' ' << ping.ipv4_hdr().header_length()
								<< L" bytes from " << ping.ipv4_hdr().source_address()
								<< L", icmp_seq=" << num
								<< L", ttl=" << ping.ipv4_hdr().time_to_live()
								<< L", time=" << ping.duration().total_milliseconds() << L" ms"
								<< L"\n";
							break;

						case pinger::ping_result::timeout:
							style = wxTextAttr(*wxRED);
							out << my::time::to_wstring(ping.time(), L"[%H:%M:%S]")
								<< L" timeout (icmp_seq=" << num
								<< L", time=" << ping.duration().total_milliseconds() << L" ms"
								<< L")\n";
							break;
					}
				}

				PingTextCtrl->SetDefaultStyle(style);
				*PingTextCtrl << out.str();

			} /* while (ss) */

			PingTextCtrl->Thaw();

			pings_repaint();

			asio::async_read_until(
				pings_socket_, pings_reply_.buf_, "\r\n",
				boost::bind(&wx_Ping::pings_handle_read, this,
					lock_ptr,
					boost::asio::placeholders::error,
					boost::asio::placeholders::bytes_transferred) );

		} /* else // if (pings_reply_.body == "START_ARCHIVE\r\n") */

		if (first_ping_time_.is_special())
			PingFirstText->SetLabel(L"");
		else
			PingFirstText->SetLabel(
				my::time::to_wstring(first_ping_time_, L"%d-%m-%Y\n%H:%M:%S") );

		if (last_ping_time_.is_special())
			PingLastText->SetLabel(L"");
		else
			PingLastText->SetLabel(
				my::time::to_wstring(last_ping_time_, L"%d-%m-%Y\n%H:%M:%S") );
	}
	else
	{
		boost::system::system_error se(error);
		wstring str = my::str::to_wstring( se.what() );

		wxMessageBox(str, L"Ошибка чтения данных",
			wxOK | wxICON_ERROR, this);
	}
}

/* Прорисовка состояний */
void wx_Ping::states_repaint()
{
	scoped_lock l(states_bitmap_mutex_);

	first_state_time_ = last_state_time_ = posix_time::ptime();

	int w, h;
	StatePanel->GetClientSize(&w, &h);

	if (states_bitmap_.GetWidth() != w || states_bitmap_.GetHeight() != h)
		states_bitmap_.Create(w, h);

	wxMemoryDC dc(states_bitmap_);

	int ok_y = h - 4;
	int warn_y = h /2;
	int fail_y = 3;

	/* Стираем */
	dc.SetBrush(*wxBLACK_BRUSH);
	dc.DrawRectangle(0, 0, w, h);

	/* Рисуем границы */
	dc.SetPen(*wxGREY_PEN);
	dc.DrawLine(0, ok_y, w, ok_y);
	dc.DrawLine(0, warn_y, w, warn_y);
	dc.DrawLine(0, fail_y, w, fail_y);

	//dc.SetPen(*wxGREEN_PEN);

	int prev_y = -1;
	int index = 0;

	scoped_lock ll(states_mutex_);

	/*-
	for (states_list::iterator iter = states_.begin();
		iter != states_.end(); iter++)
	{
		pinger::host_state state = iter->value();

		if (index++ == 0)
			last_state_time_ = state.time();

		int x = w - index * BLOCK_W;

		if (x <= 0 && x > -BLOCK_W)
			first_state_time_ = state.time();

		if (state.state() == pinger::ping_result::timeout)
		{
			prev_y = -1;
			continue;
		}

		int y = zero_y - (double)ping.duration().total_milliseconds()
			/ timeout * (zero_y - timeout_y);
		if (y < 0)
			y = 0;

		if (prev_y != -1)
			dc.DrawLine(x + BLOCK_W, prev_y, x + BLOCK_W, y);

		dc.DrawLine(x + BLOCK_W, y, x, y);

		prev_y = y;
	}
	-*/

	StatePanel->Refresh();
}

void wx_Ping::pings_repaint()
{
	scoped_lock l(pings_bitmap_mutex_);

	first_ping_time_ = last_ping_time_ = posix_time::ptime();

	int w, h;
	PingPanel->GetClientSize(&w, &h);

	if (pings_bitmap_.GetWidth() != w || pings_bitmap_.GetHeight() != h)
		pings_bitmap_.Create(w, h);

	wxMemoryDC dc(pings_bitmap_);

	int zero_y = h - 4;

	double timeout = 2000; /*TODO: не должно быть жестко зашито */
	int timeout_y = h * 0.2;

	/* Стираем */
	dc.SetBrush(*wxBLACK_BRUSH);
	dc.DrawRectangle(0, 0, w, h);

	/* Рисуем нижнюю и верхнюю границы */
	dc.SetPen(*wxGREY_PEN);
	dc.DrawLine(0, zero_y, w, zero_y);
	dc.DrawLine(0, timeout_y, w, timeout_y);

	dc.SetPen(*wxGREEN_PEN);

	int prev_y = -1;
	int index = 0;

	scoped_lock ll(pings_mutex_);

	for (pings_list::iterator iter = pings_.begin();
		iter != pings_.end(); iter++)
	{
		pinger::ping_result ping = iter->value();

		if (index++ == 0)
			last_ping_time_ = ping.time();

		int x = w - index * BLOCK_W;

		if (x <= 0 && x > -BLOCK_W)
			first_ping_time_ = ping.time();

		if (ping.state() == pinger::ping_result::timeout)
		{
			prev_y = -1;
			continue;
		}

		int y = zero_y - (double)ping.duration().total_milliseconds()
			/ timeout * (zero_y - timeout_y);
		if (y < 0)
			y = 0;

		if (prev_y != -1)
			dc.DrawLine(x + BLOCK_W, prev_y, x + BLOCK_W, y);

		dc.DrawLine(x + BLOCK_W, y, x, y);

		prev_y = y;
	}

	PingPanel->Refresh();
}

void wx_Ping::on_pingpanel_mousemove(wxMouseEvent& event)
{
	int w, h;
	PingPanel->GetClientSize(&w, &h);

	pings_active_index_ = (w - event.GetX() + 1) / BLOCK_W;
	PingPanel->Refresh();

	scoped_lock l(pings_mutex_);

	int index = pings_active_index_;
	pings_list::iterator iter = pings_.begin();

	while (iter != pings_.end() && index-- != 0)
		iter++;

	if (iter != pings_.end())
	{
		wostringstream out;

		out << iter->value().state() << endl
			<< my::time::to_wstring(iter->value().time(), L"%Y-%m-%d %H:%M:%S") << endl
			<< L"icmp_seq=" << iter->key() << endl
			<< L"time=" << iter->value().duration().total_milliseconds() << L" ms";

		PingPanel->SetToolTip(out.str());
	}
}

void wx_Ping::on_pingpanel_mouseleave(wxMouseEvent& event)
{
}

void wx_Ping::OnStatePanelPaint(wxPaintEvent& event)
{
	scoped_lock l(states_bitmap_mutex_);

	if (states_bitmap_.IsOk())
	{
		wxPaintDC dc(StatePanel);
		dc.DrawBitmap(states_bitmap_, 0, 0);

		/*-
		if (states_active_index_ >= 0)
		{
			int w, h;
			StatePanel->GetClientSize(&w, &h);

			dc.SetPen(*wxWHITE_PEN);
			int x = w - states_active_index_ * BLOCK_W - BLOCK_W / 2;

			dc.DrawLine(x, 0, x, h);
		}
		-*/
	}

	event.Skip(false);
}

void wx_Ping::OnPingPanelPaint(wxPaintEvent& event)
{
	scoped_lock l(pings_bitmap_mutex_);

	if (pings_bitmap_.IsOk())
	{
		wxPaintDC dc(PingPanel);
		dc.DrawBitmap(pings_bitmap_, 0, 0);

		if (pings_active_index_ >= 0)
		{
			int w, h;
			PingPanel->GetClientSize(&w, &h);

			dc.SetPen(*wxWHITE_PEN);
			int x = w - pings_active_index_ * BLOCK_W - BLOCK_W / 2;

			dc.DrawLine(x, 0, x, h);
		}
	}

	event.Skip(false);
}

void wx_Ping::OnStatePanelEraseBackground(wxEraseEvent& event)
{
	event.Skip(false);
}

void wx_Ping::OnPingPanelEraseBackground(wxEraseEvent& event)
{
	event.Skip(false);
}

void wx_Ping::OnPanelsEraseBackground(wxEraseEvent& event)
{
	event.Skip(false);
}

void wx_Ping::OnStatePanelMouseMove(wxMouseEvent& event)
{
	/*-
	int w, h;
	StatePingPanel->GetClientSize(&w, &h);

	states_active_index_ = (w - event.GetX() + 1) / BLOCK_W;
	StatePanel->Refresh();

	scoped_lock l(states_mutex_);

	int index = states_active_index_;
	states_list::iterator iter = states_.begin();

	while (iter != states_.end() && index-- != 0)
		iter++;

	if (iter != states_.end())
	{
		wostringstream out;

		out << iter->value().state() << endl
			<< my::time::to_wstring(iter->value().time(), L"%Y-%m-%d %H:%M:%S") << endl
			<< L"icmp_seq=" << iter->key() << endl
			<< L"time=" << iter->value().duration().total_milliseconds() << L" ms";

		StatePanel->SetToolTip(out.str());
	}
	-*/
}

void wx_Ping::OnPingPanelMouseMove(wxMouseEvent& event)
{
	int w, h;
	PingPanel->GetClientSize(&w, &h);

	pings_active_index_ = (w - event.GetX() + 1) / BLOCK_W;
	PingPanel->Refresh();

	scoped_lock l(pings_mutex_);

	int index = pings_active_index_;
	pings_list::iterator iter = pings_.begin();

	while (iter != pings_.end() && index-- != 0)
		iter++;

	if (iter != pings_.end())
	{
		wostringstream out;

		out << iter->value().state() << endl
			<< my::time::to_wstring(iter->value().time(), L"%Y-%m-%d %H:%M:%S") << endl
			<< L"icmp_seq=" << iter->key() << endl
			<< L"time=" << iter->value().duration().total_milliseconds() << L" ms";

		PingPanel->SetToolTip(out.str());
	}
}

void wx_Ping::OnStatePanelMouseLeave(wxMouseEvent& event)
{
	states_active_index_ = -1;
	StatePanel->Refresh();
}

void wx_Ping::OnPingPanelMouseLeave(wxMouseEvent& event)
{
	pings_active_index_ = -1;
	PingPanel->Refresh();
}
