#ifndef MY_NUM_H
#define MY_NUM_H

#include <cstddef> /* std::size_t */
#include <string>

namespace my { namespace num {


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
