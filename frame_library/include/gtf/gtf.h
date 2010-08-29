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
 * Author: JianYi, eyjian@qq.com
 */
#ifndef GTF_H
#define GTF_H
#include "util/log.h"
#include "gtf/factory.h"
MY_NAMESPACE_BEGIN

/** GTF is General TCP server Framework£¬¼´Í¨ÓÃTCP·þÎñ¶Ë¿ò¼Ü
  */
class IGtf
{ 
public:
    /** ¿ÕÐéÄâÎö¹¹º¯Êý£¬ÒÔÆÁ±Î±àÒëÆ÷¸æ¾¯ */
    virtual ~IGtf() {}

    virtual bool create(IGtfConfig* config, IGtfFactory* factory) = 0;
    virtual void destroy() = 0;  
};

// Global functions exported
extern IGtf* create_gtf();
extern void destroy_gtf(IGtf* gtf);

MY_NAMESPACE_END
#endif // GTF_H
