#include "ping_result.h"
using namespace std;

int main()
{
	pinger::ping_result pr;

	pr.set_state(pinger::ping_result::ok);
	pr.set_time( posix_time::microsec_clock::universal_time() );
	pr.set_duration( posix_time::milliseconds(643) );
	pr.set_sequence_number(123);

	cout << "cout << " << pr << endl;
	wcout << "wcout << " << pr << endl;

	my::time::set_format(cout, "", "@%H:%M:%S%F");
	//cout << posix_time::milliseconds(643) << endl;
	//cout << posix_time::milliseconds(643) << endl;
	//cout << my::time::format("#%H:%M:%S%F", posix_time::milliseconds(643)) << endl;

	pinger::ping_result pr2;
	stringstream ss;
	ss << pr;
	cout << "ss << " << ss.str() << endl;
	ss >> pr2;
	cout << "2: " << pr2 << endl;

	pinger::ping_result pr3;
	wstringstream wss;
	wss << pr2;
	wcout << L"wss << " << wss.str() << endl;
	wss >> pr3;
	cout << "3: " << pr3 << endl;

	cout << pr.to_str<char>() << endl;
	cout << pr.to_string() << endl;
	wcout << pr.to_str<wchar_t>() << endl;
	wcout << pr.to_wstring() << endl;

	cout << pr.brief<char>() << endl;
	wcout << pr.brief<wchar_t>() << endl;

	return 0;
}