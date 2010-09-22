/**
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Author: jian yi, eyjian@qq.com
 */
#ifndef LIST_QUEUE_H
#define LIST_QUEUE_H
#include "util/listable.h"
UTIL_NAMESPACE_BEGIN

/***
  * 可链表对象模板队列
  */
template <class ListableClass>
class CListQueue
{
public:
    /** 构造一个可链表对象模板队列 */
    CListQueue()
        :_head(NULL)
        ,_tail(NULL)
    {
    }

    /** 得到指向队首对象的指针 */
    ListableClass* front() const
    { 
        return _head;
    }

    /** 在队尾添加一个可链表对象 */
    void push(ListableClass* listable)
    {
        ASSERT(listable != NULL);
        if (NULL == listable) return;
        
        listable->set_next(NULL);
        if (NULL == _head && NULL == _tail)
        {
            listable->set_prev(NULL);
            _head = listable;
        }
        else
        {
            listable->set_prev(_tail);
            _tail->set_next(listable);		
        }
        
        _tail = listable;
    }

    /** 
      * 将一个可链表对象从队列中删除
      * 删除操作是高效的，因为0查找，只需要解除链接关系即可
      */
    void remove(ListableClass* listable)
    {
        ASSERT(listable != NULL);
        if (NULL == listable) return;
        
        ListableClass* prev = (ListableClass *)listable->get_prev();
        ListableClass* next = (ListableClass *)listable->get_next();
        if ((NULL == prev) || (NULL == next))
        {    
            if (NULL == prev)
            {
                _head = (ListableClass *)listable->get_next();
                if (_head)
                    _head->set_prev(NULL);
            }	    
            if (NULL == next)
            {
                _tail = (ListableClass *)listable->get_prev();
                if (_tail)
                    _tail->set_next(NULL);
            }
        }
        else
        {
            prev->set_next(next);
            next->set_prev(prev);
        }
        
        listable->set_prev(NULL);
        listable->set_next(NULL);
    }

private:
    ListableClass* _head;
    ListableClass* _tail;
};

UTIL_NAMESPACE_END
#endif // LIST_QUEUE_H
