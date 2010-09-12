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
 * Author: JianYi, eyjian@qq.com or eyjian@gmail.com
 */
#include "sys/thread.h"
#include "sys/sys_util.h"
#include "time_keeper/time_keeper.h"

MY_NAMESPACE_BEGIN
class CTimeKeeper: public sys::CThread, public ITimeKeeper
{
public:
    CTimeKeeper();
    void stop();

private:
    virtual void run();
    virtual time_t now();    

private:   
    volatile bool _stop;    
    volatile time_t _now;
};

CTimeKeeper::CTimeKeeper()
    :_stop(false)
{
}

void CTimeKeeper::run()
{
    while (!_stop)
    {
        sys::CSysUtil::millisleep(1000);
        _now = time(NULL);
    }
}

time_t CTimeKeeper::now()
{
    return _now;
}

void CTimeKeeper::stop()
{
    _stop = true;
}

//////////////////////////////////////////////////////////////////////////
// µ¼³öº¯Êý

CTimeKeeper* g_time_keeper = NULL;

void destroy_time_keeper()
{
    if (g_time_keeper != NULL)
    {
        g_time_keeper->stop();

        try
        {
            g_time_keeper->join();
        }
        catch (sys::CSyscallException& ex)
        {
            // Nothing to do
        }
        g_time_keeper = NULL;
    }
}

ITimeKeeper* create_time_keeper()
{
    if (NULL == g_time_keeper)
        g_time_keeper = new CTimeKeeper;

    try
    {
        g_time_keeper->start();
    }
    catch (sys::CSyscallException& ex)
    {
        delete g_time_keeper;
        g_time_keeper = NULL;
    }

    return g_time_keeper;
}

ITimeKeeper* get_time_keeper()
{
    return g_time_keeper;
}

MY_NAMESPACE_END
