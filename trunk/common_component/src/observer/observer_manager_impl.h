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
#ifndef OBSERVER_MANAGER_IMPL_H
#define OBSERVER_MANAGER_IMPL_H
#include <set>
#include "observer/observer_thread.h"
#include "observer/observer_manager.h"
MOOON_NAMESPACE_BEGIN

class CObserverManager: public IObserverManager
{
public:
	CObserverManager(IDataReporter* data_reporter, uint16_t report_frequency_seconds);
	
	bool create();
	void destroy();
	void collect();
	uint16_t get_report_frequency_seconds() const { return _report_frequency_seconds; }
	
private:
	virtual void register_observee(IObservable* observee);

private:
	IDataReporter* _data_reporter;       /** 数据上报器 */
	uint16_t _report_frequency_seconds;  /** 上报频率，单位为秒 */
	CObserverThread* _observer_thread;   /** 观察线程 */
	std::set<IObservable*> _observee_set;
};

MOOON_NAMESPACE_END
#endif // OBSERVER_MANAGER_IMPL_H
