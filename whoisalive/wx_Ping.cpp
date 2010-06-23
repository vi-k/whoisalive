#include "wx_Ping.h"
#include "handle_exception.h"

#include "../common/my_exception.h"
#include "../common/my_str.h"
#include "../common/my_num.h"
#include "../common/my_utf8.h"

#include <sstream>
#include <istream>
#include <algorithm>
using namespace std;

#include <boost/bind.hpp>
#include <boost/system/system_error.hpp>

//#define wxUSE_GRAPHICS_CONTEXT 1
#include <wx/graphics.h>
#include <wx/app.h>
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
const long wx_Ping::ID_STATICTEXT5 = wxNewId();
const long wx_Ping::ID_STATEPANEL = wxNewId();
const long wx_Ping::ID_STATICTEXT1 = wxNewId();
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
	: server_(server)
	, object_(object)
	, io_service_()
	, anim_handler_index_(0)
	, states_socket_(io_service_)
	, states_()
	, states_resolution_(posix_time::milliseconds(1000))
	, states_active_index_(-1)
	, pings_socket_(io_service_)
	, pings_(1000)
	, pings_active_index_(-1)
{
	wxWindowID id = -1;

	//(*Initialize(wx_Ping)
	wxBoxSizer* BoxSizer2;
	wxFlexGridSizer* FlexGridSizer1;

	Create(parent, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE|wxFRAME_TOOL_WINDOW|wxFRAME_FLOAT_ON_PARENT, _T("wxID_ANY"));
	SetClientSize(wxSize(544,333));
	SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
	FlexGridSizer1 = new wxFlexGridSizer(6, 1, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	FlexGridSizer1->AddGrowableRow(2);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT5, _("Изменение состояний"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT5"));
	wxFont StaticText1Font(8,wxDEFAULT,wxFONTSTYLE_NORMAL,wxNORMAL,false,wxEmptyString,wxFONTENCODING_DEFAULT);
	StaticText1->SetFont(StaticText1Font);
	FlexGridSizer1->Add(StaticText1, 1, wxTOP|wxLEFT|wxRIGHT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StatePanel = new wxPanel(this, ID_STATEPANEL, wxDefaultPosition, wxSize(400,61), wxTAB_TRAVERSAL, _T("ID_STATEPANEL"));
	FlexGridSizer1->Add(StatePanel, 1, wxLEFT|wxRIGHT|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT1, _("Пинги"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	wxFont StaticText2Font(8,wxDEFAULT,wxFONTSTYLE_NORMAL,wxNORMAL,false,wxEmptyString,wxFONTENCODING_DEFAULT);
	StaticText2->SetFont(StaticText2Font);
	FlexGridSizer1->Add(StaticText2, 1, wxTOP|wxLEFT|wxRIGHT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	PingPanel = new wxPanel(this, ID_PINGPANEL, wxDefaultPosition, wxSize(400,61), wxTAB_TRAVERSAL, _T("ID_PINGPANEL"));
	FlexGridSizer1->Add(PingPanel, 1, wxLEFT|wxRIGHT|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
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
	StatePanel->Connect(ID_STATEPANEL,wxEVT_LEFT_DOWN,(wxObjectEventFunction)&wx_Ping::OnStatePanelLeftDown,0,this);
	StatePanel->Connect(ID_STATEPANEL,wxEVT_RIGHT_DOWN,(wxObjectEventFunction)&wx_Ping::OnStatePanelRightDown,0,this);
	StatePanel->Connect(ID_STATEPANEL,wxEVT_MOTION,(wxObjectEventFunction)&wx_Ping::OnStatePanelMouseMove,0,this);
	StatePanel->Connect(ID_STATEPANEL,wxEVT_LEAVE_WINDOW,(wxObjectEventFunction)&wx_Ping::OnStatePanelMouseLeave,0,this);
	PingPanel->Connect(ID_PINGPANEL,wxEVT_PAINT,(wxObjectEventFunction)&wx_Ping::OnPingPanelPaint,0,this);
	PingPanel->Connect(ID_PINGPANEL,wxEVT_ERASE_BACKGROUND,(wxObjectEventFunction)&wx_Ping::OnPanelsEraseBackground,0,this);
	PingPanel->Connect(ID_PINGPANEL,wxEVT_MOTION,(wxObjectEventFunction)&wx_Ping::OnPingPanelMouseMove,0,this);
	PingPanel->Connect(ID_PINGPANEL,wxEVT_LEAVE_WINDOW,(wxObjectEventFunction)&wx_Ping::OnPingPanelMouseLeave,0,this);
	//*)

	if (object->hosts().empty())
		throw my::exception(L"Объект не содержит ни один адрес");

	if (object->hosts().size() > 1)
		throw my::exception(L"Объект содержит больше одного адреса");

	wstring host = object->hosts().front();
	wstring name = object->name() + L" / " + host;
	SetLabel(name);

	/* Асинхронное чтение состояний для хоста */
	server_.get_header(states_socket_, states_reply_,
		L"/pinger/state.log?address=" + host);

	asio::async_read_until(
		states_socket_, states_reply_.buf_, "\r\n",
		boost::bind(&wx_Ping::states_handle_read, this,
            new_worker("states_async_read", boost::bind(
            	&tcp::socket::close, &states_socket_)),
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred) );

	/* Асинхронное чтение пингов хоста */
	server_.get_header(pings_socket_, pings_reply_,
		L"/pinger/ping.log?address=" + host);

	asio::async_read_until(
		pings_socket_, pings_reply_.buf_, "\r\n",
		boost::bind(&wx_Ping::pings_handle_read, this,
            new_worker("pings_async_read", boost::bind(
            	&tcp::socket::close, &pings_socket_)),
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred) );

	/* Поток для io_service_. Используем отдельный поток
		для обеспечения надёжности. При сбое, просто завершаем
		работу и закрываем окно */
	boost::thread( boost::bind(
		&wx_Ping::io_thread_proc, this, new_worker("io_thread") ) );

	/* Анимация */
	anim_handler_index_ = server_.add_anim_handler(
		boost::bind(&wx_Ping::anim_handler, this, new_worker("animator")) );

	Show();
}

wx_Ping::~wx_Ping()
{
	/* Оповещаем о завершении работы */
	lets_finish();

	/* "Увольняем" все ссылки на "работников" */
	/* ... */

	server_.remove_anim_handler(anim_handler_index_);

	/* Наши асинхронные "работники" активно задействуют контролы формы,
		а это требует реакции основного (т.е. данного) потока,
		и поэтому его никак нельзя останавливать на wait_for_workers() */
	while (!check_for_finish() && wxTheApp->Pending())
    {
    	#if 0
    	vector<std::string> v;
    	workers_state(v);
    	#endif

		wxTheApp->Dispatch();
    }

	/* Обязательно ждём, т.к. даже после check_for_finish()
		есть вероятность запуска start_animate() */
	wait_for_finish();

	//(*Destroy(wx_Ping)
	//*)
}

void wx_Ping::io_thread_proc(my::worker::ptr this_worker)
{
	try
	{
		io_service_.run(); /* Работаем */
	}
	catch (std::exception &e)
	{
		handle_exception(&e, L"in wx_Ping", L"Ошибка");
		Destroy();
    }
	catch(...)
	{
		handle_exception(0, L"in wx_Ping", L"Ошибка");
		Destroy();
	}
}

void wx_Ping::anim_handler(my::worker::ptr this_worker)
{
	if (!finish())
		states_repaint();
}

void wx_Ping::Open(wxWindow* parent, who::server &server, who::object *object)
{
	try
	{
		new wx_Ping(parent, server, object); /* Удалит себя сам */
	}
	catch(my::exception &e)
	{
		wxMessageBox(e.message(), L"Ошибка", wxOK | wxICON_ERROR, parent);
	}
}

/* Асинхронное чтение состояний */
void wx_Ping::states_handle_read( my::worker::ptr worker,
	const boost::system::error_code& error, size_t bytes_transferred )
{
	if (finish())
		return;

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
				boost::bind(&wx_Ping::states_handle_read, this, worker,
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
					unique_lock<shared_mutex> l(states_mutex_);
					states_[utc_time] = state;
					states_active_index_ = -1;
				}
			} /* while (ss) */

			states_repaint();

			asio::async_read_until(
				states_socket_, states_reply_.buf_, "\r\n",
				boost::bind(&wx_Ping::states_handle_read, this, worker,
					boost::asio::placeholders::error,
					boost::asio::placeholders::bytes_transferred) );

		} /* else // if (states_reply_.body == "START_ARCHIVE\r\n") */
	}
	else
	{
		boost::system::system_error se(error);
		wstring str = my::str::to_wstring( se.what() );

		wxMessageBox(str, L"Ошибка чтения данных",
			wxOK | wxICON_ERROR, this);
	}
}

posix_time::ptime wx_Ping::states_start_time()
{
	return my::time::ceil(
		states_start_.is_special() ?
			posix_time::microsec_clock::universal_time() : states_start_,
		states_resolution_);
}

/* Прорисовка состояний */
void wx_Ping::states_repaint()
{
	{
		unique_lock<mutex> l(states_bitmap_mutex_);

		int w, h;
		StatePanel->GetClientSize(&w, &h);

		if (states_bitmap_.GetWidth() != w || states_bitmap_.GetHeight() != h)
			states_bitmap_.Create(w, h);

		wxMemoryDC dc(states_bitmap_);
		scoped_ptr<wxGraphicsContext> gc( wxGraphicsContext::Create(dc) );

		int cy = h / 2;
		int ok_sz = 1;
		int warn_sz = cy / 3 * 1;
		int fail_sz = cy / 3 * 2;

		posix_time::ptime start = states_start_time();

		/* Стираем */
		gc->SetBrush(*wxBLACK_BRUSH);
		gc->DrawRectangle(0, 0, w, h);

		/* Рисуем границы */
		gc->SetPen(*wxGREY_PEN);
		gc->StrokeLine(0, cy, w, cy);

#if 0
		/* Рисуем сетку */
		{
			static const long grid[]
				= {1, 5, 15, 60, 5*60, 15*60, 60*60, 3*60*60, 12*60*60, 24*60*60};
			static const unsigned char levels[]
				= {96, 64, 32};

			long min_grid = (states_resolution_ * 5).total_seconds();

			int min_grid_index;
			for (int i = 0; i < sizeof(grid)/sizeof(*grid); ++i)
			{
				min_grid_index = i;
				if (grid[i] >= min_grid)
					break;
			}

			int max_grid_index = min(
				sizeof(levels)/sizeof(*levels) + min_grid_index,
				sizeof(grid)/sizeof(*grid));

			gc->SetFont( wxFont(6, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL,
				wxFONTWEIGHT_NORMAL), wxColour(255, 255, 255));

			for (int i = min_grid_index; i < max_grid_index; ++i)
			{
				unsigned char l = levels[ max_grid_index - 1 - i ];

				posix_time::time_duration grid_resolution = posix_time::seconds(grid[i]);
				posix_time::ptime grid_start = my::time::floor(start, grid_resolution);

				int grid_size = (int)(grid_resolution / states_resolution_);
				int x = w - (int)((start - grid_start) / states_resolution_) - 1;

				gc->SetPen( wxPen(wxColour(l, l, l)) );

				while (x >= 0)
				{
					gc->StrokeLine(x, 0, x, h);

					if (i == max_grid_index - 1)
					{
						wstring str = my::time::to_wstring(grid_start, L"%H:%M:%S");
						wxDouble tw, th, descent, tx, ty;
						gc->GetTextExtent(str, &tw, &th, &descent, 0);
						tx = x - tw / 2;
						ty = h - th;
						gc->DrawText(str, tx, ty);
					}

					grid_start -= grid_resolution;
					x -= grid_size;
				}
			}
		}

#endif

		{
			shared_lock<shared_mutex> l(states_mutex_);

			states_list::reverse_iterator iter = states_.rbegin();
			for (; iter != states_.rend() && iter->first > start; ++iter);

			int prev_x = w - 1;

			while (iter != states_.rend())
			{
				pinger::host_state state = iter->second;

				int x = w - (start - iter->first) / states_resolution_ - 1;
				wxColour colors[4];
				int sz = 0;

				unsigned char alpha = state.acknowledged() ?
					255 : (unsigned char)(255 * server_.flash_alpha());

				switch (state.state())
				{
					case pinger::host_state::ok:
						colors[0].Set(0, 255, 0, alpha);
						colors[1].Set(0, 192, 0, alpha);
						sz = ok_sz;
						break;

					case pinger::host_state::warn:
						colors[0].Set(255, 255, 0, alpha);
						colors[1].Set(192, 192, 0, alpha);
						sz = warn_sz;
						break;

					case pinger::host_state::fail:
						colors[0].Set(255, 0, 0, alpha);
						colors[1].Set(192, 0, 0, alpha);
						sz = fail_sz;
						break;
				}

				gc->SetPen( wxPen(colors[0]) );
				gc->SetBrush( wxBrush(colors[1]) );
				gc->DrawRectangle(x, cy - sz, prev_x - x - 1, 2 * sz);

				if (x < 0)
					break;

				prev_x = x;
				++iter;
			}
		} /* shared_lock<shared_mutex> l(states_mutex_) */

		/*-
		gc1->SetPen(*wxBLACK_PEN);
		gc1->SetBrush(*wxTRANSPARENT_BRUSH);
		gc1->DrawRectangle(0, 0, w, h);
		-*/

		scoped_ptr<wxGraphicsContext> gc2( wxGraphicsContext::Create(StatePanel) );
		gc2->DrawBitmap(states_bitmap_, 0, 0, w, h);
	} /* unique_lock<mutex> l(states_bitmap_mutex_) */
}

void wx_Ping::OnStatePanelPaint(wxPaintEvent& event)
{
	unique_lock<mutex> l(states_bitmap_mutex_);

	if (states_bitmap_.IsOk())
	{
		wxPaintDC dc(StatePanel);
		scoped_ptr<wxGraphicsContext> gc( wxGraphicsContext::Create(dc) );

		int w, h;
		StatePanel->GetClientSize(&w, &h);

		gc->DrawBitmap(states_bitmap_, 0, 0, w, h);

		if (states_active_index_ >= 0)
		{
			gc->SetPen(*wxWHITE_PEN);
			int x = w - states_active_index_ - 1;

			gc->StrokeLine(x, 0, x, h);
		}
	}

	event.Skip(false);
}

/* Асинхронное чтение пингов */
void wx_Ping::pings_handle_read( my::worker::ptr worker,
	const boost::system::error_code& error, size_t bytes_transferred )
{
	if (finish())
		return;

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
				boost::bind(&wx_Ping::pings_handle_read, this, worker,
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
						unique_lock<shared_mutex> l(pings_mutex_);
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
				boost::bind(&wx_Ping::pings_handle_read, this, worker,
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

void wx_Ping::pings_repaint()
{
	{
		unique_lock<mutex> l(pings_bitmap_mutex_);

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

		{
			shared_lock<shared_mutex> l(pings_mutex_);

			int prev_y = -1;
			int index = 0;

			for (pings_list::iterator iter = pings_.begin();
				iter != pings_.end(); iter++)
			{
				++index;

				pinger::ping_result ping = iter->value();

				if (index == 1)
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
		} /* shared_lock<shared_mutex> l(pings_mutex_) */
	} /* unique_lock<mutex> l(pings_bitmap_mutex_) */

	PingPanel->Refresh();
}

void wx_Ping::OnPingPanelPaint(wxPaintEvent& event)
{
	wxPaintDC dc(PingPanel);

	unique_lock<mutex> l(pings_bitmap_mutex_);

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

void wx_Ping::OnPanelsEraseBackground(wxEraseEvent& event)
{
	event.Skip(false);
}

pinger::host_state wx_Ping::get_state_by_offset(int offset)
{
	posix_time::ptime start = states_start_time()
		- states_resolution_ * offset;

	states_list::reverse_iterator iter = states_.rbegin();
	for (; iter != states_.rend() && iter->first > start; ++iter);

	if (iter == states_.rend())
		return pinger::host_state();

	return iter->second;
}

void wx_Ping::OnStatePanelMouseMove(wxMouseEvent& event)
{
	int w, h;
	StatePanel->GetClientSize(&w, &h);

	states_active_index_ = w - 1 - event.GetX();
	StatePanel->Refresh();

	wostringstream out;
	bool show_tool_tip = true;

	{
		shared_lock<shared_mutex> l(states_mutex_);

		pinger::host_state state = get_state_by_offset(states_active_index_);

		if (state == pinger::host_state::unknown)
			show_tool_tip = false;
		else
			out << state.state() << endl
				<< state.acknowledged() << endl
				<< my::time::to_wstring(state.time(), L"%Y-%m-%d %H:%M:%S");

	} /* shared_lock<shared_mutex> l(states_mutex_) */

	StatePanel->SetToolTip(out.str());
}

void wx_Ping::OnPingPanelMouseMove(wxMouseEvent& event)
{
	int w, h;
	PingPanel->GetClientSize(&w, &h);

	pings_active_index_ = (w - event.GetX() + 1) / BLOCK_W;
	PingPanel->Refresh();

	wostringstream out;
	bool show_tool_tip = true;

	{
		shared_lock<shared_mutex> l(pings_mutex_);

		/* Ищем ping, на который указывает pings_active_index_.
			advance() не используем, т.к. pings_active_index_ может
			указывать на несуществующий ping */
		pings_list::iterator iter = pings_.begin();
		int index = pings_active_index_;
		while (iter != pings_.end() && index-- != 0)
			iter++;

		if (iter == pings_.end())
			show_tool_tip = false;
		else
			out << iter->value().state() << endl
				<< my::time::to_wstring(iter->value().time(), L"%Y-%m-%d %H:%M:%S") << endl
				<< L"icmp_seq=" << iter->key() << endl
				<< L"time=" << iter->value().duration().total_milliseconds() << L" ms";

	} /* shared_lock<shared_mutex> l(pings_mutex_) */

	PingPanel->SetToolTip(out.str());
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

void wx_Ping::OnStatePanelLeftDown(wxMouseEvent& event)
{
	states_resolution_ *= 2;
	StatePanel->Refresh();
}

void wx_Ping::OnStatePanelRightDown(wxMouseEvent& event)
{
	states_resolution_ /= 2;
	StatePanel->Refresh();
}
