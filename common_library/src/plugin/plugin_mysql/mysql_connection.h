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
 * 代码采用商业友好的Apache协议，可任意修改和分发，但请保留版权说明文字。
 * 如遇到的问题，请发送到上述邮箱，以便及时修复。谢谢合作，共创开源！ 
 *
 * 数据库操作出错时，均要求以CDBException异常的方式处理
 */
#ifndef MYSQL_CONNECTION_H
#define MYSQL_CONNECTION_H
#include "sys/db.h"
#include "plugin/plugin.h"
PLUGIN_NAMESPACE_BEGIN

/***
  * MySQL行
  */
class CMySQLRow: public sys::IRecordrow
{
public:
    CMySQLRow(char** field_array);
    
private:    
    /***
      * 通过字段编号取得字段的值(高效率)
      */
    virtual const char* get_field(uint16_t index) const;

    /***
      * 通过字段名称取得字段的值(低效率)
      */
    virtual const char* get_field(const char* filed_name) const;

private:
    char** _field_array;
};

/***
  * MySQL记录集
  */
class CMySQLRecordset: public sys::IRecordset
{
public:
    CMySQLRecordset(void* resultset);

private:
    /***
      * 得到记录集的行数
      */
    virtual size_t get_row_number() const;

    /***
      * 得到字段个数
      */
    virtual size_t get_field_number() const;

    /***
      * 判断记录集是否为空
      */
    virtual bool is_empty() const;

    /***
      * 检索结果集的下一行
      * @return: 如果没有要检索的行返回NULL
      */
    virtual sys::IRecordrow* get_next_recordrow() const;

    /***
      * 释放get_next_recordrow得到的记录行
      */
    virtual void release_recordrow(sys::IRecordrow* recordrow);

private:
    void* _resultset; /** 存储MySQL结果集 */
};

/***
  * 数据库连接接口
  */
class CMySQLConnection: public sys::IDBConnection
{
public:    
    CMySQLConnection();
    ~CMySQLConnection();
    
    bool is_in_pool() const;
    void set_in_pool(bool yes); 
    void open(const char* db_ip, uint16_t db_port, const char* db_name, const char* db_user, const char* db_password);
    void close();

private:
    /** 是否允许自动提交 */
    virtual void enable_autocommit(bool enabled);
    
    /***
      * 用来判断数据库连接是否正建立着 
      */
    virtual bool is_established() const;
    
    /***
      * 数据库查询类操作，包括：select, show, describe, explain和check table等
      * @is_stored: 是否将所有记录集拉到本地存储
      * @return: 如成功返回记录集的指针
      * @exception: 如出错抛出CDBException异常
      */
    virtual sys::IRecordset* query(bool is_stored, const char* format, ...);
    
    /***
      * 释放query得到的记录集
      */
    virtual void release_recordset(sys::IRecordset* recordset);

    /***
      * 数据库insert和update更新操作
      * @return: 如成功返回受影响的记录个数
      * @exception: 如出错抛出CDBException异常
      */
    virtual size_t update(const char* format, ...);

private:
    bool _in_pool;          /** 是否在连接池中 */
    bool _is_established;   /** 是否已经和数据库建立的连接 */
    void* _mysql_handler;   /** MySQL句柄, 使用void类型是为减少头文件的依赖 */
};

PLUGIN_NAMESPACE_END
#endif // MYSQL_CONNECTION_H
