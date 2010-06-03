#ifndef HOSTS_STATE_H
#define HOSTS_STATE_H

#include "../pinger/host_state.h"

namespace pinger {

class hosts_state
{
	private:
		int counters_[4]; /* Счётчик объектов для каждого состояния */
		bool ack_by_state_[4]; /* Квитирование по состояниям */

	public:
		hosts_state()
		{
			counters_[host_state::unknown] = 0;
			counters_[host_state::ok]      = 0;
			counters_[host_state::warn]    = 0;
			counters_[host_state::fail]    = 0;

			ack_by_state_[host_state::unknown] = true;
			ack_by_state_[host_state::ok]      = true;
			ack_by_state_[host_state::warn]    = true;
			ack_by_state_[host_state::fail]    = true;
		}

		inline bool operator==(const hosts_state &other) const
		{
			return state() == other.state()
				&& acknowledged() == other.acknowledged();
		}

		inline bool operator!=(const hosts_state &other) const
		{
			return state() != other.state()
				|| acknowledged() != other.acknowledged();
		}

		hosts_state& operator<<(const host_state &hs)
		{
			counters_[ hs.state() ]++;
			if (!hs.acknowledged())
				ack_by_state_[ hs.state() ] = false;
			return *this;
		}

		inline host_state::state_t state() const
		{
			host_state::state_t st = host_state::unknown;

            /* Состояние группы:
            	- равно состоянию всех объектов, если оно у всех одинаковое;
            	- равно unknown, если есть хоть одно отличие */

			for (int i = host_state::ok; i <= host_state::fail; i++)
			{
				if (counters_[i] != 0)
				{
					if (st == host_state::unknown) 
						st = (host_state::state_t)i;
					else
						return host_state::unknown;
				}
			}
			
			return st;
		}
		
		inline bool acknowledged(void) const
		{
			bool ack = true;

			/* Состояние квитирования:
				- не квитировано - если хоть один объект не квитирован */
			
			for (int i = host_state::unknown; i <= host_state::fail; i++)
			{
				if (!ack_by_state_[i])
					ack = false;
			}

			return ack;
		}
		
		inline int count(host_state::state_t st) const
			{ return counters_[st]; }

		inline bool acknowledged(host_state::state_t st) const
			{ return ack_by_state_[st]; }
};

}

#endif
