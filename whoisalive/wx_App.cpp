/***************************************************************
 * Name:	  wx_App.cpp
 * Purpose:   Code for Application Class
 * Author:    vi.k (vi.k@mail.ru)
 * Created:   2010-03-30
 * Copyright: vi.k ()
 * License:
 **************************************************************/

#include <boost/config/warning_disable.hpp> /* против unsafe */

#include "ipgui.h"
#include "wx_App.h"

#include "handle_exception.h"

//(*AppHeaders
#include "wx_Main.h"
#include <wx/image.h>
//*)

#include "../common/my_str.h"
#include "../common/my_exception.h"
#include "../common/my_log.h"
extern my::log main_log;

#include <exception>
using namespace std;

IMPLEMENT_APP(wx_App);

wx_App *App;

bool wx_App::OnInit()
{
	App =  this;

	wxHandleFatalExceptions(true);

	//(*AppInitialize
	bool wxsOK = true;
	wxInitAllImageHandlers();
	if ( wxsOK )
	{
	wx_Frame* Frame = new wx_Frame(0);
	Frame->Show();
	SetTopWindow(Frame);
	}
	//*)
	return wxsOK;

}

bool wx_App::OnExceptionInMainLoop()
{
	try
	{
		throw;
	}
	catch (exception &e)
	{
		handle_exception(&e, L"in App::OnExceptionInMainLoop", L"Ошибка");
	}
	catch (...)
	{
		handle_exception(0, L"in App::OnExceptionInMainLoop", L"Ошибка");
	}

	return true;
}

void wx_App::OnUnhandledException()
{
	try
	{
		throw;
	}
	catch (exception &e)
	{
		handle_exception(&e, L"in App::OnUnhandledException", L"Ошибка");
	}
	catch (...)
	{
		handle_exception(0, L"in App::OnUnhandledException", L"Ошибка");
	}
}

void wx_App::OnFatalException()
{
	try
	{
		throw;
	}
	catch (exception &e)
	{
		handle_exception(&e, L"in App::OnFatalException", L"Критическая ошибка");
	}
	catch (...)
	{
		handle_exception(0, L"in App::OnFatalException", L"Критическая ошибка");
	}
}
