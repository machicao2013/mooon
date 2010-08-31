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
 * Author: eyjian@qq.com or eyjian@gmail.com
 */
#include "sys/logger.h"
SYS_NAMESPACE_BEGIN

CLogger::CLogger()
    :_log_array(NULL)
    ,_log_thread(NULL)
{    
}

CLogger::~CLogger()
{    
}

void CLogger::destroy()
{
    _log_thread->stop();
    _log_thread->dec_refcount();
    delete _log_array;
}

bool CLogger::create(const char* logfile_name, uint32_t log_queue_size=1000)
{
    _log_array = new util::CArrayQueue<char*>(log_queue_size);

    try
    {
        _log_thread = new CLogThread;
        _log_thread->inc_refcount()
        if (!_log_thread->start()) 
        {
            _log_thread->dec_refcount();
            return false;
        }
    }
    catch (CSyscallException& ex)
    {
        delete _log_array;
        _log_array = NULL;

        if (_log_thread)
        {
            _log_thread->dec_refcount();
            _log_thread = NULL;
        }

        throw;
    }
    
    return true;
}

void CLogger::enabled_screen(bool both)
{

}

void CLogger::set_single_filesize(uint32_t filesize)
{

}

void CLogger::set_backup_number(uint16_t backup_number)
{
    
}

bool CLogger::enabled_debug()
{

}

bool CLogger::enabled_info()
{

}

bool CLogger::enabled_warn()
{

}

bool CLogger::enabled_error()
{

}

bool CLogger::enabled_fatal()
{

}

bool CLogger::enabled_trace()
{

}

void CLogger::log_debug(const char* format, ...)
{

}

void CLogger::log_info(const char* format, ...)
{

}

void CLogger::log_warn(const char* format, ...)
{

}

void CLogger::log_error(const char* format, ...)
{

}

void CLogger::log_fatal(const char* format, ...)
{

}

void CLogger::log_trace(const char* format, ...)
{

}

//////////////////////////////////////////////////////////////////////////
CLogThread::CLogThread(uint16_t queue_number, uint32_t queue_size)
    :_queue_index(0)
    ,_queue_number(queue_number)
    ,_queue_size(queue_size)
{
    _lock_array = new CLock[queue_number];
    _queue_array = new util::CArrayQueue<char*>*[queue_number];
    for (uint16_t i=0; i<queue_number; ++i)
        _queue_array[i] = new util::CArrayQueue<char*>(queue_size);
}

bool CLogThread::before_start()
{
    for (uint16_t i=0; i<queue_number; ++i)
        delete _queue_array[i];
    delete []_queue_array;
    delete []_lock_array;
}

void CLogThread::run()
{
    while (!_stop)
    {
        
    }
}

int CLogThread::choose_queue()
{
    // _queue_index发生溢出也不会造成影响
    return ++_queue_index % _queue_number;
}

void CLogThread::push_log(const char* log)
{
    int queue_index = choose_queue();

    if (!_queue_array[queue_index]->is_full())
    {
        CLockHelper<CLock> lock(_lock_array[queue_index]);
        _queue_array[queue_index]->push_back(log);

        if (_waiting)
            _event.signal();
    }
}

char* CLogThread::get_log()
{
    for (uint16_t i=0; i<_queue_number; ++i)
    {
        while (!_queue_array[i]->is_empty())
        {
            
        }
    }
}

SYS_NAMESPACE_END
