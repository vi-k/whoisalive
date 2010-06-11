#include "my_time.h"

#include <boost/io/ios_state.hpp>

#include <iostream>
#include <sstream>
#include <string>
using namespace std;


int main()
{

#define TEST_SPECIAL(TN,T,COUT,STR) {\
	T t;\
	COUT << "type=" << TN << endl;\
	t = T(date_time::neg_infin);\
	COUT << "[neg_infin] " << t << " = " << flush;\
	COUT << my::time::to_##STR(t) << endl;\
	t = T(date_time::pos_infin);\
	COUT << "[pos_infin] " << t << " = " << flush;\
	COUT << my::time::to_##STR(t) << endl;\
	t = T(date_time::not_a_date_time);\
	COUT << "[not_a_date_time] " << t << " = " << flush;\
	COUT << my::time::to_##STR(t) << endl;\
	t = T(date_time::min_date_time);\
	COUT << "[min_date_time] " << t << " = " << flush;\
	COUT << my::time::to_##STR(t) << endl;\
	t = T(date_time::max_date_time);\
	COUT << "[max_date_time] " << t << " = " << flush;\
	COUT << my::time::to_##STR(t) << endl;\
	COUT << endl; }

	cout << "*** special values ***\n" << endl;
	TEST_SPECIAL("gregorian::date",gregorian::date,cout,string)
	TEST_SPECIAL("posix_time::time_duration",posix_time::time_duration,cout,string)
	TEST_SPECIAL("posix_time::ptime",posix_time::ptime,cout,string)


#define TEST_D(y,m,d) {\
	gregorian::date t(y,m,d);\
	cout << "       " << t << " (" << y << "," << m << "," << d << ")" << endl << flush;\
	cout << "str  = " << my::time::to_string(t) << flush << endl;\
	wcout << "wstr = " << my::time::to_wstring(t) << flush << endl << endl;}

#define TEST_TD(h,m,s,f) {\
	posix_time::time_duration t(h,m,s,f);\
	cout << "       " << t << " (" << h << "," << m << "," << s\
		<< "," << f << ")" << endl << flush;\
	cout << "str  = " << my::time::to_string(t) << flush << endl;\
	wcout << "wstr = " << my::time::to_wstring(t) << flush << endl << endl;}

#define TEST_PT(y,mo,d,h,mi,s,f) {\
	posix_time::ptime t(gregorian::date(y,mo,d),\
		posix_time::time_duration(h,mi,s,f));\
	cout << "       " << t << " (" << y << "," << mo << "," << d << ","\
		<< h << "," << mi << "," << s << "," << f << ")" << endl << flush;\
	cout << "str  = " << my::time::to_string(t) << flush << endl;\
	wcout << "wstr = " << my::time::to_wstring(t) << flush << endl << endl;}


	cout << "\n*** date/time ***\n" << endl;
	
	TEST_D(1945,5,9)
	TEST_D(2010,1,1)
	TEST_D(2010,12,31)

	cout << "\n*** time_duration ***\n" << endl;

	TEST_TD( 0,0,0,0)
	TEST_TD( 0,0,0,1)
	TEST_TD( 0,0,1,1)
	TEST_TD( 0,1,0,1)
	TEST_TD( 1,0,0,1)
	TEST_TD( 0,0,0,-1)
	TEST_TD( 0,0,-1,1)
	TEST_TD( 0,-1,0,1)
	TEST_TD( -1,0,0,1)
	TEST_TD( -11,-11,-11,-11)

	TEST_TD( 2147483647,0,0,0)
	TEST_TD( 2147483648,0,0,0)
	cout << "!!! time_duration(2147483648,0,0,0).is_negative()="
		<< posix_time::time_duration(2147483648,0,0,0).is_negative()
		<< endl;
	cout << "!!! time_duration(2147483648,0,0,0) > time_duration(2147483647,0,0,0)="
		<< (posix_time::time_duration(2147483648,0,0,0)
			> posix_time::time_duration(2147483647,0,0,0))
		<< endl;
	cout << "!!! Boost.DateTime cast problem\n\n";
		
	TEST_TD( 22,27,9,0)

	cout << "\n*** ptime ***\n" << endl;

	TEST_PT( 2010,6,10, 23,19,0,0)

	cout << "Time to sleep!!!\n\n";

    /*-
	wcout << my::time::to_wstring(gregorian::date(2010,06,29)) << flush << endl;

	cout << my::time::to_string(posix_time::time_duration(23,45,29,123456)) << flush << endl;
	wcout << my::time::to_wstring(posix_time::time_duration(23,45,29,123456)) << flush << endl;
	cout << my::time::to_string(posix_time::time_duration(00,-15,-01)) << flush << endl;
	wcout << my::time::to_wstring(posix_time::time_duration(00,15,-01)) << flush << endl;
	cout << my::time::to_string(posix_time::time_duration(-01,15,00)) << flush << endl;
	wcout << my::time::to_wstring(posix_time::time_duration(-01,15,00)) << flush << endl;

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