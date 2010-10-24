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
#include "sys/semaphore.h"
SYS_NAMESPACE_BEGIN

CSysVSemaphore::CSysVSemaphore()
    :_semid(-1)
{

}

void CSysVSemaphore::verhoog(uint16_t number)
{
    semaphore_operation(1, number);
}

void CSysVSemaphore::passeren(uint16_t number)
{
    semaphore_operation(-1, number);
}

void CSysVSemaphore::semaphore_operation(int operation, uint16_t number)
{
    /* If an operation specifies SEM_UNDO, 
       it will be automatically undone when the process terminates.
    */

    struct sembuf sops[1];
    sops[0].sem_num = number;        /* semaphore number */
    sops[0].sem_op  = operation;     /* semaphore operation */    
    sops[0].sem_flg = SEM_UNDO;      /* operation flags, such as SEM_UNDO */
    
    if (-1 == semop(_semid, sops, sizeof(sops)/sizeof(sops[0])))
        throw CSyscallException(errno, __FILE__, __LINE__, "verhoog error");
}

SYS_NAMESPACE_END
