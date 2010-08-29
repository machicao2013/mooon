#ifndef READ_WRITE_LOCK
#define READ_WRITE_LOCK
#include <pthread.h>
#include "sys/syscall_exception.h"
SYS_NAMESPACE_BEGIN

class CReadWriteLock
{
public:
	CReadWriteLock();
	~CReadWriteLock();

	void unlock();
	void lock_read();
	void lock_write();		
	bool try_lock_read();
	bool try_lock_write();
	bool timed_lock_read(uint32_t millisecond);
	bool timed_lock_write(uint32_t millisecond);
	
private:
	pthread_rwlock_t _rwlock;
};

class CReadLockHelper
{
public:
    CReadLockHelper(CReadWriteLock& lock)
      :_read_lock(lock)
    {
        _read_lock.lock_read();
    }    
    
    ~CReadLockHelper()
    {
        _read_lock.unlock();
    }
    
private:
    CReadWriteLock& _read_lock;
};

class CWriteLockHelper
{
public:
    CWriteLockHelper(CReadWriteLock& lock)
      :_write_lock(lock)
    {
        _write_lock.lock_write();
    }
    
    ~CWriteLockHelper()
    {
        _write_lock.unlock();
    }
    
private:
    CReadWriteLock& _write_lock;
};

SYS_NAMESPACE_END
#endif // READ_WRITE_LOCK
