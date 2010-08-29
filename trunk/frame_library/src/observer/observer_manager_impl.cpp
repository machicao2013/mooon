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
#include "observer/observer_manager_impl.h"
MY_NAMESPACE_BEGIN

CObserverManager::CObserverManager(IDataReporter* data_reporter, uint16_t report_frequency_seconds)
	:_data_reporter(data_reporter)
	,_report_frequency_seconds(report_frequency_seconds)
{
	_observer_thread = new CObserverThread(this);
}

bool CObserverManager::create()
{
	try
	{
		_observer_thread->start();
	}
	catch (sys::CSyscallException& ex)
	{
		MYLOG_FATAL("Created observer manager failed for %s at %s:%d.\n", strerror(ex.get_errcode()), ex.get_filename(), ex.get_linenumber());
		return false;
	}

	return true;
}

void CObserverManager::destroy()
{
	_observer_thread->stop();
	_observer_thread->dec_refcount();
}

void CObserverManager::register_observee(IObservable* observee)
{
	_observee_set.insert(observee);
}

void CObserverManager::collect()
{
	for (std::set<IObservable*>::iterator iter=_observee_set.begin(); iter!=_observee_set.end(); ++iter)
	{
		IObservable* observee = *iter;
		observee->on_report(_data_reporter);
	}
}

//////////////////////////////////////////////////////////////////////////
// È«¾Öº¯Êý

IObserverManager* create_observer_manager(IDataReporter* data_reporter, uint16_t report_frequency_seconds)
{
	CObserverManager* observer_manager_impl = new CObserverManager(data_reporter, report_frequency_seconds);
	return observer_manager_impl->create()? observer_manager_impl: NULL;
}

void destroy_observer_manager(IObserverManager* observer_manager)
{
	CObserverManager* observer_manager_impl = (CObserverManager *)observer_manager;
	observer_manager_impl->destroy();
	delete observer_manager_impl;
}

MY_NAMESPACE_END

