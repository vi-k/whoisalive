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

	cout << "\nstr_to\n";
	str = "2010-01-02 12:30:00";
	cout << str << " -> ";
	cout << my::time::to_time(str) << endl;

	str = "2010-13-02 12:30:00";
	cout << str << " -> ";
	cout << my::time::to_time(str) << endl;

	str = "00:45:03.345678";
	cout << str << " -> ";
	cout << my::time::to_duration(str) << endl;

	str = "00:100:03.345678";
	cout << str << " -> ";
	cout << my::time::to_duration(str) << endl;

	str = "";
	cout << str << " -> ";
	cout << my::time::to_duration(str) << endl;

	return 0;
}