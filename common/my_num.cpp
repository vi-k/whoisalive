#include "my_num.h"
#include "my_qi.h"
#include "my_str.h"

using namespace std;

namespace my { namespace num {


template<class Char, class Type, class Rule>
Type to_num_def(const Char *str, Rule rule, Type def, std::size_t size)
{
	Type res;
	const Char *ptr = str;
	const Char *end = (std::size_t)-1 ? my::str::end(str) : str + size;
	qi::parse(ptr, end, rule, res);
	return ptr == end ? res : def;
}

template<class Char, class Type, class Rule>
std::size_t to_num(const Char *str, Rule rule, Type &res, std::size_t size)
{
	const Char *ptr = str;
	const Char *end = (std::size_t)-1 ? my::str::end(str) : str + size;
	qi::parse(ptr, end, rule, res);
	return ptr - str;
}

template<class Char, class Type, class Rule>
bool to_num_b(const Char *str, Rule rule, Type &res, std::size_t size)
{
	const Char *ptr = str;
	const Char *end = (std::size_t)-1 ? my::str::end(str) : str + size;
	qi::parse(ptr, end, rule, res);
	return ptr == end;
}


template<class Char, class Type>
inline Type to_signed_def(const Char *str, Type def, std::size_t size)
{
	return to_num_def(str, qi::int_parser<Type, 10, 1, -1>(), def, size);
}

template<class Char, class Type>
inline std::size_t to_signed(const Char *str, Type &res, std::size_t size)
{
	return to_num(str, qi::int_parser<Type, 10, 1, -1>(), res, size);
}

template<class Char, class Type>
inline bool to_signed_b(const Char *str, Type &res, std::size_t size)
{
	return to_num_b(str, qi::int_parser<Type, 10, 1, -1>(), res, size);
}


template<class Char, class Type>
inline Type to_unsigned_def(const Char *str, Type def, std::size_t size)
{
	return to_num_def(str, qi::uint_parser<Type, 10, 1, -1>(), def, size);
}

template<class Char, class Type>
inline std::size_t to_unsigned(const Char *str, Type &res, std::size_t size)
{
	return to_num(str, qi::uint_parser<Type, 10, 1, -1>(), res, size);
}

template<class Char, class Type>
inline bool to_unsigned_b(const Char *str, Type &res, std::size_t size)
{
	return to_num_b(str, qi::uint_parser<Type, 10, 1, -1>(), res, size);
}


template<class Char, class Type>
inline Type to_real_def(const Char *str, Type def, std::size_t size)
{
	return to_num_def(str, qi::real_parser<Type>(), def, size);
}

template<class Char, class Type>
inline std::size_t to_real(const Char *str, Type &res, std::size_t size)
{
	return to_num(str, qi::real_parser<Type>(), res, size);
}

template<class Char, class Type>
inline bool to_real_b(const Char *str, Type &res, std::size_t size)
{
	return to_num_b(str, qi::real_parser<Type>(), res, size);
}

}}

int my_num_dummy()
//int main()
{

#define DUMMY(N,T) {\
	T n = 0;\
	my::num::to_##N##_def("", n, -1);\
	my::num::to_##N("", n, -1);\
	my::num::to_##N##_b("", n, -1);\
	my::num::to_##N##_def(L"", n, -1);\
	my::num::to_##N##(L"", n, -1);\
	my::num::to_##N##_b(L"", n, -1);}

    DUMMY(signed,char)
    DUMMY(signed,short)
    DUMMY(signed,int)
    DUMMY(signed,long)
    DUMMY(signed,long long)

    DUMMY(unsigned, unsigned char)
    DUMMY(unsigned, unsigned short)
    DUMMY(unsigned, unsigned int)
    DUMMY(unsigned, unsigned long)
    DUMMY(unsigned, unsigned long long)

    DUMMY(real, float)
    DUMMY(real, double)
    DUMMY(real, long double)

    return 0;
}
