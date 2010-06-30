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
const long wx_Ping::ID_PINGTEXTCTRL = wxNewId();
//*)

BEGIN_EVENT_TABLE(wx_Ping,wxFrame)
	//(*EventTable(wx_Ping)
	//*)
END_EVENT_TABLE()

wx_Ping::wx_Ping(wxWindow* parent, who::server &server, who::object *object)
	: server_(server)
	, object_(object)
	, io_service_()
	, anim_handler_index_(0)
	, states_socket_(io_service_)
	, pings_socket_(io_service_)
{
	wxWindowID id = -1;

	//(*Initialize(wx_Ping)
	wxFlexGridSizer* FlexGridSizer1;

	Create(parent, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE|wxFRAME_TOOL_WINDOW|wxFRAME_FLOAT_ON_PARENT, _T("wxID_ANY"));
	SetClientSize(wxSize(544,333));
	SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
	FlexGridSizer1 = new wxFlexGridSizer(5, 1, 0, 0);
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
	PingTextCtrl = new wxTextCtrl(this, ID_PINGTEXTCTRL, wxEmptyString, wxDefaultPosition, wxSize(400,97), wxTE_AUTO_SCROLL|wxTE_MULTILINE|wxTE_READONLY|wxTE_RICH2|wxTE_NOHIDESEL, wxDefaultValidator, _T("ID_PINGTEXTCTRL"));
	PingTextCtrl->SetForegroundColour(wxColour(192,192,192));
	PingTextCtrl->SetBackgroundColour(wxColour(0,0,0));
	FlexGridSizer1->Add(PingTextCtrl, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->SetSizeHints(this);

	StatePanel->Connect(ID_STATEPANEL,wxEVT_ENTER_WINDOW,(wxObjectEventFunction)&wx_Ping::OnStatePanelMouseEnter,0,this);
	PingPanel->Connect(ID_PINGPANEL,wxEVT_ENTER_WINDOW,(wxObjectEventFunction)&wx_Ping::OnPingPanelMouseEnter,0,this);
	//*)

	if (object->hosts().empty())
		throw my::exception(L"Объект не содержит ни один адрес");

	if (object->hosts().size() > 1)
		throw my::exception(L"Объект содержит больше одного адреса");

	wstring host = object->hosts().front();
	wstring name = object->name() + L" / " + host;
	SetLabel(name);

	states_grid_.reset( new my::time_grid(StatePanel, true, 8, 32,
		boost::bind(&wx_Ping::states_on_before_paint, this, _1, _2, _3, _4, _5),
		boost::bind(&wx_Ping::states_on_after_paint, this, _1, _2, _3, _4, _5),
		server_.def_anim_steps(), 7, 0, 19) );

	pings_grid_.reset( new my::time_grid(PingPanel, false, 8, 32,
		boost::bind(&wx_Ping::pings_on_before_paint, this, _1, _2, _3, _4, _5),
		boost::bind(&wx_Ping::pings_on_after_paint, this, _1, _2, _3, _4, _5),
		server_.def_anim_steps(), 2, 0, 6) );

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
		states_grid_->paint();
		pings_grid_->paint();
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

			states_grid_->paint();

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

void wx_Ping::states_on_before_paint(wxGraphicsContext *gc,
	wxDouble width, wxDouble height, posix_time::ptime right_bound,
	posix_time::time_duration resolution)
{
	gc->StrokeLine(0.0, 20.0, width, 20.0);
}

void wx_Ping::states_on_after_paint(wxGraphicsContext *gc,
	wxDouble width, wxDouble height, posix_time::ptime right_bound,
	posix_time::time_duration resolution)
{
	static const wxDouble ok_y = 8.0;
	static const wxDouble warn_y = 20.0;
	static const wxDouble fail_y = 32.0;

	/* Рисуем состояния */
	shared_lock<shared_mutex> l(states_list_mutex_);

	states_list::reverse_iterator iter = states_.rbegin();
	for (; iter != states_.rend() && iter->first > right_bound; ++iter);

	wxDouble prev_x = width - 1.0;

	while (iter != states_.rend())
	{
		pinger::host_state state = iter->second;

		wxDouble x = my::time_grid::static_time_to_x(
			iter->first, right_bound, resolution, width);
		wxColour colors[2];
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
}

void wx_Ping::OnStatePanelMouseMove(wxMouseEvent& event)
{
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

				posix_time::ptime utc_time = ping.time();
				ping.set_time( my::time::utc_to_local(utc_time) );

				wostringstream out;

				wxTextAttr style;

				if (ping.state() == pinger::ping_result::unknown)
				{
					style = wxTextAttr(*wxLIGHT_GREY);
					out << L"unknown message\n";
				}
				else
				{
					{
						unique_lock<shared_mutex> l(pings_list_mutex_);
						pings_[utc_time] = ping;
					}

					switch (ping.state())
					{
						case pinger::ping_result::ok:
							style = wxTextAttr(*wxGREEN);
							out << my::time::to_wstring(ping.time(), L"[%H:%M:%S]")
								<< L' ' << ping.ipv4_hdr().header_length()
								<< L" bytes from " << ping.ipv4_hdr().source_address()
								<< L", icmp_seq=" << ping.sequence_number()
								<< L", ttl=" << ping.ipv4_hdr().time_to_live()
								<< L", time=" << ping.duration().total_milliseconds() << L" ms"
								<< L"\n";
							break;

						case pinger::ping_result::timeout:
							style = wxTextAttr(*wxRED);
							out << my::time::to_wstring(ping.time(), L"[%H:%M:%S]")
								<< L" timeout (icmp_seq=" << ping.sequence_number()
								<< L", time=" << ping.duration().total_milliseconds() << L" ms"
								<< L")\n";
							break;
					}
				}

				PingTextCtrl->SetDefaultStyle(style);
				*PingTextCtrl << out.str();

			} /* while (ss) */

			PingTextCtrl->Thaw();

			pings_grid_->paint();

			asio::async_read_until(
				pings_socket_, pings_reply_.buf_, "\r\n",
				boost::bind(&wx_Ping::pings_handle_read, this, worker,
					boost::asio::placeholders::error,
					boost::asio::placeholders::bytes_transferred) );

		} /* else // if (pings_reply_.body == "START_ARCHIVE\r\n") */
	}
	else
	{
		boost::system::system_error se(error);
		wstring str = my::str::to_wstring( se.what() );

		wxMessageBox(str, L"Ошибка чтения данных",
			wxOK | wxICON_ERROR, this);
	}
}

void wx_Ping::pings_on_before_paint(wxGraphicsContext *gc,
	wxDouble width, wxDouble height, posix_time::ptime right_bound,
	posix_time::time_duration resolution)
{
	//gc->StrokeLine(0.0, 14.0, width, 14.0);
	gc->StrokeLine(0.0, 20.0, width, 20.0);
	//gc->StrokeLine(0.0, 26.0, width, 26.0);
}

void wx_Ping::pings_on_after_paint(wxGraphicsContext *gc,
	wxDouble width, wxDouble height, posix_time::ptime right_bound,
	posix_time::time_duration resolution)
{
	static const wxDouble top = 8.0;
	static const wxDouble bottom = 32.0;

	/* Рисуем пинги */
	shared_lock<shared_mutex> l(pings_list_mutex_);

	pings_list::reverse_iterator iter = pings_.rbegin();

	if (iter != pings_.rend())
	{
		pings_list::reverse_iterator prev_iter = iter++;
		while (iter != pings_.rend() && iter->first > right_bound)
			++iter, ++prev_iter;
		iter = prev_iter;
	}

	wxDouble prev_x = width - 1.0;
	wxDouble prev_y = -1.0;

	while (iter != pings_.rend())
	{
		pinger::ping_result ping = iter->second;

		wxDouble x = my::time_grid::static_time_to_x(
			iter->first, right_bound, resolution, width);
		wxColour colors[2];

		switch (ping.state())
		{
			case pinger::ping_result::ok:
				colors[0].Set(0, 255, 0);
				colors[1].Set(0, 255, 0, 128);
				break;

			case pinger::ping_result::timeout:
				colors[0].Set(255, 0, 0);
				colors[1].Set(255, 0, 0, 128);
				break;
		}

		//if (ping.state() == pinger::ping_result::timeout)
		//	prev_y = -1.0;
		//else
		{
			wxDouble y = bottom - (bottom - top) * my::time::div(
				ping.duration(), posix_time::seconds(2));
			if (y < 0.0)
				y = 0.0;

			wxDouble x2 = prev_x;
			wxDouble y2 = prev_y;

			if (prev_y == -1.0)
			{
				x2 = x;
				y2 = y;
			}

			gc->SetPen( wxPen(colors[1], 3) );
			gc->StrokeLine(x2, y2, x, y);
			gc->SetPen( wxPen(colors[0], 1) );
			gc->StrokeLine(x2, y2, x, y);

			prev_y = y;

		} /* else // if (ping.state() == pinger::ping_result::timeout) */

		if (x < 0.0)
			break;

		prev_x = x;
		++iter;

	} /* while (iter != pings_.rend()) */
}

#if 0
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
#endif

void wx_Ping::OnPingPanelMouseMove(wxMouseEvent& event)
{
#if 0
	int w, h;
	PingPanel->GetClientSize(&w, &h);

	pings_active_index_ = (w - event.GetX() + 1) / BLOCK_W;
	PingPanel->Refresh();

	wostringstream out;
	bool show_tool_tip = true;

	{
		shared_lock<shared_mutex> l(pings_list_mutex_);

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

	} /* shared_lock<shared_mutex> l(pings_list_mutex_) */

	PingPanel->SetToolTip(out.str());
#endif
}

void wx_Ping::OnStatePanelMouseEnter(wxMouseEvent& event)
{
	StatePanel->SetFocus();
}

void wx_Ping::OnPingPanelMouseEnter(wxMouseEvent& event)
{
	PingPanel->SetFocus();
}
