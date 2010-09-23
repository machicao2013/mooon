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
#include "sys/db.h"
#include "plugin/plugin_mysql/plugin_mysql.h"
using namespace sys;
using namespace plugin;

int main()
{
    std::string sql = "SELECT * FROM test"; // 需要查询的SQL语句
    std::string db_ip = "127.0.0.1";
    std::string db_name = "test";
    std::string db_user = "root";
    std::string db_password = "";
    
    IDBConnectionPool* db_connection_pool = create_mysql_connection_pool();
    
    try
    {    
        // 创建数据库连接池
        db_connection_pool->create(10, db_ip.c_str(), 3306, db_name.c_str(), db_user.c_str(), db_password.c_str());
    }
    catch (sys::CDBException& ex)
    {
        fprintf(stderr, "Create database connection pool error: %s.\n", ex.get_error_message());
        exit(1);
    }

    do // 这个循环无实际意义，仅为简化代码结构
    {
        // 从数据库连接池中取一个连接
        IDBPoolConnection* db_connection = db_connection_pool->get_connection();
        if (NULL == db_connection)
        {
            fprintf(stderr, "Database pool is empty.\n");
            break;
        }

        // 自动释放
        DBConnectionHelper db_connection_helper(db_connection_pool, db_connection);
        
        try
        {
            size_t row = 0; // 当前行数
            // 执行一条查询语句
            IRecordset* recordset = db_connection->query(false, "%s", sql.c_str());
            uint16_t field_number = recordset->get_field_number();

            // 自动释放
            RecordsetHelper recordset_helper(db_connection, recordset);
            
            for (;;)
            {
                // 取下一行记录
                IRecordrow* recordrow = recordset->get_next_recordrow();
                if (NULL == recordrow) break;

                // 自动释放
                RecordrowHelper recordrow_helper(recordset, recordrow);

                // 循环打印出所有字段值
                fprintf(stdout, "ROW[%04d] ==>\t", row++);
                for (uint16_t col=0; col<field_number; ++col)
                {
                    const char* field_value = recordrow->get_field_value(col);
                    fprintf(stdout, "%s\t", field_value);
                }
                fprintf(stdout, "\n");
            }
        }
        catch (sys::CDBException& ex)
        {
            fprintf(stderr, "Query %s error: %s.\n", ex.get_sql(), ex.get_error_message());            
        }
    } while(false);

    // 销毁数据库连接池
    destroy_mysql_connection_pool(db_connection_pool);
}
