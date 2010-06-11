#include "my_num.h"

#include <string>
#include <iostream>
using namespace std;

int main()
{
#define TEST(NN,N,T,STR){\
	const char cstr[] = STR;\
	string str(STR);\
	const wchar_t wcstr[] = L##STR;\
	wstring wstr(L##STR);\
	cout << "type=" << NN << endl;\
	cout << "string=" << STR << endl;\
	T n;\
	size_t s;\
	\
	n = 0;\
	s = my::num::get(cstr, -1, n);\
	cout << "get(cstr)=" << s;\
	if (cstr[s] != 0) cout << " (fail)";\
	cout << " n=" << n << endl;\
	n = 0;\
	s = my::num::get(str, n);\
	cout << "get(str)=" << s;\
	if (str[s] != 0) cout << " (fail)";\
	cout << " n=" << n << endl;\
	n = 0;\
	s = my::num::get(wcstr, -1, n);\
	cout << "get(wcstr)=" << s;\
	if (wcstr[s] != 0) cout << " (fail)";\
	cout << " n=" << n << endl;\
	n = 0;\
	s = my::num::get(wstr, n);\
	cout << "get(wstr)=" << s;\
	if (wstr[s] != 0) cout << " (fail)";\
	cout << " n=" << n << endl;\
	\
	n = 0;\
	n = my::num::to_##N##_def(cstr, -1, 0);\
	cout << "to_"##NN##"_def(cstr)=" << n << endl;\
	n = 0;\
	n = my::num::to_##N##_def(str, 0);\
	cout << "to_"##NN##"_def(str)=" << n << endl;\
	n = 0;\
	n = my::num::to_##N##_def(wcstr, -1, 0);\
	cout << "to_"##NN##"_def(wcstr)=" << n << endl;\
	n = 0;\
	n = my::num::to_##N##_def(wstr, 0);\
	cout << "to_"##NN##"_def(wstr)=" << n << endl;\
	\
	n = 0;\
	bool b = my::num::try_to_##N(cstr, -1, n);\
	cout << "try_to_"##NN##"(cstr)=" << b << " n=" << n; cout << endl;\
	n = 0;\
	b = my::num::try_to_##N(str, n);\
	cout << "try_to_"##NN##"(str)=" << b << " n=" << n; cout << endl;\
	n = 0;\
	b = my::num::try_to_##N(wcstr, -1, n);\
	cout << "try_to_"##NN##"(wcstr)=" << b << " n=" << n; cout << endl;\
	n = 0;\
	b = my::num::try_to_##N(wstr, n);\
	cout << "try_to_"##NN##"(wstr)=" << b << " n=" << n; cout << endl;\
	cout << endl; }

	cout << boolalpha << fixed << dec;

	TEST("int",int,int,"12345")
	TEST("int",int,int,"-999999")
	TEST("int",int,int,"3000000000")
	TEST("uint",uint,unsigned int,"-1")
	TEST("short",short,short,"-32768")
	TEST("short",short,short,"32768")
	TEST("ulong",ulong,unsigned long,"123")
	TEST("ulong",ulong,unsigned long,"123a")
	TEST("longlong",longlong,long long,"-9223372036854775808")
	TEST("ulonglong",ulonglong,unsigned long long,"18446744073709551615")

	long long a;

	cout.precision(7);
	TEST("float",float,float,"0.123456")
	cout.precision(16);
	TEST("double",double,double,"0.123456789012345")
	TEST("long_double",long_double,long double,"0.123456789012345")

	return 0;
}