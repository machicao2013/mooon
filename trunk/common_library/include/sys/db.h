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
#ifndef SYS_DB_H
#define SYS_DB_H
#include "sys/sys_config.h"
#define SQL_MAX 4096 /** 一条SQL语句允许的最大长度 */
SYS_NAMESPACE_BEGIN

class CDBException
{
public:
    /***
      * 构造一个异常对象
      * 请注意不应当显示调用构造函数
      */
    CDBException(const char* sql, const char* error_message, int error_number=0, const char* filename=__FILE__, int line_number=__LINE__)
    {
        if (sql != NULL) _sql = sql;
        if (error_message != NULL) _error_message = error_message;
        if (filename != NULL) _filename = filename;
        _error_number = error_number;
        _line_number = line_number;
    }

    /** 返回执行出错的SQL语句，如果不是执行SQL语句，则仅返回一个字符串结尾符 */
    const char* get_sql() const
    {
        return _sql.c_str();
    }
    
    /** 返回数据库的出错信息 */
    const char* get_error_message() const
    { 
        return _error_message.c_str(); 
    }

    /** 返回数据库的出错代码 */
    int get_error_number() const
    {
        return _error_number;
    }

    /** 返回执行数据库操作时出错的文件名 */
    const char* get_filename() const
    {
        return _filename.c_str();
    }

    /** 返回执行数据库操作时出错的代码行 */
    int get_line_number() const
    {
        return _line_number;
    }
    
private:   
    std::string _sql;
    std::string _error_message; /** 错误信息 */
    int _error_number;          /** 错误号 */
    std::string _filename;      /** 抛出异常的文件名 */
    int _line_number;                  /** 抛出异常的代码行 */
};

/***
  * 记录行接口
  */
class IRecordrow
{
public:
    /** 虚拟析构函数，仅用于应付编译器的告警 */
    virtual ~IRecordrow() {}

    /***
      * 通过字段编号取得字段的值
      */
    virtual const char* get_field_value(uint16_t index) const = 0;
};

/***
  * 记录集接口
  */
class IRecordset
{
public:
    /** 虚拟析构函数，仅用于应付编译器的告警 */
    virtual ~IRecordset() {}

    /***
      * 得到记录集的行数
      * 对于MySQL，如果query时，参数is_stored为false，则该函数不能返回正确的值，
      * 所以应当只有在is_stored为true，才使用该函数
      */
    virtual size_t get_row_number() const = 0;

    /***
      * 得到字段个数
      */
    virtual uint16_t get_field_number() const = 0;

    /***
      * 判断记录集是否为空
      */
    virtual bool is_empty() const = 0;

    /***
      * 检索结果集的下一行
      * @return: 如果没有要检索的行返回NULL，否则返回指向记录行的指针，这时必须调用release_recordrow，否则有内存泄漏
      */
    virtual IRecordrow* get_next_recordrow() const = 0;

    /***
      * 释放get_next_recordrow得到的记录行
      */
    virtual void release_recordrow(IRecordrow* recordrow) = 0;
};

/***
  * 数据库连接接口
  */
class IDBConnection
{
public:    
    /** 虚拟析构函数，仅用于应付编译器的告警 */
    virtual ~IDBConnection() {}

    /** 是否允许自动提交 */
    virtual void enable_autocommit(bool enabled) = 0;  
    
    /***
      * 用来判断数据库连接是否正建立着 
      */
    virtual bool is_established() const = 0;

    /***
      * 数据库查询类操作，包括：select, show, describe, explain和check table等
      * @is_stored: 是否将所有记录集拉到本地存储
      * @return: 如成功返回记录集的指针，这时必须调用release_recordset，否则有内存泄漏
      * @exception: 如出错抛出CDBException异常
      */
    virtual IRecordset* query(bool is_stored, const char* format, ...) = 0;
    
    /***
      * 释放query得到的记录集
      */
    virtual void release_recordset(IRecordset* recordset) = 0;

    /***
      * 数据库insert和update更新操作
      * @return: 如成功返回受影响的记录个数
      * @exception: 如出错抛出CDBException异常
      */
    virtual size_t update(const char* format, ...) = 0;
};

/***
  * 数据库连接池接口
  */
class IDBConnectionPool
{
public:
    /** 虚拟析构函数，仅用于应付编译器的告警 */
    virtual ~IDBConnectionPool() {}

    /***
      * 得到全小写形式的数据库类型名，如：mysql和postgresql等
      */
    virtual const char* get_type_name() const = 0;
    
    /***
      * 线程安全函数
      * 从数据库连接池中获取一个连接
      * @return: 如果当前无可用的连接，则返回NULL，否则返回指向数据库连接的指针
      * @exception: 不会抛出任何异常
      */
    virtual IDBConnection* get_connection() = 0;

    /***
      * 线程安全函数
      * 将已经获取的数据库连接放回到数据库连接池中      
      * @exception: 不会抛出任何异常
      */
    virtual void release_connection(IDBConnection* db_connection) = 0;

    /***
      * 创建连接池
      * @pool_size: 数据库连接池中的数据库连接个数
      * @db_ip: 需要连接的数据库IP地址
      * @db_port: 需要连接的数据库服务端口号
      * @db_name: 需要连接的数据库池
      * @db_user: 连接数据库用的用户名
      * @db_password: 连接数据库用的密码
      * @exception: 如出错抛出CDBException异常
      */
    virtual void create(uint16_t pool_size, const char* db_ip, uint16_t db_port, const char* db_name, const char* db_user, const char* db_password) = 0;

    /***
      * 销毁已经创建的数据库连接池
      */
    virtual void destroy() = 0;

    /***
      * 得到连接池中的连接个数
      */
    virtual uint16_t get_connection_number() const = 0;
};

//////////////////////////////////////////////////////////////////////////
// 助手类: DBConnectionHelper, RecordsetHelper和RecordrowHelper

/***
  * DB连接助手类，用于自动释放已经获取的DB连接
  */
class DBConnectionHelper
{
public:
    DBConnectionHelper(IDBConnectionPool* db_connection_pool, IDBConnection*& db_connection)
        :_db_connection_pool(db_connection_pool)
        ,_db_connection(db_connection)
    {
    }
    
    ~DBConnectionHelper()
    {
        if ((_db_connection_pool != NULL) && (_db_connection != NULL))
        {
            _db_connection_pool->release_connection(_db_connection);
            _db_connection = NULL;
        }
    }

private:
    IDBConnectionPool* _db_connection_pool;
    IDBConnection*& _db_connection;
};

/***
  * 记录集助手类，用于自动释放已经获取的记录集
  */
class RecordsetHelper
{
public:
    RecordsetHelper(IDBConnection* db_connection, IRecordset* recordset)
        :_db_connection(db_connection)
        ,_recordset(recordset)
    {        
    }

    ~RecordsetHelper()
    {
        if ((_db_connection != NULL) && (_recordset != NULL))
            _db_connection->release_recordset(_recordset);
    }

private:
    IDBConnection* _db_connection;
    IRecordset* _recordset;
};

/***
  * 记录行助手类，用于自动释放已经获取的记录行
  */
class RecordrowHelper
{
public:
    RecordrowHelper(IRecordset* recordset, IRecordrow* recordrow)
        :_recordset(recordset)
        ,_recordrow(recordrow)
    {        
    }

    ~RecordrowHelper()
    {
        if ((_recordset != NULL) && (_recordrow != NULL))
            _recordset->release_recordrow(_recordrow);
    }

private:
    IRecordset* _recordset;
    IRecordrow* _recordrow;
};

SYS_NAMESPACE_END
#endif // SYS_DB_H
