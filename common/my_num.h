/*
	Быстрые функции преобразования чисел в строки и из строки
	
	В строку (signed_to, unsigned_to)- своя функция (первый придуманный
		алгоритм оказался быстрее itoa и Boost.Spirit.Karma (но после
		всех наворотов стал таким же как Karma).

	Из строки (to_signed, to_unsigned, to_real)- использую
		Boost.Spirit2 (ужасно медленно компилируется, поэтому шаблоны
		вынес в *.cpp (и чтоб они компилировались, а не игнорировались,
		включил функцию my_num_dummy, которая задействует все возможные
		варианты).
*/

#ifndef MY_NUM_H
#define MY_NUM_H

#include <cstddef> /* std::size_t */
#include <string>

namespace my { namespace num {


#define NUM_TO_STR_BUF_SIZE 64

/*
	signed_to

	Алгоритм преобразования целого числа со знаком в строку.
	Ради производительности имеет ограничение - используется
	временный буфер размером NUM_TO_STR_BUF_SIZE байт
	(128 бит требует 39 байт).
	
	Напрямую использовать не рекомендуется, т.к. шаблон принимает
	любые типы и результат такого действия не предсказуем.
*/
template<class T, class Char>
std::size_t signed_to(T n, Char *str, std::size_t size,
	std::size_t decimals = 0)
{
	static const Char sym[]
		= { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9' };

	/* Результат сохраняем во временный буфер в обратном порядке */
	Char buf[NUM_TO_STR_BUF_SIZE];
	Char *buf_ptr = buf;
	bool neg;

	/* Сохраняем знак. Операции проводим только с отрицательными
		числами. Логичнее было бы проводить с положительными,
		но не все отрицательные числа можно перевести
		в положительные - например, (char)-128 */
	if ( (neg = n < 0) == false)
		n = -n;

	do
	{
		T nn = n / 10;
		*buf_ptr++ = sym[ (nn<<3) + (nn<<1) - n ];
		n = nn;
	} while (n);

	/* На данный момент в буфере хранится число в обратном
		порядке, без знака и без завершающего нуля */

	/* Заполняем выходную строку. Отдельно, если не входим
		в ограничение размера (###), отдельно - если входим */
	Char *str_ptr = str;
	std::size_t buf_size = buf_ptr - buf;
	std::size_t zero_count = (decimals > buf_size ? decimals - buf_size : 0);

	if (buf_size + zero_count + neg >= size)
	{
		if (size != 0)
		{
			Char *str_last = str + size - 1;
			while (str_ptr != str_last)
   				*str_ptr++ = '#';

   			*str_ptr = 0;
		}	
   	}
   	else
   	{
		if (neg)
			*str_ptr++ = '-';

		while (zero_count)
			zero_count--, *str_ptr++ = '0';

		while (buf_ptr != buf)
   			*str_ptr++ = *--buf_ptr;

		*str_ptr = 0;   	
   	}

	return str_ptr - str;
}

/*
	unsigned_to

	Алгоритм преобразования целого числа без знака в строку.
	Ради производительности имеет ограничение - используется
	временный буфер размером NUM_TO_STR_BUF_SIZE байт
	(128 бит требует 39 байт).
	
	Напрямую использовать не рекомендуется, т.к. шаблон принимает
	любые типы и результат такого действия не предсказуем.
*/

template<class T, class Char>
std::size_t unsigned_to(T n, Char *str, std::size_t size,
	std::size_t decimals = 0)
{
	static const Char sym[]
		= { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9' };

	/* Результат сохраняем во временный буфер в обратном порядке */
	Char buf[NUM_TO_STR_BUF_SIZE];
	Char *buf_ptr = buf;

	do
	{
		T nn = n / 10;
		*buf_ptr++ = sym[ n - (nn<<3) - (nn<<1) ];
		n = nn;
	} while (n);

	/* На данный момент в буфере хранится число в обратном
		порядке, без завершающего нуля */

	/* Заполняем выходную строку. Отдельно, если не входим
		в ограничение размера (###), отдельно - если входим */
	Char *str_ptr = str;
	std::size_t buf_size = buf_ptr - buf;
	std::size_t zero_count = (decimals > buf_size ? decimals - buf_size : 0);

	if (buf_size + zero_count >= size)
	{
		if (size != 0)
		{
			Char *str_last = str + size - 1;
			while (str_ptr != str_last)
   				*str_ptr++ = '#';

   			*str_ptr = 0;
		}	
   	}
   	else
   	{
		while (zero_count)
			zero_count--, *str_ptr++ = '0';

		while (buf_ptr != buf)
   			*str_ptr++ = *--buf_ptr;

		*str_ptr = 0;   	
   	}

	return str_ptr - str;
}

template<class T, class Char>
inline std::basic_string<Char> signed_to(T n, std::size_t decimals = 0)
{
	Char buf[NUM_TO_STR_BUF_SIZE];
	std::size_t sz
		= signed_to<T,Char>(n, buf, sizeof(buf), decimals);
	return std::basic_string<Char>(buf);
}

template<class T, class Char>
inline std::basic_string<Char> unsigned_to(T n, std::size_t decimals = 0)
{
	Char buf[NUM_TO_STR_BUF_SIZE];
	std::size_t sz
		= unsigned_to<T,Char>(n, buf, sizeof(buf), decimals);
	return std::basic_string<Char>(buf);
}

#define DEF_NUM_TO_FUNCS(S,N,T)\
template<class Char>\
inline std::size_t N##_to(T n, Char *str, std::size_t size,\
	std::size_t decimals = 0)\
	{ return S##_to<T,Char>(n, str, size, decimals); }\
template<class Char>\
inline std::basic_string<Char> N##_to(T n, std::size_t decimals = 0)\
	{ return S##_to<T,Char>(n, decimals); }\
inline std::string N##_to_string(T n, std::size_t decimals = 0)\
	{ return S##_to<T,char>(n, decimals); }\
inline std::wstring N##_to_wstring(T n, std::size_t decimals = 0)\
	{ return S##_to<T,wchar_t>(n, decimals); }

DEF_NUM_TO_FUNCS(signed,char,char)
DEF_NUM_TO_FUNCS(signed,short,short)
DEF_NUM_TO_FUNCS(signed,int,int)
DEF_NUM_TO_FUNCS(signed,long,long)
DEF_NUM_TO_FUNCS(signed,longlong,long long)

DEF_NUM_TO_FUNCS(unsigned,uchar,unsigned char)
DEF_NUM_TO_FUNCS(unsigned,ushort,unsigned short)
DEF_NUM_TO_FUNCS(unsigned,uint,unsigned int)
DEF_NUM_TO_FUNCS(unsigned,ulong,unsigned long)
DEF_NUM_TO_FUNCS(unsigned,ulonglong,unsigned long long)


template<class Char, class Type, class Rule>
Type to_num_def(const Char *str, Rule rule, Type def,
	std::size_t size = -1);

template<class Char, class Type, class Rule>
std::size_t to_num(const Char *str, Rule rule, Type &res,
	std::size_t size = -1);

template<class Char, class Type, class Rule>
bool to_num_b(const Char *str, Rule rule, Type &res,
	std::size_t size = -1);


template<class Char, class Type>
inline Type to_signed_def(const Char *str, Type def, std::size_t size = -1);

template<class Char, class Type>
inline std::size_t to_signed(const Char *str, Type &res, std::size_t size = -1);

template<class Char, class Type>
inline bool to_signed_b(const Char *str, Type &res, std::size_t size = -1);


template<class Char, class Type>
inline Type to_unsigned_def(const Char *str, Type def, std::size_t size = -1);

template<class Char, class Type>
inline std::size_t to_unsigned(const Char *str, Type &res, std::size_t size = -1);

template<class Char, class Type>
inline bool to_unsigned_b(const Char *str, Type &res, std::size_t size = -1);


template<class Char, class Type>
inline Type to_real_def(const Char *str, Type def, std::size_t size = -1);

template<class Char, class Type>
inline std::size_t to_real(const Char *str, Type &res, std::size_t size = -1);

template<class Char, class Type>
inline bool to_real_b(const Char *str, Type &res, std::size_t size = -1);


#define DEF_TO_NUM_FUNCS(S,N,T)\
template<class Char>\
inline T to_##N##_def(const Char *str, T def, std::size_t size = -1)\
	{ return to_##S##_def<Char,T>(str, def, size); }\
template<class Char>\
inline std::size_t to_##N(const Char *str, T &res, std::size_t size = -1)\
	{ return to_##S##<Char,T>(str, res, size); }\
template<class Char>\
inline bool to_##N##_b(const Char *str, T &res, std::size_t size = -1)\
	{ return to_##S##_b<Char,T>(str, res, size); }\
template<class Char>\
inline T to_##N##_def(const std::basic_string<Char> &str, T def)\
	{ return to_##S##_def<Char,T>(str.c_str(), def, str.size()); }\
template<class Char>\
inline std::size_t to_##N(const std::basic_string<Char> &str, T &res)\
	{ return to_##S##<Char,T>(str.c_str(), res, str.size()); }\
template<class Char>\
inline bool to_##N##_b(const std::basic_string<Char> &str, T &res)\
	{ return to_##S##_b<Char,T>(str.c_str(), res, str.size()); }


DEF_TO_NUM_FUNCS(signed,char,char)
DEF_TO_NUM_FUNCS(signed,short,short)
DEF_TO_NUM_FUNCS(signed,int,int)
DEF_TO_NUM_FUNCS(signed,long,long)
DEF_TO_NUM_FUNCS(signed,longlong,long long)

DEF_TO_NUM_FUNCS(unsigned,uchar,unsigned char)
DEF_TO_NUM_FUNCS(unsigned,ushort,unsigned short)
DEF_TO_NUM_FUNCS(unsigned,uint,unsigned int)
DEF_TO_NUM_FUNCS(unsigned,ulong,unsigned long)
DEF_TO_NUM_FUNCS(unsigned,ulonglong,unsigned long long)

DEF_TO_NUM_FUNCS(real,float,float)
DEF_TO_NUM_FUNCS(real,double,double)
DEF_TO_NUM_FUNCS(real,long_double,long double)

}}

#endif
