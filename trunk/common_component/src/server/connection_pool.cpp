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
#include <sys/log.h>
#include <net/net_util.h>
#include "connection_pool.h"
MOOON_NAMESPACE_BEGIN

CConnectionPool::CConnectionPool()
    :_connection_array(NULL)
    ,_connection_queue(NULL)
{
}

CConnectionPool::~CConnectionPool()
{
    destroy();    
}

void CConnectionPool::destroy()
{    
    if (_connection_array != NULL)
    {
        delete []_connection_array;
        _connection_array = NULL;
    }

    if (_connection_queue != NULL)
    {
        delete _connection_queue;
        _connection_queue = NULL;
    }
}

void CConnectionPool::create(uint32_t connection_count, IServerFactory* factory)
{
    _connection_array = new CConnection[connection_count];
    _connection_queue = new util::CArrayQueue<CConnection*>(connection_count);    
    
    for (uint32_t i=0; i<connection_count; ++i)
    {
        IProtocolParser* parser = factory->create_protocol_parser();
        IRequestResponsor* responsor = factory->create_request_responsor(parser);

        _connection_array[i].set_parser(parser);
        _connection_array[i].set_responsor(responsor);

        push_waiter(&_connection_array[i]);
    }
}

CConnection* CConnectionPool::pop_waiter()
{
    if (_connection_queue->is_empty()) return NULL;
    
    CConnection* connection = _connection_queue->pop_front();
    connection->set_in_poll(false);

    return connection;
}

void CConnectionPool::push_waiter(CConnection* connection)
{
    // 防止同一个Connection多次被PUsh
    if (!connection->is_in_pool())
    {    
        connection->set_in_poll(true);
        
        if (connection->get_fd() != -1)
        {
            SERVER_LOG_DEBUG("Close waiter: %s:%d.\n", connection->get_peer_ip().to_string().c_str(), connection->get_peer_port());
	        connection->close();
        }
    
	    connection->reset();    
        _connection_queue->push_back(connection);
    }
}

MOOON_NAMESPACE_END
