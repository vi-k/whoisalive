#include "my_time.h"

#include <boost/io/ios_state.hpp>

#include <iostream>
#include <sstream>
#include <string>
using namespace std;


/*-
posix_time::ptime now()
{
	return posix_time::microsec_clock::universal_time();
}
-*/

int main()
{
	#if 0
	posix_time::ptime pt1( gregorian::date(2010,06,02),
		posix_time::time_duration(20,41,35,653) );

	posix_time::time_duration td1(0,20,35,500000);

	cout << "set_output_format\n";
	{
		boost::io::ios_all_saver ios_saver(cout);

		my::time::set_output_format(cout, "[%Y.%m.%d %H:%M]");
		cout << pt1 << endl;
		cout << posix_time::ptime() << endl;

		my::time::set_output_format(cout, "{%Hh %Mm %Ss}");
		cout << td1 << endl;
		cout << posix_time::time_duration() << endl;

		my::time::set_output_format(cout, "[%Y.%m.%d %H:%M]", "{%Hh %Mm %Ss}");
		cout << "user fmt: " << pt1 << ' ' << td1 << endl;

		my::time::set_mydef_output_format(cout);
		cout << "my fmt: " << pt1 << ' ' << td1 << endl;
	}

	cout << "sys fmt: " << pt1 << ' ' << td1 << endl;

	cout << "\nset_input_format\n";
	{
		posix_time::ptime pt2;

		stringstream ss;
		boost::io::ios_all_saver ios_saver(ss);
		
		my::time::set_output_format(ss, "[%Y.%m.%d %H:%M]");

		ss << pt1;
		cout << ss.str() << endl;
		my::time::set_input_format(ss, "%Y-%m-%d %H:%M");
		pt2 = posix_time::ptime();
		ss >> pt2;
		cout << pt2 << endl;
		if (!ss)
			cout << "failed\n";

		ss.str("");
		ss.clear();

		ss << pt1;
		cout << ss.str() << endl;
		my::time::set_input_format(ss, "[%Y.%m.%d %H:%M]");
		ss >> pt2;
		cout << pt2 << endl;
		if (!ss)
			cout << "failed\n";

		ss.str("");
		ss.clear();

		my::time::set_mydef_output_format(ss);
		ss << pt1;
		cout << ss.str() << endl;
		my::time::set_mydef_input_format(ss);
		ss >> pt2;
		cout << pt2 << endl;
		if (!ss)
			cout << "failed\n";
	}

	cout << "\nformat\n";
	cout << my::time::format("%Y-%b-%d (%A) %H:%M:%S", pt1) << endl;
	cout << my::time::format("%Y-%b-%d (%A) %H:%M:%S", posix_time::ptime()) << endl;
	cout << my::time::format("%+%H:%M:%S", td1) << endl;

	cout << "\nto_str\n";
	cout << my::time::to_str<char>(pt1) << endl;
	cout << my::time::to_str<char>(posix_time::ptime()) << endl;
	cout << my::time::to_str<char>(td1) << endl;

	cout << "\nformat_to\n";
	string str("1978-Jun-29 00:05:30");
	cout << str << " -> ";
	cout << my::time::format_to_time("%Y-%b-%d %H:%M:%S", str) << endl;

	str = "1976-03-05 14:00:00";
	cout << str << " -> ";
	cout << my::time::format_to_time("%Y-%b-%d %H:%M:%S", str) << endl;

	str = "{23h 45m 23s}";
	cout << str << " -> ";
	cout << my::time::format_to_duration("{%Hh %Mm %Ss}", str) << endl;

	str = "12:00:00";
	cout << str << " -> ";
	cout << my::time::format_to_duration("{%Hh %Mm %Ss}", str) << endl;

	/*-
	cout << "\nstr_to\n";
	str = "2010-01-02 12:30:00";
	cout << str << " -> ";
	cout << my::time::to_time(str) << endl;

	str = "2010-13-02 12:30:00";
	cout << str << " -> ";
	cout << my::time::to_time(str) << endl;
    -*/

    cout << endl;

    #endif

#define TEST(NN,N,T,STR) {\
	const char cstr[] = STR;\
	string str(STR);\
	const wchar_t wcstr[] = L##STR;\
	wstring wstr(L##STR);\
	cout << "type=" << NN << endl;\
	cout << "string=\"" << STR << "\"" << endl;\
	\
	T t;\
	std::size_t n;\
	n = my::time::to_##N##_s(cstr, t);\
	cout << "to_"##NN##"_s(cstr)=" << n << " "##NN##"=" << t << endl;\
	n = my::time::to_##N##_s(str, t);\
	cout << "to_"##NN##"_s(str)=" << n << " "##NN##"=" << t << endl;\
	n = my::time::to_##N##_s(wcstr, t);\
	cout << "to_"##NN##"_s(wcstr)=" << n << " "##NN##"=" << t << endl;\
	n = my::time::to_##N##_s(wstr, t);\
	cout << "to_"##NN##"_s(wstr)=" << n << " "##NN##"=" << t << endl;\
	t = my::time::to_##N(cstr);\
	cout << "to_"##NN##"(cstr)=" << t << endl;\
	t = my::time::to_##N(str);\
	cout << "to_"##NN##"(str)=" << t << endl;\
	t = my::time::to_##N(wcstr);\
	cout << "to_"##NN##"(wcstr)=" << t << endl;\
	t = my::time::to_##N(wstr);\
	cout << "to_"##NN##"(wstr)=" << t << endl;\
	cout << endl; }

	TEST("date",date,gregorian::date,"2010-12-02")
	TEST("date",date,gregorian::date,"2010/13/02")
	TEST("date",date,gregorian::date,"2010/01/01")
	TEST("date",date,gregorian::date,"2010/01")
	TEST("date",date,gregorian::date,"25.01.2002")
	TEST("date",date,gregorian::date,"25.01-2002")
	TEST("date",date,gregorian::date,"25.01.2002 ")
	TEST("date",date,gregorian::date,"29-02-2010")
	TEST("date",date,gregorian::date,"29 06 1978")

	TEST("duration",duration,posix_time::time_duration,"-01:45:03.345678")
	TEST("duration",duration,posix_time::time_duration,"-123456:45:03")
	TEST("duration",duration,posix_time::time_duration,"123456:60:03")
	TEST("duration",duration,posix_time::time_duration,"23:59:34 ")

	TEST("time",time,posix_time::ptime,"2010-06-09 23:59:34")
	TEST("time",time,posix_time::ptime,"2010-06-09 24:00:00")
	TEST("time",time,posix_time::ptime,"2010-06-09 -23:59:34")
	TEST("time",time,posix_time::ptime,"2010/06/09 12:00:00.123456")
	TEST("time",time,posix_time::ptime,"2010.06.09 12:00:00.123456 ")
	TEST("time",time,posix_time::ptime,"29 06 2010 09:30:00 ")
	
	return 0;
}