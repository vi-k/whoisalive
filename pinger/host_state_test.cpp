#include "host_state.h"

#include "../common/my_mru.h"

#include <boost/unordered_map.hpp>
#include <boost/functional/hash.hpp>

using namespace std;

/*-
namespace boost {
std::size_t hash_value(const posix_time::ptime &t)
{
	size_t seed = 0;
	int size = sizeof(t) / sizeof(size_t);
	size_t *ptr = (size_t*)&t;

	while (size--)
		boost::hash_combine(seed, *ptr);

	return seed;
}
}
-*/

int main()
{
	pinger::host_state hs;

	hs.set_state(pinger::host_state::fail);
	hs.set_time( posix_time::microsec_clock::universal_time() );
	hs.set_acknowledged(false);

	my::time::set_output_format(cout, "", "@%H:%M:%S%F");

	cout << "cout << " << hs << endl;
	wcout << "wcout << " << hs << endl;

	pinger::host_state hs2;
	stringstream ss;
	ss << hs;
	cout << "ss << " << ss.str() << endl;
	ss >> hs2;
	cout << "2: " << hs2 << endl;

	pinger::host_state hs3;
	wstringstream wss;
	wss << hs2;
	wcout << L"wss << " << wss.str() << endl;
	wss >> hs3;
	cout << "3: " << hs3 << endl;

	cout << hs.to_str<char>() << endl;
	cout << hs.to_string() << endl;
	wcout << hs.to_str<wchar_t>() << endl;
	wcout << hs.to_wstring() << endl;

	cout << hs.brief<char>() << endl;
	wcout << hs.brief<wchar_t>() << endl;


	static const int size = sizeof(hs.time()) / sizeof(size_t);
	cout << "size: " << size << endl;

	//cout << hash_value(hs.time()) << endl;
	cout << my::time::ptime_hash()(hs.time()) << endl;

	boost::unordered_map<posix_time::ptime, pinger::host_state,
		my::time::ptime_hash>
		a;

	boost::unordered_map<posix_time::ptime, pinger::host_state>
		b;

	a[hs.time()] = hs;
	b[hs.time()] = hs;

	//typedef my::mru::list<posix_time::ptime, pinger::host_state> states_list;
	//states_list sl(1000);
	//sl[hs.time()] = hs;

	return 0;
}
