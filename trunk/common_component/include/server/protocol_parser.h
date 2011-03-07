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
#ifndef PROTOCOL_PARSER_H
#define PROTOCOL_PARSER_H
#include <util/util_config.h>
MOOON_NAMESPACE_BEGIN

class CALLBACK_INTERFACE IProtocolParser
{
public:
    /** 空虚拟析构函数，以屏蔽编译器告警 */
    virtual ~IProtocolParser() {}

    /***
      * 复位解析状态
      */
    virtual void reset() = 0;

    /***
      * 对收到的数据进行解析
      * @data_size: 新收到的数据大小
      */
    virtual util::handle_result_t parse(uint32_t data_size) = 0;

    /***
      * 更新Buffer偏移
      * @offset: 新接收到的数据大小
      */
    virtual void move_buffer_offset(uint32_t offset) = 0;

    /***
      * 得到从哪个位置开始将接收到的数据存储到Buffer
      */
    virtual uint32_t get_buffer_offset() const = 0;

    /***
      * 得到用来接收数据的Buffer大小
      */
    virtual uint32_t get_buffer_size() const = 0;    

    /***
      * 得到用来接收数据的Buffer
      */
    virtual char* get_buffer() = 0;
};

MOOON_NAMESPACE_END
#endif // PROTOCOL_PARSER_H
