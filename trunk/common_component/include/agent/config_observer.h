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
#ifndef CONFIG_OBSERVER_H
#define CONFIG_OBSERVER_H
#include <util/util_config.h>
MOOON_NAMESPACE_BEGIN

/***
  * 配置观察者，执行具体的配置更新
  */
class CALLBACK_INTERFACE IConfigObserver
{
public:
    /** 虚拟析构，用于屏蔽编译器警告 */
    virtual ~IConfigObserver() {}

    /***
      * 有配置需要更新时回调些方法
      * @config_name: 需要更新的配置名称
      * @return: 如果更新成功则返回true，否则返回false
      */
    virtual bool on_config_updated(const char* config_name) = 0;
};

MOOON_NAMESPACE_END
#endif // CONFIG_OBSERVER_H
