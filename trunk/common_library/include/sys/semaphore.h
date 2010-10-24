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
#ifndef SYS_SEMAPHORE_H
#define SYS_SEMAPHORE_H
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/types.h>
#include "sys/sys_config.h"
SYS_NAMESPACE_BEGIN

/** System V信号量C++包装类 */
class CSysVSemaphore
{
public:
    CSysVSemaphore();
    
    void open();
    void create();    
    
    /*** 
      * 信号量V操作，对信号量进行增操作，如果相加结果信号量值不小于0，则不阻塞
      * @exception: 如果出错则抛出CSyscallException异常
      */
    void verhoog(uint16_t number);

    /***
      * 信号量P操作，对信号量减操作，如果减后仍大于或等于0，则不阻塞，否则阻塞
      * @exception: 如果出错则抛出CSyscallException异常
      */
    void passeren(uint16_t number); 
    
private:
    void semaphore_operation(int operation, uint16_t number);
    
private:
    int _semid; /** 信号量句柄 */
};

SYS_NAMESPACE_END
#endif // SYS_SEMAPHORE_H
