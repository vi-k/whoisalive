#include "my_num.h"

#include <string>
#include <iostream>
#include <iterator>
using namespace std;

int main(void)
{

#define TEST_S(TN,N,T,COUT,STR) TEST_S_CAST(TN,N,T,T,COUT,STR)

#define TEST_S_CAST(TN,N,T,CAST,COUT,STR) {\
	COUT << "type=" << TN << endl;\
	STR str;\
	T n;\
	n = 0;\
	str = my::num::to_##STR(n);\
	COUT << "[0000..] " << (CAST)n << " = " << str << endl;\
	n = ~0;\
	str = my::num::to_##STR(n);\
	COUT << "[1111..] " << (CAST)n << " = " << str << endl;\
	n = (unsigned T)~0 >> 1;\
	str = my::num::to_##STR(n);\
	COUT << "[0111..] " << (CAST)n << " = " << str << endl;\
	n = ~((unsigned T)~0 >> 1);\
	str = my::num::to_##STR(n);\
	COUT << "[1000..] " << (CAST)n << " = " << str << endl;\
	n = 123;\
	str = my::num::to_##STR(n);\
	COUT << (CAST)n << " = " << str << endl;\
	n = 123;\
	str = my::num::to_##STR(n, 5);\
	COUT << (CAST)n << "[dec=5] = " << str << endl;\
	n = -123;\
	str = my::num::to_##STR(n, 5);\
	COUT << (CAST)n << "[dec=5] = " << str << endl;\
	STR::value_type buf[64];\
	size_t sz;\
	n = 123;\
	sz = my::num::put(buf, 4, n);\
	COUT << (CAST)n << "[sz=4] = " << buf << " (sz=" << sz << ")" << endl;\
	n = 123;\
	sz = my::num::put(buf, 3, n);\
	COUT << (CAST)n << "[sz=3] = " << buf << " (sz=" << sz << ")" << endl;\
	n = -123;\
	sz = my::num::put(buf, 4, n);\
	COUT << (CAST)n << "[sz=4] = " << buf << " (sz=" << sz << ")" << endl;\
	n = -123;\
	sz = my::num::put(buf, 5, n);\
	COUT << (CAST)n << "[sz=5] = " << buf << " (sz=" << sz << ")" << endl;\
	n = 123;\
	sz = my::num::put(buf, 5, n, 5);\
	COUT << (CAST)n << "[sz=5,dec=5] = " << buf << " (sz=" << sz << ")" << endl;\
	n = 123;\
	sz = my::num::put(buf, 6, n, 5);\
	COUT << (CAST)n << "[sz=6,dec=5] = " << buf << " (sz=" << sz << ")" << endl;\
	n = -123;\
	sz = my::num::put(buf, 6, n, 5);\
	COUT << (CAST)n << "[sz=6,dec=5] = " << buf << " (sz=" << sz << ")" << endl;\
	n = -123;\
	sz = my::num::put(buf, 7, n, 5);\
	COUT << (CAST)n << "[sz=7,dec=5] = " << buf << " (sz=" << sz << ")" << endl;\
	COUT << endl;}

#define TEST_U(TN,N,T,COUT,STR) TEST_U_CAST(TN,N,T,T,COUT,STR)

#define TEST_U_CAST(TN,N,T,CAST,COUT,STR) {\
	COUT << "type=" << TN << endl;\
	STR str;\
	T n;\
	n = 0;\
	str = my::num::to_##STR(n);\
	COUT << "[0000..] " << (CAST)n << " = " << str << endl;\
	n = ~0;\
	str = my::num::to_##STR(n);\
	COUT << "[1111..] " << (CAST)n << " = " << str << endl;\
	n = (T)~0 >> 1;\
	str = my::num::to_##STR(n);\
	COUT << "[0111..] " << (CAST)n << " = " << str << endl;\
	n = ~((T)~0 >> 1);\
	str = my::num::to_##STR(n);\
	COUT << "[1000..] " << (CAST)n << " = " << str << endl;\
	n = 123;\
	str = my::num::to_##STR(n);\
	COUT << (CAST)n << " = " << str << endl;\
	n = 123;\
	str = my::num::to_##STR(n, 5);\
	COUT << (CAST)n << "[dec=5] = " << str << endl;\
	STR::value_type buf[64];\
	size_t sz;\
	n = 123;\
	sz = my::num::put(buf, 4, n);\
	COUT << (CAST)n << "[sz=4] = " << buf << " (sz=" << sz << ")" << endl;\
	n = 123;\
	sz = my::num::put(buf, 3, n);\
	COUT << (CAST)n << "[sz=3] = " << buf << " (sz=" << sz << ")" << endl;\
	n = 123;\
	sz = my::num::put(buf, 5, n, 5);\
	COUT << (CAST)n << "[sz=5,dec=5] = " << buf << " (sz=" << sz << ")" << endl;\
	n = 123;\
	sz = my::num::put(buf, 6, n, 5);\
	COUT << (CAST)n << "[sz=6,dec=5] = " << buf << " (sz=" << sz << ")" << endl;\
	COUT << endl;}

	cout << "*** signed integers ***\n" << endl;
	TEST_S_CAST("char",char,char,int,cout,string)
	TEST_S("short",short,short,cout,string)
	TEST_S("int",int,int,cout,string)
	TEST_S("long",long,long,cout,string)
	TEST_S("longlong",longlong,long long,cout,string)

	cout << "*** signed integers to wstring ***\n" << endl;
	TEST_S_CAST("char",char,char,int,wcout,wstring)
	TEST_S("int",int,int,wcout,wstring)

	cout << "*** unsigned integers ***\n" << endl;
	TEST_U_CAST("uchar",uchar,unsigned char,unsigned int,cout,string)
	TEST_U("ushort",ushort,unsigned short,cout,string)
	TEST_U("uint",uint,unsigned int,cout,string)
	TEST_U("ulong",ulong,unsigned long,cout,string)
	TEST_U("ulonglong",ulonglong,unsigned long long,cout,string)

	cout << "*** unsigned integers to wstring ***\n" << endl;
	TEST_U_CAST("uchar",uchar,unsigned char,unsigned int,wcout,wstring)
	TEST_U("uint",uint,unsigned int,wcout,wstring)

	{
	char buf[20];
	char *ptr = buf;
	cout << my::num::put_signed2(ptr, buf + sizeof(buf)/sizeof(*buf), 123456)
		<< endl;
	*ptr = 0;
	cout << '\"' << buf << '\"' << endl;
	cout << ptr - buf << endl;
	}

	{
	wchar_t buf[20];
	wchar_t *ptr = buf;
	cout << my::num::put_signed2(ptr, buf + sizeof(buf)/sizeof(*buf), 123456)
		<< endl;
	*ptr = 0;
	wcout << '\"' << buf << '\"' << endl;
	cout << ptr - buf << endl;
	}

	{
	int buf[20];
	int *ptr = buf;
	cout << my::num::put_signed2(ptr, buf + sizeof(buf)/sizeof(*buf), 123456)
		<< endl;
	*ptr = 0;
	cout << buf[0] << endl;
	cout << buf[1] << endl;
	cout << buf[2] << endl;
	cout << buf[3] << endl;
	cout << buf[4] << endl;
	cout << buf[5] << endl;
	cout << ptr - buf << endl;
	}

	{
	string str(20,' ');
	string::iterator ptr = str.begin();
	cout << my::num::put_signed2(ptr, str.end(), 123456)
		<< endl;
	str.resize(ptr - str.begin());
	cout << '\"' << str << '\"' << endl;
	cout << ptr - str.begin() << endl;
	}

	/*-
	{
	typedef back_insert_iterator<string> sink_type;

	string str, str2;
	sink_type sink(str), sink2(str2);

	string::iterator ptr = str.begin();
	cout << my::num::put_signed2(sink, sink2, 123456)
		<< endl;
	str.resize(ptr - str.begin());
	cout << '\"' << str << '\"' << endl;
	cout << ptr - str.begin() << endl;
	}
    -*/

	return 0;
}
