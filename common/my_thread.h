#ifndef MY_THREAD_H
#define MY_THREAD_H

#include <boost/thread/thread.hpp>

#include <boost/thread/recursive_mutex.hpp>
#include <boost/thread/condition_variable.hpp>

typedef boost::recursive_mutex mutex;
typedef boost::recursive_mutex::scoped_lock scoped_lock;
typedef boost::condition_variable_any condition_variable;

/* Остальное включено для редких случаев, в основном для тестирования */
#include <boost/thread/mutex.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <boost/thread/locks.hpp>

typedef boost::mutex u_mutex;
typedef boost::mutex::scoped_lock u_scoped_lock;

using boost::shared_mutex;
using boost::shared_lock;
using boost::unique_lock;

#endif
