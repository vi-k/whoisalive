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
	, states_z_(7)
	, new_states_z_(states_z_)
	, states_z_step_(0)
	, pings_socket_(io_service_)
	, pings_(1000)
	, pings_active_index_(-1)
	, states_hash_(0)
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
	StatePanel->Connect(ID_STATEPANEL,wxEVT_LEFT_UP,(wxObjectEventFunction)&wx_Ping::OnStatePanelLeftUp,0,this);
	StatePanel->Connect(ID_STATEPANEL,wxEVT_RIGHT_DOWN,(wxObjectEventFunction)&wx_Ping::OnStatePanelRightDown,0,this);
	StatePanel->Connect(ID_STATEPANEL,wxEVT_RIGHT_UP,(wxObjectEventFunction)&wx_Ping::OnStatePanelLeftUp,0,this);
	StatePanel->Connect(ID_STATEPANEL,wxEVT_MOTION,(wxObjectEventFunction)&wx_Ping::OnStatePanelMouseMove,0,this);
	StatePanel->Connect(ID_STATEPANEL,wxEVT_MOUSEWHEEL,(wxObjectEventFunction)&wx_Ping::OnStatePanelMouseWheel,0,this);
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
		/* Изменяем масштаб, при необходимости, сдвигаем график */
		if (states_z_step_)
		{
			unique_lock<shared_mutex> l(states_params_mutex_);

			int w, h;
			StatePanel->GetClientSize(&w, &h);
			wxDouble width = (wxDouble)w;

			/* Запоминаем состояние графика */
			posix_time::ptime cursor_time = states_cursor_time();
			posix_time::ptime old_start_time = states_start_time();
			posix_time::time_duration old_resolution = states_resolution(states_z_);
			wxDouble old_pos = time_to_x(cursor_time,
				old_start_time, old_resolution, width);
			
			/* Меняем масштаб */
			states_z_ += (new_states_z_ - states_z_) / states_z_step_;

			/* Плавно смещаем курсор к центру */
			posix_time::ptime new_start_time = states_start_time_.is_special()
				? old_start_time : states_start_time_;
			posix_time::time_duration new_resolution = states_resolution(states_z_);
			wxDouble new_pos = time_to_x(cursor_time,
				new_start_time, new_resolution, width);
			wxDouble delta = (width / 2.0 - old_pos) / states_z_step_;

			/* new_pos - точка, куда бы переместился курсор при увеличении.
				Здесь сначала возвращаем курсор на место, а затем только
				смещаем его к центру */
			new_start_time = old_start_time - my::time::mul(
				new_resolution, old_pos - new_pos + delta);
			
			--states_z_step_;

			states_start_time_
				= new_start_time > my::time::utc_now() - new_resolution
					? posix_time::not_a_date_time : new_start_time;
		}

		states_paint();
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
					unique_lock<shared_mutex> l(states_list_mutex_);
					states_[utc_time] = state;
				}
			} /* while (true) */

			states_paint();

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
	/* Время самой правой точки панели (немного округлённое!) */
	return states_start_time_.is_special() ? my::time::utc_now() : states_start_time_;
}

posix_time::ptime wx_Ping::states_cursor_time()
{
	/* Время под курсором */
	int w, h;
	StatePanel->GetClientSize(&w, &h);

	if (states_cursor_time_.is_special())
		states_cursor_time_ = states_start_time()
			- states_resolution(states_z_) * (w / 2);

	return states_cursor_time_;
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

/* Вычисляем размеры панели и подгоняем bitmap'у под эти размеры */
void wx_Ping::prepare_buffer(wxWindow *win, wxBitmap *bmp,
	wxDouble *pw, wxDouble *ph)
{
	int iw, ih;
	win->GetClientSize(&iw, &ih);
	*pw = (wxDouble)iw;
	*ph = (wxDouble)ih;

	if (bmp->GetWidth() != iw || bmp->GetHeight() != ih)
		bmp->Create(iw, ih);
}

/* Прорисовка фона для состояний */
void wx_Ping::states_paint_background()
{
	static const wxDouble ok_y = 8.0;
	static const wxDouble warn_y = ok_y + 12.0;
	static const wxDouble fail_y = warn_y + 12.0;

	static const long grids_for_z[][3] =
	{
		/* 0 */ { 60, 60, 10},
		/* 1 */ { 60, 60, 10},
		/* 2 */ { 10*60, 2*60, 60},
		/* 3 */ { 10*60, 10*60, 60},
		/* 4 */ { 10*60, 10*60, 60},
		/* 5 */ { 1*3600, 3600/2, 10*60},
		/* 6 */ { 1*3600, 1*3600, 10*60},
		/* 7 */ { 1*3600, 1*3600, 10*60},
		/* 8 */ { 24*3600, 3*3600, 1*3600},
		/* 9 */ { 24*3600, 6*3600, 1*3600},
		/* 10 */ { 24*3600, 24*3600, 1*3600},
		/* 11 */ { 24*3600, 24*3600, 12*3600},
		/* 12 */ { 0, 0, 24*3600}, /* неделя */
		/* 13 */ { 0, 0, 24*3600},
		/* 14 */ { 0, 0, 24*3600},
		/* 15 */ { 0, 0, 24*3600}, /* месяц */
		/* 16 */ { 0, 0, 24*3600},
		/* 17 */ { 0, 0, 24*3600}, /* год */
		/* 18 */ { 0, 0, 24*3600},
		/* 19 */ { 0, 0, 24*3600}
	};

	wxDouble w, h;
	prepare_buffer(StatePanel, &states_background_, &w, &h);

	wxMemoryDC dc(states_background_);
	scoped_ptr<wxGraphicsContext> gc( wxGraphicsContext::Create(dc) );

	posix_time::ptime start_time = my::time::utc_to_local(states_start_time());
	posix_time::time_duration resolution = states_resolution(states_z_);
	gregorian::date cur_date = posix_time::microsec_clock::local_time().date();

	/* Очищаем. Чёрный фон - для текущего времени, серый - если сдвинули */
	{
		shared_lock<shared_mutex> l(states_params_mutex_);

		gc->SetBrush( states_start_time_.is_special() ?
			*wxBLACK_BRUSH : wxBrush(wxColour(48, 48, 48)) );
		gc->DrawRectangle(0, 0, w, h);
	}

	gc->SetPen( wxPen( wxColour(64, 64, 64) ) );
	gc->StrokeLine(0, ok_y, w, ok_y);
	gc->StrokeLine(0, warn_y, w, warn_y);
	gc->StrokeLine(0, fail_y, w, fail_y);

	wxFont font(6, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);

	/* Рисуем сетку дважды для плавного перехода между z */
	for (int i = 0; i < 2; ++i)
	{
		int iz = (int)states_z_;
		double za = states_z_ - iz;
		int z;

		if (i == 0)
			z = iz, za = 1.0 - za;
		else
			z = iz + 1;

		unsigned char zac = (unsigned char)(255.0 * za);

		if (z > 19)
			break;

		posix_time::time_duration grid_res[3];
		grid_res[0] = posix_time::seconds( grids_for_z[z][0] );
		grid_res[1] = posix_time::seconds( grids_for_z[z][1] );
		grid_res[2] = posix_time::seconds( grids_for_z[z][2] );

		posix_time::ptime grid_time
			= my::time::floor(start_time, grid_res[2]);

		wxDouble x;

		do /* while (x >= 0.0) */
		{
			gregorian::date date = grid_time.date();
			x = time_to_x(grid_time, start_time, resolution, w);

			/* Яркие линии */
			if ( z <= 11 && my::time::floor(grid_time, grid_res[0]) == grid_time
				|| z >= 12 && z <= 14 && date.day_of_week() == 1
				|| z >= 15 && z <= 16 && date.day() == 1
				|| z >= 17 && date.day_of_year() == 1 )
			{
				wxDouble ext = 4.0 * za;
				gc->SetPen( wxPen( wxColour(192, 192, 192, zac) ) );
				gc->StrokeLine(x, ok_y - ext, x, fail_y + ext);
			}
			else if (z <= 14 || z >= 17 && date.day() == 1)
			{
				gc->SetPen( wxPen( wxColour(64, 64, 64, zac) ) );
				gc->StrokeLine(x, ok_y, x, fail_y);
			}

			/* Выводим подписи */
			if ( z <= 11 && my::time::floor(grid_time, grid_res[1]) == grid_time
				|| z >= 12 && z <= 14 && date.day_of_week() == 1
				|| z >= 15 && date.day() == 1 &&
					(z <= 16 || z == 17 && (date.month() & 1) == 1
					|| z == 18 && (date.month() == 1 || date.month() == 7)
					|| z == 19 && date.month() == 1) )
			{
				gc->SetFont( font, wxColour(192, 192, 192, zac) );

				wxDouble str_w, str_h, str_x, str_y, d;
				wstring str;

				str_y = fail_y + 5.0;
				str = my::time::to_wstring(grid_time, L"%H:%M:%S");
				gc->GetTextExtent(str, &str_w, &str_h, &d, 0);
				str_x = x - str_w / 2.0;

				if (i == 0 && z == 9)
					str_y -= (str_h - 2.0) * (1.0 - za);

				if (z < 10)
				{
					gc->DrawText(str, str_x, str_y);
					str_y += str_h - 2.0;
				}

				if (i == 1 && z == 10)
					str_y += (str_h - 2.0) * (1.0 - za);

				if (z >= 10 || date != cur_date)
				{
					str = my::time::to_wstring(grid_time, L"%d.%m.%Y");
					gc->GetTextExtent(str, &str_w, &str_h, &d, 0);
					str_x = x - str_w / 2.0;
					gc->DrawText(str, str_x, str_y);
				}
			}

			grid_time -= grid_res[2];

		} while (x >= 0.0);

	} /* for (int i = 0; i < 2; ++i) */
}

/* Прорисовка состояний */
void wx_Ping::states_paint()
{
	static const wxDouble ok_y = 8.0;
	static const wxDouble warn_y = ok_y + 12.0;
	static const wxDouble fail_y = warn_y + 12.0;

	unique_lock<mutex> l(states_bitmap_mutex_);

	/* Готовим буфера для отрисовки */
	wxDouble w, h;
	prepare_buffer(StatePanel, &states_bitmap_, &w, &h);

	wxMemoryDC dc(states_bitmap_);
	scoped_ptr<wxGraphicsContext> gc( wxGraphicsContext::Create(dc) );

	posix_time::ptime start_time = states_start_time();
	posix_time::time_duration resolution = states_resolution(states_z_);

	/* Узнаём, изменился ли фон */
	size_t hash = 0;
	boost::hash_combine(hash, boost::hash_value(
		my::time::floor(start_time, resolution / 3) ));
	boost::hash_combine(hash, boost::hash_value(states_z_));
	boost::hash_combine(hash, boost::hash_value(w));
	boost::hash_combine(hash, boost::hash_value(h));

	/* Если фон изменился, перерисовываем его */
	if (hash != states_hash_)
	{
		states_paint_background();
		states_hash_ = hash;
	}

	gc->DrawBitmap(states_background_, 0, 0, w, h);


	/* Рисуем состояния */
	{
		shared_lock<shared_mutex> l(states_list_mutex_);

		states_list::reverse_iterator iter = states_.rbegin();
		for (; iter != states_.rend() && iter->first > start_time; ++iter);

		wxDouble prev_x = w - 1.0;

		while (iter != states_.rend())
		{
			pinger::host_state state = iter->second;

			wxDouble x = time_to_x(iter->first, start_time, resolution, w);
			wxColour colors[4];
			wxDouble y = 0.0;

			unsigned char alpha = state.acknowledged() ?
				255 : (unsigned char)(255 * server_.flash_alpha());

			switch (state.state())
			{
				case pinger::host_state::ok:
					colors[0].Set(0, 255, 0, alpha);
					colors[1].Set(0, 255, 0, alpha / 2);
					y = ok_y;
					break;

				case pinger::host_state::warn:
					colors[0].Set(255, 255, 0, alpha);
					colors[1].Set(255, 255, 0, alpha / 2);
					y = warn_y;
					break;

				case pinger::host_state::fail:
					colors[0].Set(255, 0, 0, alpha);
					colors[1].Set(255, 0, 0, alpha / 2);
					y = fail_y;
					break;
			}

			gc->SetPen( wxPen(colors[1], 3) );
			gc->StrokeLine(x, y, prev_x, y);

			gc->SetPen( wxPen(colors[0], 1) );
			gc->StrokeLine(x, y, prev_x, y);

			if (x < 0.0)
				break;

			prev_x = x;
			++iter;
		}
	} /* shared_lock<shared_mutex> l(states_list_mutex_) */

	if (!states_cursor_time_.is_special())
	{
		wxDouble x = time_to_x(states_cursor_time_, start_time, resolution, w);
		gc->SetPen( wxColour(255, 128, 0) );
		gc->StrokeLine(x, 0.0, x, h);
	}

	scoped_ptr<wxGraphicsContext> gc_panel( wxGraphicsContext::Create(StatePanel) );
	gc_panel->DrawBitmap(states_bitmap_, 0, 0, w, h);
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

void wx_Ping::OnStatePanelLeftDown(wxMouseEvent& event)
{
	posix_time::ptime cursor_time = states_x_to_time( (wxDouble)event.GetX() );
	posix_time::time_duration dist = cursor_time - states_cursor_time_;
	
	if (dist.is_negative())
		dist = -dist;

	if (dist < states_resolution(states_z_))
		states_cursor_time_ = posix_time::not_a_date_time;
	else
		states_cursor_time_ = cursor_time;

	OnStatePanelRightDown(event);
}

void wx_Ping::OnStatePanelRightDown(wxMouseEvent& event)
{
	states_move_x_ = event.GetX();
	states_move_time_ = states_start_time();
	StatePanel->CaptureMouse();
}

void wx_Ping::OnStatePanelMouseMove(wxMouseEvent& event)
{
	if (StatePanel->HasCapture())
	{
		unique_lock<shared_mutex> l(states_params_mutex_);

		states_start_time_ = states_move_time_
			+ states_resolution(states_z_) * (states_move_x_ - event.GetX());

		if (states_start_time_ >= my::time::utc_now())
			states_start_time_ = posix_time::not_a_date_time;
	}

	posix_time::ptime mouse_time = states_x_to_time( (wxDouble)event.GetX() );

	StatePanel->SetToolTip( my::time::to_wstring(
		my::time::utc_to_local(mouse_time), L"%H:%M:%S\n%d-%m-%Y" ) );

#if 0

	wostringstream out;
	bool show_tool_tip = true;

	{
		shared_lock<shared_mutex> l(states_list_mutex_);

		pinger::host_state state = get_state_by_offset(states_active_index_);

		if (state == pinger::host_state::unknown)
			show_tool_tip = false;
		else
			out << state.state() << endl
				<< state.acknowledged() << endl
				<< my::time::to_wstring(state.time(), L"%Y-%m-%d %H:%M:%S");

	} /* shared_lock<shared_mutex> l(states_list_mutex_) */

	StatePanel->SetToolTip(out.str());
#endif
}

void wx_Ping::OnStatePanelLeftUp(wxMouseEvent& event)
{
	if (StatePanel->HasCapture())
		StatePanel->ReleaseMouse();
}

void wx_Ping::OnStatePanelMouseWheel(wxMouseEvent& event)
{
	double z = new_states_z_ - event.GetWheelRotation() / event.GetWheelDelta();

	if (z >= 0 && z <= 19)
	{
		new_states_z_ = z;
		states_z_step_ = 4 * server_.def_anim_steps();
	}
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

void wx_Ping::OnPingPanelMouseLeave(wxMouseEvent& event)
{
	pings_active_index_ = -1;
	PingPanel->Refresh();
}
