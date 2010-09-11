#ifndef REF_COUNTABLE_H
#define REF_COUNTABLE_H
#include "sys/atomic.h"
SYS_NAMESPACE_BEGIN

class CRefCountable
{
public:
	CRefCountable()
	{
		atomic_set(&_refcount, 0);
	}

	virtual ~CRefCountable()
	{
	}

	void inc_refcount()
	{
		atomic_inc(&_refcount);
	}

	void dec_refcount()
	{
		if (atomic_dec_and_test(&_ref))
		{
			CShared* shared = (CShared *)this;
			delete shared;
		}
	}

private:
	atomic_t _refcount;
};

SYS_NAMESPACE_END
#endif // REF_COUNTABLE_H
