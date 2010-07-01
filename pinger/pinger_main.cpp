#include "stdafx.h"

#include "config.h"

#include "acceptor.h"

#include "../common/my_time.h"
#include "../common/my_xml.h"
#include "../common/my_fs.h"
#include "../common/my_exception.h"
#include "../common/my_log.h"
#include "../common/my_ptr.h"
#include "../common/my_utf8.h"

using namespace std;

void WINAPI service_main(int argc, wchar_t** argv);
int start(bool as_service);
int install_service(const wchar_t *path, const wchar_t *name);
int remove_service(const wchar_t *name);
int start_service(const wchar_t *name);
wstring get_winerror_text(DWORD error_id);
int echo_lastwinerror();
void service_control_handler(DWORD request);
BOOL __stdcall console_ctrl_handler(DWORD dwCtrlType);

SERVICE_STATUS g_service_status;
SERVICE_STATUS_HANDLE g_status_handle;

wofstream main_log_stream;
void on_main_log(const wstring &text)
{
	main_log_stream << my::time::to_wstring(
		posix_time::microsec_clock::universal_time(), L"[%Y-%m-%d %H:%M:%S]\n")
		<< text << endl << endl;
	main_log_stream.flush();
}
my::log main_log(on_main_log);

int wmain(int argc, wchar_t* argv[])
{
	/**********/
	setlocale(LC_ALL, ""); /* для wcstombs, wcout, wcerr и подобных */
	/**********/

	/* Полный путь к программе */
	fs::wpath path(argv[0]);
	path = fs::system_complete(path);

	if (argc > 1)
	{
		wchar_t *service_name = argc < 3 ? L"pinger" : argv[2];

		if ( wcscmp(argv[1], L"?") == 0
			|| wcscmp(argv[1], L"-?") == 0
			|| wcscmp(argv[1], L"/?") == 0)
		{
			wcout
				<< L"\nUsage:\n"
				<< L"    pinger.exe install/remove/start [service_name]\n"
				<< L"  or\n"
				<< L"    pinger.exe [standalone]\n\n";
			return EXIT_SUCCESS;
		}
		
		else if ( wcscmp(argv[1], L"install") == 0 )
			return install_service(path.file_string().c_str(), service_name);
		
		else if ( wcscmp(argv[1], L"remove") == 0 )
			return remove_service(service_name);
		
		else if ( wcscmp(argv[1], L"start") == 0 )
			return start_service(service_name);
		
		else if ( wcscmp(argv[1], L"standalone") == 0 )
		{
			wcout << L"Starting as standalone...\n";
			return start(false);
		}
		
		else if (argc == 2)
			wcerr << argc << L"Unknown parameter: " << argv[1] << endl;
		else if (argc > 2)
			wcerr << argc << L"Unknown parameters: " << argv[1]
				<< L" ..." << endl;
		
		return EXIT_FAILURE;
	}

	/* Установка текущей директории туда же, где исходный файл */
	fs::current_path( path.parent_path() );

	/* Запуск лога */
	{
		path.replace_extension(L".log");

		bool log_exists = fs::exists(path);
		main_log_stream.open(path.string().c_str(), ios::app);
		if (!log_exists)
			main_log_stream << L"\xEF\xBB\xBF";
		else
			main_log_stream << wstring(78, L'-') << endl << endl;

		main_log_stream.imbue( locale( main_log_stream.getloc(),
			new boost::archive::detail::utf8_codecvt_facet) );
	}

	wchar_t empty[1] = {0};
	SERVICE_TABLE_ENTRY service_table[1];
	service_table[0].lpServiceName = empty;
	service_table[0].lpServiceProc = (LPSERVICE_MAIN_FUNCTION)service_main;

	wcout << L"Starting as service...\n";
	main_log << L"Starting as service" << main_log;

	if (!StartServiceCtrlDispatcher(service_table))
	{
		if (GetLastError() != ERROR_FAILED_SERVICE_CONTROLLER_CONNECT)
			return echo_lastwinerror();

		wcout << L"Failed.\nStarting as standalone...\n";
		main_log << L"Don't started as service.\nStarting as standalone" << main_log;
		return start(false);
	}

	wcout << "ok" << endl;

	return EXIT_FAILURE;
}

void WINAPI service_main(int argc, wchar_t** argv)
{
	wcout << "service_main()" << endl;
	
	g_service_status.dwServiceType = SERVICE_WIN32_OWN_PROCESS; 
	g_service_status.dwCurrentState = SERVICE_START_PENDING; 
	g_service_status.dwControlsAccepted = 0;//SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;
	g_service_status.dwWin32ExitCode = 0; 
	g_service_status.dwServiceSpecificExitCode = 0; 
	g_service_status.dwCheckPoint = 0; 
	g_service_status.dwWaitHint = 0; 

	g_status_handle = RegisterServiceCtrlHandler(L"",
		(LPHANDLER_FUNCTION)service_control_handler);

	if (g_status_handle == 0)
	{
		g_service_status.dwCurrentState = SERVICE_STOPPED;
		g_service_status.dwWin32ExitCode = GetLastError();
		return; 
	}

	g_service_status.dwCurrentState = SERVICE_RUNNING; 
	SetServiceStatus(g_status_handle, &g_service_status);

	int ret = start(true);
	if (ret != EXIT_SUCCESS)
	{
		g_service_status.dwCurrentState = SERVICE_STOPPED;
		g_service_status.dwWin32ExitCode = ret;
	}
}

int start(bool as_service)
{
	try
	{
		#ifdef _DEBUG
		wcout << L"Debug: " << VERSION << L" " << BUILDNO
			<< L" " << BUILDDATE L" " << BUILDTIME << endl;
		main_log << L"Start as "
			<< (as_service ? L"service" : L"standalone")
			<< L"\nDebug: " VERSION L" " BUILDNO L" " BUILDDATE L" " BUILDTIME
			<< main_log;
		#else
		wcout << L"Release: " << VERSION << endl;
		main_log << L"Start as "
			<< (as_service ? L"service" : L"standalone")
			<< L"\nRelease: " VERSION
			<< main_log;
		#endif


		/* Реакция на нажатие Ctrl-C */
		SetConsoleCtrlHandler(console_ctrl_handler, TRUE);

		scoped_ptr<acceptor::server> acceptor;

		try
		{
			xml::wptree pt;
			my::xml::load(L"config.xml", pt);
			acceptor.reset( new acceptor::server(pt.get_child(L"config")) );
		}
		catch(my::exception &e)
		{
			throw my::exception(L"Ошибка инициализации сервера")
				<< my::param(L"config", L"config.xml")
				<< e;
		}
		catch(exception &e)
		{
			throw my::exception(L"Ошибка инициализации сервера")
				<< my::param(L"config", L"config.xml")
				<< e;
		}
		
		acceptor->run();
	}
	catch (my::exception &e)
	{
		wstring error = e.message();
		wcout << L"\n-- my::exception --\n"
			<< error << endl;
		main_log << L"-- my::exception --\n"
			<< error << main_log;
		return EXIT_FAILURE;
	}
	catch (std::exception &e)
	{
		my::exception my_e(e);
		wstring error = my_e.message();
		wcout << L"\n-- std::exception --\n"
			<< error << endl;
		main_log << L"-- std::exception --\n"
			<< error << main_log;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

void service_control_handler(DWORD request)
{
	main_log << L"Service stop" << main_log;
	g_service_status.dwWin32ExitCode = 0; 
	g_service_status.dwCurrentState = SERVICE_STOPPED; 
	SetServiceStatus(g_status_handle, &g_service_status);
	
	exit(EXIT_FAILURE);
}

/* Обработчик нажатий Ctrl-Break */
BOOL __stdcall console_ctrl_handler(DWORD dwCtrlType)
{
	main_log << L"Break" << main_log;
	return FALSE;
}

int install_service(const wchar_t *path, const wchar_t *name)
{
	SC_HANDLE hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
	if (!hSCManager)
		return echo_lastwinerror();

	SC_HANDLE hService = CreateService(
		hSCManager,
		name,
		name,
		SERVICE_ALL_ACCESS,
		SERVICE_WIN32_OWN_PROCESS,
		SERVICE_DEMAND_START,
		SERVICE_ERROR_NORMAL,
		path,
		NULL, NULL, NULL, NULL, NULL);

	if (!hService)
	{
		CloseServiceHandle(hSCManager);
		return echo_lastwinerror();
	}

	CloseServiceHandle(hService);
	CloseServiceHandle(hSCManager);

	wcout << L"Success install service \"" << name << L'\"';
	
	#ifdef _DEBUG
	wcout << " (" << path << L")";
	#endif
	
	wcout << endl;
	
	return EXIT_SUCCESS;
}

int remove_service(const wchar_t *name)
{
	SC_HANDLE hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (!hSCManager)
		return echo_lastwinerror();

	SC_HANDLE hService = OpenService(hSCManager,
		name, SERVICE_STOP | DELETE);

	if (!hService)
	{
		CloseServiceHandle(hSCManager);
		return echo_lastwinerror();
	}

	if (!DeleteService(hService))
	{
		CloseServiceHandle(hSCManager);
		return echo_lastwinerror();
	}

	CloseServiceHandle(hService);
	CloseServiceHandle(hSCManager);
	wcout << L"Success remove service \"" << name << '\"' << endl;

	return EXIT_SUCCESS;
}

int start_service(const wchar_t *name)
{
	SC_HANDLE hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
	if (!hSCManager)
		return echo_lastwinerror();

	SC_HANDLE hService = OpenService(hSCManager, name, SERVICE_START);
	if (!hService)
	{
		CloseServiceHandle(hSCManager);
		return echo_lastwinerror();
	}

	if (!StartService(hService, 0, NULL))
	{
		CloseServiceHandle(hSCManager);
		return echo_lastwinerror();
	}

	wcout << L"Success start service \"" << name << '\"' << endl;
  
	CloseServiceHandle(hService);
	CloseServiceHandle(hSCManager);
	return EXIT_SUCCESS;
}

/* Текст Windows-ошибки */
wstring get_winerror_text(DWORD error_id)
{
	wchar_t *tmp = NULL;
	wstring str;

	FormatMessageW( FORMAT_MESSAGE_FROM_SYSTEM
			| FORMAT_MESSAGE_ALLOCATE_BUFFER
			| FORMAT_MESSAGE_MAX_WIDTH_MASK,
		NULL, error_id,
		MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT),
		(wchar_t*)&tmp, 0, NULL );

	if (tmp)
	{
		str = tmp;
		LocalFree(tmp);

		boost::algorithm::trim(str);
	
		/* Некрасивая точка в конце всех ошибок */
		if ( str[ str.size() - 1 ] = L'.' )
			str.resize( str.size() - 1 );
	}

	return str;
}

int echo_lastwinerror()
{
	DWORD err = GetLastError();
	wcerr << get_winerror_text(err) << L" (code: " << err << L")\n";
	return err;
}
