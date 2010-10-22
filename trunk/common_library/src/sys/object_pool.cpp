#include <util/bit_util.h>
#include "sys/object_pool.h"
SYS_NAMESPACE_BEGIN

CRawObjectPool::CRawObjectPool()
    :_use_heap(false)
    ,_guard_size(0)
    ,_object_size(0)
    ,_object_number(0)   
    ,_stack_top_index(0)
    ,_available_number(0)
    ,_stack_top(NULL)
    ,_stack_bottom(NULL)
    ,_object_stack(NULL)
    ,_bucket_bitmap(NULL)
{
}

CRawObjectPool::~CRawObjectPool()
{
    destroy();
}

void CRawObjectPool::destroy()
{
    _use_heap = false;
    _guard_size = 0;
    _object_size = 0;
    _object_number = 0;
    _stack_top_index = 0;
    _available_number = 0;

    if (_stack_bottom != NULL)
    {
        delete []_stack_bottom;
        _stack_bottom = NULL;
    }
    if (_object_stack != NULL)
    {
        delete []_object_stack;
        _object_stack = NULL;
    }
    if (_bucket_bitmap != NULL)
    {
        delete []_bucket_bitmap;
        _bucket_bitmap = NULL;
    }
}

void CRawObjectPool::create(uint16_t object_size, uint32_t object_number, bool use_heap, uint8_t guard_size, char guard_flag)
{
    // 释放之前已经创建的
    destroy();

    // 保存对象大小和个数值
    _use_heap = use_heap;
    _guard_size = guard_size;
    _object_size = (object_size > 0)? object_size: 1;
    _object_number = (object_number > 0)? object_number: 1;

    // 有了guard_size更容易分析出是否有内存越界之类的行为
    _object_size += _guard_size;
    
    _object_stack = new char*[object_number];
    _stack_bottom = new char[_object_size * object_number];
    _stack_top = _stack_bottom + _object_size * (object_number - 1);
    _stack_top_index = object_number-1;
    _available_number = object_number;

    // 设置警戒标识
    memset(_stack_bottom, guard_flag, object_number * _object_size);
    
    for (uint32_t i=0; i<_object_number; ++i)    
        _object_stack[i] = _stack_bottom + _object_size * i; 
        
    // 初始化为1，加8是为了不四舍五入
    _bucket_bitmap = new char[(object_number+8) / 8];
    memset(_bucket_bitmap, 0, (object_number+8) / 8);
}

void* CRawObjectPool::allocate()
{
    if (0 == _stack_top_index)
    {
        return _use_heap? new char[_object_size]: NULL;
    }
    else
    {        
        --_available_number;
        char*  ptr = _object_stack[_stack_top_index--];
        uint32_t bitmap_index = (ptr - _stack_bottom) / _object_size;

        util::CBitUtil::set_bit(_bucket_bitmap, bitmap_index, false);  
        return ptr;
    }
}

bool CRawObjectPool::reclaim(void* object)
{
    char* ptr = (char*)object;

    if ((ptr < _stack_bottom) || (ptr > _stack_top))
    {
        if (_use_heap)
        {
            delete []ptr;
            return true;
        }
        
        return false;
    }
    if ((ptr - _stack_bottom) % _object_size != 0)
    {
        // 边界不对
        return false;
    }

    uint32_t bitmap_index = (ptr - _stack_bottom) / _object_size;
    if (util::CBitUtil::test(_bucket_bitmap, bitmap_index))
    {
        ++_available_number;
        _object_stack[++_stack_top_index] = ptr;
        util::CBitUtil::set_bit(_bucket_bitmap, bitmap_index, true); 
    }

    return true;
}

bool CRawObjectPool::use_heap() const
{
    return _use_heap;
}

uint8_t CRawObjectPool::get_guard_size() const
{
    return _guard_size;
}

uint32_t CRawObjectPool::get_pool_size() const
{
    return _object_number;
}

uint16_t CRawObjectPool::get_object_size() const
{
    return _object_size;
}

uint32_t CRawObjectPool::get_available_number() const
{
    return _available_number;
}

//////////////////////////////////////////////////////////////////////////
// CThreadObjectPool

void CThreadObjectPool::destroy()
{
    CLockHelper<CLock> lock_helper(_lock);
    _raw_object_pool.destroy();
}

void CThreadObjectPool::create(uint16_t object_size, uint32_t object_number, bool use_heap, uint8_t guard_size, char guard_flag)
{
    CLockHelper<CLock> lock_helper(_lock);
    _raw_object_pool.create(object_size, object_number, use_heap, guard_size, guard_flag);
}

void* CThreadObjectPool::allocate()
{
    CLockHelper<CLock> lock_helper(_lock);
    return _raw_object_pool.allocate();
}

bool CThreadObjectPool::reclaim(void* object)
{
    CLockHelper<CLock> lock_helper(_lock);
    return _raw_object_pool.reclaim(object);
}

bool CThreadObjectPool::use_heap() const
{
    return _raw_object_pool.use_heap();
}

uint8_t CThreadObjectPool::get_guard_size() const
{
    return _raw_object_pool.get_guard_size();
}

uint32_t CThreadObjectPool::get_pool_size() const
{
    return _raw_object_pool.get_pool_size();
}

uint16_t CThreadObjectPool::get_object_size() const
{
    return _raw_object_pool.get_object_size();
}

uint32_t CThreadObjectPool::get_available_number() const
{
    return _raw_object_pool.get_available_number();
}

SYS_NAMESPACE_END
