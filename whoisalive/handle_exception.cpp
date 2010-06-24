#include "stdafx.h"

#include "handle_exception.h"

using namespace std;

#include "../common/my_exception.h"
#include "../common/my_log.h"
extern my::log main_log;

#include <boost/config/warning_disable.hpp> /* против unsafe */

#include <wx/msgdlg.h>

void handle_exception(
	std::exception *e,
	const std::wstring &add_to_log,
	const std::wstring &window_title)
{
    wstring log_title;
    wstring error;

    if (!e)
    {
    	log_title = L"unknown exception";
    	error = L"Неизвестное исключение";
    }
    else
    {
	    my::exception *my_e_ptr = dynamic_cast<my::exception*>(e);

	    if (my_e_ptr)
    	{
    		log_title = L"my::exception";
	    	error = my_e_ptr->message();
    	}
		else
		{
    		log_title = L"std::exception";
			my::exception my_e(*e);
			error = my_e.message();
		}
	}

	main_log << L"-- " << log_title;
	
	if (!add_to_log.empty())
		main_log << ' ' << add_to_log;

	main_log << L" --\n" << error << main_log;

	wxMessageBox(error, window_title, wxOK | wxICON_ERROR);
}
