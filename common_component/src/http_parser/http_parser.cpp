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
 * Author: JianYI, eyjian@qq.com
 */
#include "util/log.h"
#include "parse_command.h"
MY_NAMESPACE_BEGIN

class CHttpParser: public IHttpParser
{
public:
    CHttpParser();
    
private:
	virtual void reset();
    virtual int get_head_length() const;
    virtual void set_http_event(IHttpEvent* event);
    virtual util::TReturnResult parse(const char* buf);

private:
    int _head_length; /** 包头字节数 */
    CParseCommand* _current_command;
    CMethodCommand _method_command;
    CURLCommand _url_command;
    CVersionCommand _version_command;
    CNameValuePairCommand _name_value_pair_command;
    CHeadEndCommand _head_end_command;
};

CHttpParser::CHttpParser()
{
    reset();
}

void CHttpParser::reset()
{
    _head_length = 0;
    _current_command = &_method_command;
    
	_method_command.reset();
	_method_command.set_next(&_url_command);	

	_url_command.reset();
    _url_command.set_next(&_version_command);

	_version_command.reset();
    _version_command.set_next(&_name_value_pair_command);

	_name_value_pair_command.reset();
    _name_value_pair_command.set_next(&_head_end_command);
    
	_head_end_command.reset();
	_head_end_command.set_next(NULL); // 循环终止条件
}

void CHttpParser::set_http_event(IHttpEvent* event)
{
    _method_command.set_event(event);
    _method_command.set_event(event);
    _url_command.set_event(event);
    _version_command.set_event(event);
    _name_value_pair_command.set_event(event);
    _head_end_command.set_event(event);
}

int CHttpParser::get_head_length() const
{
    return _head_length;
}

util::TReturnResult CHttpParser::parse(const char* buffer)
{
    const char* tmp = buffer;

    for (;;)
    {
        int offset = 0;
        util::TReturnResult rr = _current_command->execute(tmp, offset);
        if (util::rr_error == rr)
		{
			return util::rr_error;
		}
        if (util::rr_finish == rr) 
        {
            _current_command = _current_command->get_next();
            if (NULL == _current_command->get_next())
			{
				return util::rr_finish;
			}
        }

        tmp += offset;
        _head_length += offset;
    }

    return util::rr_continue;
}

//////////////////////////////////////////////////////////////////////////
// 导出函数

IHttpParser* create_http_parser()
{
    return new CHttpParser;
}

void destroy_http_parser(IHttpParser* parser)
{
    delete (CHttpParser *)parser;
}
MY_NAMESPACE_END
