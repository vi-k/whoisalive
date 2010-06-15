#include "wx_Ping.h"

#include "../common/my_exception.h"
#include "../common/my_str.h"
#include "../common/my_num.h"
#include "../common/my_utf8.h"
#include "../common/my_stopwatch.h"

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

wx_Ping::wx_Ping(wxWindow* parent, who::server &server, who::object *object)
	: terminate_(false)
	, server_(server)
	, socket_(server.io_service())
	, object_(object)
	, pings_(1000)
	, active_index_(-1)
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
	StateFirstText = new wxStaticText(this, ID_STATICTEXT1, _("\n"), wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT, _T("ID_STATICTEXT1"));
	wxFont StateFirstTextFont(6,wxDEFAULT,wxFONTSTYLE_NORMAL,wxNORMAL,false,wxEmptyString,wxFONTENCODING_DEFAULT);
	StateFirstText->SetFont(StateFirstTextFont);
	BoxSizer1->Add(StateFirstText, 1, wxRIGHT|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StateLastText = new wxStaticText(this, ID_STATICTEXT2, _("\n"), wxDefaultPosition, wxDefaultSize, wxST_NO_AUTORESIZE|wxALIGN_RIGHT, _T("ID_STATICTEXT2"));
	wxFont StateLastTextFont(6,wxDEFAULT,wxFONTSTYLE_NORMAL,wxNORMAL,false,wxEmptyString,wxFONTENCODING_DEFAULT);
	StateLastText->SetFont(StateLastTextFont);
	BoxSizer1->Add(StateLastText, 1, wxLEFT|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(BoxSizer1, 1, wxBOTTOM|wxLEFT|wxRIGHT|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	PingPanel = new wxPanel(this, ID_PINGPANEL, wxDefaultPosition, wxSize(400,61), wxTAB_TRAVERSAL, _T("ID_PINGPANEL"));
	FlexGridSizer1->Add(PingPanel, 1, wxTOP|wxLEFT|wxRIGHT|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BoxSizer2 = new wxBoxSizer(wxHORIZONTAL);
	PingFirstText = new wxStaticText(this, ID_STATICTEXT3, _("\n"), wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT, _T("ID_STATICTEXT3"));
	wxFont PingFirstTextFont(6,wxDEFAULT,wxFONTSTYLE_NORMAL,wxNORMAL,false,wxEmptyString,wxFONTENCODING_DEFAULT);
	PingFirstText->SetFont(PingFirstTextFont);
	BoxSizer2->Add(PingFirstText, 1, wxRIGHT|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	PingLastText = new wxStaticText(this, ID_STATICTEXT4, _("\n"), wxDefaultPosition, wxDefaultSize, wxST_NO_AUTORESIZE|wxALIGN_RIGHT, _T("ID_STATICTEXT4"));
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

	PingPanel->Connect(ID_PINGPANEL,wxEVT_PAINT,(wxObjectEventFunction)&wx_Ping::on_pingpanel_paint,0,this);
	PingPanel->Connect(ID_PINGPANEL,wxEVT_ERASE_BACKGROUND,(wxObjectEventFunction)&wx_Ping::on_pingpanel_erasebackground,0,this);
	PingPanel->Connect(ID_PINGPANEL,wxEVT_MOTION,(wxObjectEventFunction)&wx_Ping::on_pingpanel_mousemove,0,this);
	PingPanel->Connect(ID_PINGPANEL,wxEVT_LEAVE_WINDOW,(wxObjectEventFunction)&wx_Ping::on_pingpanel_mouseleave,0,this);
	Connect(wxID_ANY,wxEVT_CLOSE_WINDOW,(wxObjectEventFunction)&wx_Ping::OnClose);
	//*)

	if (object->hosts().empty())
		throw my::exception(L"Объект не содержит ни один адрес");

	if (object->hosts().size() > 1)
		throw my::exception(L"Объект содержит больше одного адреса");

	wstring host = object->hosts().front();
	wstring name = object->name() + L" / " + host;
	SetLabel(name);

	server_.get_header(socket_, reply_,
		L"/pinger/ping.log?address=" + host);

	reply_.buf_.consume(reply_.buf_.size());
	reply_.buf_.prepare(65536);

	asio::async_read_until(
		socket_, reply_.buf_, "\r\n",
		boost::bind(&wx_Ping::handle_read, this,
            boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred) );

	server_.io_wake_up();

	Show();
}

wx_Ping::~wx_Ping()
{
	terminate_ = true;

	scoped_lock l(read_mutex_);
	socket_.close();

	//(*Destroy(wx_Ping)
	//*)
}

void wx_Ping::OnClose(wxCloseEvent& event)
{
	delete this;
}

void wx_Ping::handle_read(const boost::system::error_code& error,
	size_t bytes_transferred)
{
	if (terminate_)
		return;

	scoped_lock l(read_mutex_);

	if (!error)
	{
		Refresh();

		reply_.body.resize(bytes_transferred);
		reply_.buf_.sgetn((char*)reply_.body.c_str(), bytes_transferred);

		wstringstream out;
		wxTextAttr style;

		if (reply_.body == "START_ARCHIVE\r\n")
		{
			asio::async_read_until(
				socket_, reply_.buf_, "END_ARCHIVE\r\n",
				boost::bind(&wx_Ping::handle_read, this,
					boost::asio::placeholders::error,
					boost::asio::placeholders::bytes_transferred) );
		}
		else
		{
			wistringstream ss( my::utf8::decode(reply_.body) );

			PingTextCtrl->Freeze();

			my::stopwatch timer;
			timer.start();

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
						active_index_ = -1;
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

			timer.finish();

			PingTextCtrl->Thaw();

			*PingTextCtrl << timer.to_wstring() << L'\n';

			repaint();

			asio::async_read_until(
				socket_, reply_.buf_, "\r\n",
				boost::bind(&wx_Ping::handle_read, this,
					boost::asio::placeholders::error,
					boost::asio::placeholders::bytes_transferred) );

		} /* else // if (reply_.body == "START_ARCHIVE\r\n") */
	}
	else
	{
		boost::system::system_error se(error);
		wstring str = my::str::to_wstring( se.what() );

		wxMessageBox(str, L"Ошибка чтения данных",
			wxOK | wxICON_ERROR, this);
		Close();
	}

	StateFirstText->SetLabel(L"[");
	StateLastText->SetLabel(L"]");

	if (first_ping_.is_special())
		PingFirstText->SetLabel(L"");
	else
		PingFirstText->SetLabel(
			my::time::to_wstring(first_ping_, L"%d-%m-%Y\n%H:%M:%S") );

	if (last_ping_.is_special())
		PingLastText->SetLabel(L"");
	else
		PingLastText->SetLabel(
			my::time::to_wstring(last_ping_, L"%d-%m-%Y\n%H:%M:%S") );
}

void wx_Ping::repaint()
{
	scoped_lock l(bitmap_mutex_);

	first_ping_ = last_ping_ = posix_time::ptime();

	int w, h;
	PingPanel->GetClientSize(&w, &h);

	if (bitmap_.GetWidth() != w || bitmap_.GetHeight() != h)
		bitmap_.Create(w, h);

	wxMemoryDC dc(bitmap_);

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
			last_ping_ = ping.time();

		int x = w - index * BLOCK_W;

		if (x <= 0 && x > -BLOCK_W)
			first_ping_ = ping.time();

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

void wx_Ping::on_pingpanel_paint(wxPaintEvent& event)
{
	scoped_lock l(bitmap_mutex_);

	if (bitmap_.IsOk())
	{
		wxPaintDC dc(PingPanel);
		dc.DrawBitmap(bitmap_, 0, 0);

		if (active_index_ >= 0)
		{
			int w, h;
			PingPanel->GetClientSize(&w, &h);

			dc.SetPen(*wxWHITE_PEN);
			int x = w - active_index_ * BLOCK_W - BLOCK_W / 2;

			dc.DrawLine(x, 0, x, h);
		}
	}

	event.Skip(false);
}

void wx_Ping::on_pingpanel_erasebackground(wxEraseEvent& event)
{
	event.Skip(false);
}

void wx_Ping::on_pingpanel_mousemove(wxMouseEvent& event)
{
	int w, h;
	PingPanel->GetClientSize(&w, &h);

	active_index_ = (w - event.GetX() + 1) / BLOCK_W;
	PingPanel->Refresh();

	scoped_lock l(pings_mutex_);

	int index = active_index_;
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
	active_index_ = -1;
	PingPanel->Refresh();
}
