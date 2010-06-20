#ifndef MY_EMPLOYER_H
#define MY_EMPLOYER_H

#include <cstddef> /* std::size_t */
#include <algorithm>
#include <iterator>
#include <string>
#include <vector>

#include <boost/bind.hpp>
#include <boost/function.hpp>

#include "my_ptr.h"
#include "my_thread.h"

namespace my
{

class worker
{
public:
	friend class employer;
	typedef shared_ptr<class worker> ptr;

private:
	shared_lock<shared_mutex> lock_;
	std::string name_;
	mutex sleep_mutex_;
	condition_variable sleep_cond_;
	boost::function<void ()> on_finish_;

	void on_finish()
	{
		if (on_finish_)
			on_finish_();
    }

public:
	worker(shared_mutex &mutex, const std::string &name,
		boost::function<void ()> on_finish)
		: lock_(mutex)
		, name_(name)
		, on_finish_(on_finish)
	{
	}
};


class employer
{
private:
	typedef std::list<worker::ptr> workers_list;
	bool employer_finish_;
	shared_mutex employer_mutex_;
	workers_list employer_workers_;

public:
	employer()
		: employer_finish_(false)
	{
	}

	~employer()
	{
		/* На всякий случай */
		employer_workers_.clear();
	}

	worker::ptr new_worker(const std::string &name = std::string(),
		boost::function<void ()> on_finish = boost::function<void ()>())
	{
		worker::ptr ptr( new worker(employer_mutex_, name, on_finish) );
		
		employer_workers_.push_back(ptr);
		
		return ptr;
	}

	/* Усыпить поток (но только, если
		не было команды завершить работу) */
	bool sleep(worker::ptr &ptr)
	{
		/* Блокировкой гарантируем атомарность операций:
			сравнения и засыпания */
		unique_lock<mutex> lock(ptr->sleep_mutex_);
			
		if (!employer_finish_)
		{
			ptr->sleep_cond_.wait(lock);
			return true;
		}

		return false;
	}

	/* Разбудить поток */
	void wake_up(worker::ptr &ptr)
	{
		/* Блокировкой гарантируем, что не окажемся
			между if (!finish()) и wait(). Иначе мы
			"разбудим" ещё не спящий поток, но который
			тут же заснёт  */
		unique_lock<mutex> l(ptr->sleep_mutex_);
		ptr->sleep_cond_.notify_all();
	}

	inline void dismiss(worker::ptr &ptr)
	{
		ptr.reset();
	}

	void workers_state(std::vector<std::string> &v)
	{
		v.clear();

		for (workers_list::iterator iter = employer_workers_.begin();
			iter != employer_workers_.end(); ++iter)
		{
			std::ostringstream out;
			long count = iter->use_count();

			out << (*iter)->name_ << " - "
				<< (count == 0 ? "???"
					: count == 1 ? "finished"
					: "works")
				<< " (use: " << iter->use_count() << ")";
			
			v.push_back(out.str());
		}
	}
    
    /* Проверка флага завершения работы */
	inline bool finish()
		{ return employer_finish_; }

	/* Завершить работу */
	void lets_finish()
	{
		/* Устанавливаем флаг завершения */
		employer_finish_ = true;

		/* Будим все потоки */
		for_each(employer_workers_.begin(), employer_workers_.end(),
			boost::bind(&employer::wake_up, this, _1));

		/* Вызываем обработчики завершения */
		for_each(employer_workers_.begin(), employer_workers_.end(),
			boost::bind(&worker::on_finish, _1));
	}

	/* Проверка - завершили ли "работники" работу */
	bool check_for_finish()
	{
		std::size_t count = 0;

		for (workers_list::iterator iter = employer_workers_.begin();
			iter != employer_workers_.end(); ++iter)
		{
			if ( !iter->unique() )
				count++;
		}
		
		return count == 0;
	}

    /* Ожидаем, когда все "работники" завершат работу */
	void wait_for_finish()
	{
		employer_workers_.clear();
		unique_lock<shared_mutex> l(employer_mutex_);
	}
};

}

#endif
