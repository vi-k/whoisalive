#include "my_time.h"

#include <boost/io/ios_state.hpp>

#include <iostream>
#include <sstream>
#include <string>
using namespace std;


int main()
{
	/*-
	cout << my::time::to_string(gregorian::date(2010,06,29)) << flush << endl;
	wcout << my::time::to_wstring(gregorian::date(2010,06,29)) << flush << endl;

	cout << my::time::to_string(posix_time::time_duration(23,45,29,123456)) << flush << endl;
	wcout << my::time::to_wstring(posix_time::time_duration(23,45,29,123456)) << flush << endl;
	cout << my::time::to_string(posix_time::time_duration(00,-15,-01)) << flush << endl;
	wcout << my::time::to_wstring(posix_time::time_duration(00,15,-01)) << flush << endl;
	cout << my::time::to_string(posix_time::time_duration(-01,15,00)) << flush << endl;
	wcout << my::time::to_wstring(posix_time::time_duration(-01,15,00)) << flush << endl;
	
	cout << my::time::to_string(
		posix_time::time_duration(0,-15,-1)
		+ posix_time::time_duration(0,0,-59)) << flush << endl;
    -*/

	posix_time::time_duration td;

	*(long long*)&td = 0x7ffffffffffffffdi64;
	//*(long long*)&td = 0x8000000000000001i64;

	cout << *(long long*)&td << endl;
	cout << td.ticks() << flush << endl;
	cout << td << flush << endl;
	cout << td.hours() << flush << endl;

	cout << "-> " << posix_time::time_duration::ticks_per_second() << endl;
	cout << "-> " << (long long)td.ticks() / (3600*td.ticks_per_second()) << endl;

	cout << my::time::to_string(td) << flush << endl;
	cout << (td.ticks() < 0) << flush << endl;
	cout << td.is_negative() << flush << endl;

    /*-
	cout << my::time::to_string( posix_time::ptime(
		gregorian::date(2010,06,29),
		posix_time::time_duration(23,45,29,123456))) << flush << endl;
	wcout << my::time::to_wstring( posix_time::ptime(
		gregorian::date(2010,06,29),
		posix_time::time_duration(23,45,29,123456))) << flush << endl;
    -*/

	return 0;
}