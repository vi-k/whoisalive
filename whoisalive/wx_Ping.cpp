#include "stdafx.h"

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
	, states_z_(2)
	, new_states_z_(states_z_)
	, states_z_step_(0)
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
	{
		if (states_z_step_)
		{
			states_z_ += (new_states_z_ - states_z_) / states_z_step_;
			--states_z_step_;
		}

		states_repaint();
	}
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
	return states_start_time_.is_special() ?
		posix_time::microsec_clock::universal_time() : states_start_time_;
	
	/*-
	return my::time::ceil(
		states_start_time_.is_special() ?
			posix_time::microsec_clock::universal_time() : states_start_time_,
		states_resolution(states_z_));
	-*/
}

wxDouble wx_Ping::time_to_x(
	const posix_time::ptime &time,
	const posix_time::ptime &start_time,
	const posix_time::time_duration &resolution,
	wxDouble width)
{
	return width - 1.0 - my::time::div(start_time - time, resolution);
}

posix_time::ptime wx_Ping::x_to_time(
	wxDouble x,
	const posix_time::ptime &start_time,
	const posix_time::time_duration &resolution,
	wxDouble width)
{
	return start_time - my::time::mul(resolution, width - 1.0 - x);
}

wxDouble wx_Ping::states_time_to_x(const posix_time::ptime &time)
{
	int w, h;
	StatePanel->GetClientSize(&w, &h);

	return (wxDouble)w - 1.0
		- my::time::div(states_start_time() - time,
			states_resolution(states_z_));
}

posix_time::ptime wx_Ping::states_x_to_time(wxDouble x)
{
	int w, h;
	StatePanel->GetClientSize(&w, &h);

	return states_start_time()
		- my::time::mul(states_resolution(states_z_), (wxDouble)w - 1.0 - x);
}

/* Прорисовка состояний */
void wx_Ping::states_repaint()
{
	{
		unique_lock<mutex> l(states_bitmap_mutex_);

		wxDouble w, h;

		/* Вычисляем размеры панели и подгоняем bitmap'у под эти размеры */
		{
			int iw, ih;
			StatePanel->GetClientSize(&iw, &ih);
			w = (wxDouble)iw;
			h = (wxDouble)ih;

			if (states_bitmap_.GetWidth() != iw || states_bitmap_.GetHeight() != ih)
				states_bitmap_.Create(iw, ih);
		}

		wxMemoryDC dc(states_bitmap_);
		scoped_ptr<wxGraphicsContext> gc( wxGraphicsContext::Create(dc) );
		gc->SetAntialiasMode(wxANTIALIAS_DEFAULT);

		wxDouble ok_y = 8.0;
		wxDouble warn_y = ok_y + 12.0;
		wxDouble fail_y = warn_y + 12.0;

		posix_time::ptime start_time = states_start_time();
		posix_time::ptime local_start_time = my::time::utc_to_local(start_time);
		posix_time::time_duration states_res = states_resolution(states_z_);

		/* Стираем */
		gc->SetBrush(*wxBLACK_BRUSH);
		gc->DrawRectangle(0.0, 0.0, w, h);

		/* Рисуем сетку */
		{
			static const long grids[] =
			{
				1, 5, 10, 30, /* секунды */
				1*60, 5*60, 10*60, 30*60, /* минуты */
				1*3600, 6*3600, 12*3600, /* часы */
				24*3600, /* дни */
				7*24*3600, /* недели */
				30*24*3600, /* месяцы */
				365*24*3600  /* годы */
			};

			gc->SetFont( wxFont(6, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL,
				wxFONTWEIGHT_NORMAL), wxColour(192, 192, 192));

			wxDouble caption_width;
			
			{
				wxDouble h, d;
				wstring str(L"00-00-0000");
				gc->GetTextExtent(str, &caption_width, &h, &d, 0);

				/*-
				if ( (double)states_caption_bitmap_.GetWidth() < caption_width
					|| (double)states_caption_bitmap_.GetHeight() < caption_height)
					states_bitmap_.Create(iw, ih);
				-*/
			}

			bool guides_painted = false;
			bool captions_painted = false;

			for (int i = 0; i < sizeof(grids)/sizeof(*grids); ++i)
			{
				posix_time::time_duration grid_resolution = posix_time::seconds(grids[i]);

				double cell_sz = my::time::div(grid_resolution, states_res);
				if (cell_sz < 5.0) continue;

				double alpha = cell_sz / 255.0 ;
				if (alpha > 1.0) alpha = 1.0;

				static const int max_a = 192;
				static const int min_a = 32;
				static const int guides_a = 64;

				unsigned char a = (unsigned char)(alpha * (max_a - min_a) + min_a);
				gc->SetPen( wxPen(wxColour(a, a, a)) );

				posix_time::ptime grid_time
					= my::time::floor(local_start_time, grid_resolution);

				/* Рисуем направляющие */
				if (!guides_painted && a >= guides_a)
				{
					guides_painted = true;
					gc->SetPen( wxColour(guides_a, guides_a, guides_a) );
					gc->StrokeLine(0, ok_y, w, ok_y);
					gc->StrokeLine(0, warn_y, w, warn_y);
					gc->StrokeLine(0, fail_y, w, fail_y);
				}

				wxDouble x;
				wxDouble ext = 4.0 * (a - min_a) / (max_a - min_a);

				bool captions_paint = false;
				int k;

				/* Выводим подписи */
				if (!captions_painted && cell_sz > caption_width * 0.75)
				{
					captions_paint = true;
					k = (cell_sz > caption_width * 1.5 ? 1 : 2);
				}

				do
				{
					x = time_to_x(grid_time, local_start_time, states_res, w);
					//gc->StrokeLine(x, ok_y - ext, x, fail_y + ext);

					/* Выводим подписи */
					if (captions_paint && (k == 1 || k == 2
						&& my::time::floor(grid_time, grid_resolution)
							== my::time::floor(grid_time, grid_resolution * 2)))
					{
						wxDouble str_w, str_h, str_x, str_y, d;

						str_y = fail_y + 5.0;

						wstring str = my::time::to_wstring(grid_time, L"%H:%M:%S");
						gc->GetTextExtent(str, &str_w, &str_h, &d, 0);
						str_x = x - str_w / 2.0;
						gc->DrawText(str, str_x, str_y);

						str_y += str_h - 2.0;

						str = my::time::to_wstring(grid_time, L"%d-%m-%Y");
						gc->GetTextExtent(str, &str_w, &str_h, &d, 0);
						str_x = x - str_w / 2.0;
						gc->DrawText(str, str_x, str_y);
					}

					grid_time -= grid_resolution;

				} while (x >= 0.0);

				if (captions_paint)
					captions_painted = true;
			}
		} /* Рисуем сетку */

		/* Рисуем состояния */
		{
			shared_lock<shared_mutex> l(states_mutex_);

			states_list::reverse_iterator iter = states_.rbegin();
			for (; iter != states_.rend() && iter->first > start_time; ++iter);

			wxDouble prev_x = w - 1.0;

			while (iter != states_.rend())
			{
				pinger::host_state state = iter->second;

				wxDouble x = time_to_x(iter->first, start_time, states_res, w);
				wxColour colors[4];
				wxDouble y = 0.0;

				unsigned char alpha = state.acknowledged() ?
					255 : (unsigned char)(255 * server_.flash_alpha());

				switch (state.state())
				{
					case pinger::host_state::ok:
						colors[0].Set(0, 255, 0, alpha);
						colors[1].Set(0, 160, 0, alpha);
						y = ok_y;
						break;

					case pinger::host_state::warn:
						colors[0].Set(255, 255, 0, alpha);
						colors[1].Set(160, 160, 0, alpha);
						y = warn_y;
						break;

					case pinger::host_state::fail:
						colors[0].Set(255, 0, 0, alpha);
						colors[1].Set(160, 0, 0, alpha);
						y = fail_y;
						break;
				}

				gc->SetPen( wxPen(colors[0]) );
				gc->SetBrush( wxBrush(colors[1]) );
				gc->StrokeLine(x, y, prev_x, y);

				if (x < 0.0) break;

				prev_x = x;
				++iter;
			}
		} /* shared_lock<shared_mutex> l(states_mutex_) */

		if (!states_cursor_time_.is_special())
		{
			wxDouble x = time_to_x(states_cursor_time_, start_time, states_res, w);
			gc->SetPen(*wxWHITE_PEN);
			gc->StrokeLine(x, 0.0, x, h);
		}

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
	posix_time::ptime start_time = states_start_time()
		- states_resolution(states_z_) * offset;

	states_list::reverse_iterator iter = states_.rbegin();
	for (; iter != states_.rend() && iter->first > start_time; ++iter);

	if (iter == states_.rend())
		return pinger::host_state();

	return iter->second;
}

void wx_Ping::OnStatePanelMouseMove(wxMouseEvent& event)
{
	states_cursor_time_ = states_x_to_time( (wxDouble)event.GetX() );

	StatePanel->SetToolTip( my::time::to_wstring(states_cursor_time_) );

#if 0

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
#endif
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
	states_cursor_time_ = posix_time::not_a_date_time;
}

void wx_Ping::OnPingPanelMouseLeave(wxMouseEvent& event)
{
	pings_active_index_ = -1;
	PingPanel->Refresh();
}

void wx_Ping::OnStatePanelLeftDown(wxMouseEvent& event)
{
	if (new_states_z_ > 0) {
		--new_states_z_;
		states_z_step_ = 4 * server_.def_anim_steps();
	}
}

void wx_Ping::OnStatePanelRightDown(wxMouseEvent& event)
{
	if (new_states_z_ < 20) {
		++new_states_z_;
		states_z_step_ = 4 * server_.def_anim_steps();
	}
}
