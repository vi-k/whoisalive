#ifndef MY_MANY_WORKERS_H
#define MY_MANY_WORKERS_H

#include "my_ptr.h"
#include "my_thread.h"

namespace my
{

class many_workers
{
public:
	typedef shared_ptr< unique_lock<mutex> > lock;

private:
	bool finish_;
	lock lock_ptr_;
	mutex mutex_;

#ifdef _DEBUG
	int num_;
	int __counter()
	{
		static int n = 0;
		return ++n;
	}
#endif

public:
	many_workers()
		: finish_(false)
		#ifdef _DEBUG
		, num_(__counter())
		#endif
	{
		lock_ptr_.reset( new unique_lock<mutex>(mutex_) );
	};

	inline lock get_lock_for_worker()
		{ return lock_ptr_; }

	inline bool finish()
		{ return finish_; }

	inline void lets_finish()
		{ finish_ = true; }

	inline long number_of_workers()
		{ return lock_ptr_.use_count(); }

	inline void wait_for_finish()
	{
		lock_ptr_.reset();
		unique_lock<mutex> l(mutex_);
	}
};

}
#endif
