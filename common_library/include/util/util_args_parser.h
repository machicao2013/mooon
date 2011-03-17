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
 * 功能说明: 通过解析命令行参数，用全局变量代码对应的参数值，
 * 全局变量位于ArgsParser名字空间下，全局变量的名字和参数名相同，
 * 全局变量的类型为ArgsType
 *
 * 支持三种形式的命令行参数：
 * 1. --arg_name=arg_value，要求arg_name至少两个字符，如：--port=8080
 * 2. --arg_name，不带参数值，要求arg_name至少两个字符，如：--color
 * 3. -arg_name，不带参数值，这种情况下要求arg_name为单个字符，如：-p
 *
 * 定义命令行参数是必须的，而声明根据需要可选
 *
 * 使用方法：
 * 假设需要一个端口号和IP命令参数，数据类型分别为uint16_t和std::string，则可定义为：
 *STRING_ARG_DEFINE(true, ip, 127.0.0.1, listen IP)
 *INTEGER_ARG_DEFINE(true, uint16_t, port, 8080, 1000, 6554, listen port)
 *注:字符串不需要用""括起来
 * 上面这段定义，通常和main函数放在同一个文件中。
 *
 * 如果多个文件需要使用到命令行参数，则可以使用声明的方式，如：
 * INTEGER_ARG_DECLARE(uint16_t, port)
 * STRING_ARG_DECLARE(ip)
 *
 * 在需要使用到命令行参数的地方，只需要这样：
 * ArgsParser::port->get_value()
 * ArgsParser::ip->get_value()
 *
 *其它接口：
 *ArgsParser::get_help_string() 获取命名行的帮助信息
 *ArgsParser::g_error_message 解析的错误信息
 * 示例：
 *
 * INTEGER_ARG_DEFINE(true, uint16_t, port, 8080, 1000, 6554, listen port)
 * STRING_ARG_DEFINE(true, ip, 127.0.0.1, listen IP)
 *
 * int main(int argc, char* argv[])
 * {
 *    if (!ArgsParser::parse(argc, argv))
 *    {
 *        // 解析失败，可能是某参数不符合要求，
 *        // 或者是必选参数不存在，
 *        // 或者是出现未定义的参数
 *        // 或者是出现重复的参数
 *        exit(1);
 *    }
 *
 *    // 使用
 *    ArgsParser::port->get_value(); // 这里的get_value返回的是uint16_t类型
 *    ArgsParser::ip->get_value(); // 这里的get_value返回的是std::string类型
 *
 *    return 0;
 * }
 *
 * Author: weijingqi  kekimail@gmail.com
 *
 */
#ifndef UTIL_ARGS_PARSER_H
#define UTIL_ARGS_PARSER_H
#include <map> // 用来存储命令行参数名和它对应的值信息
#include <typeinfo>
#include "util/string_util.h"
//////////////////////////////////////////////////////////////////////////

//参数信息接口
class IArgInfo
{
public:
	IArgInfo()
	{
	}
	virtual ~IArgInfo()
	{
	}
	;
	/**设置参数值*/
	virtual void set_value(std::string value) = 0;
	/**得到参数名*/
	virtual std::string get_param_name() = 0;
	/** 判断命令行中是否有该参数*/
	virtual bool is_set() = 0;
	/**设置命令行中有该参数*/
	virtual void set()=0;
	/** 判断参数是否有值 */
	virtual bool has_value() = 0;
	/** 判断是否为可选参数 */
	virtual bool is_optional() = 0;

	virtual std::string to_string() = 0;
	/**获取帮助*/
	virtual std::string get_help_string() = 0;
	/**
	 * 验证value是否符合该参数
	 */
	virtual bool validate_value(std::string value_str) = 0;
};
//解析器命名空间
namespace ArgsParser {
/***
 * 用来存储命令行参数名和它对应的值信息，
 * Key为参数名，Value为参数信息
 */
std::map<std::string, IArgInfo*> g_ArgsInfoMap;
std::string g_error_message;
/***
 * 解析命令行参数，
 * 参数和main函数相同，通过在main中调用它
 * @return: 如果解析成功，则返回true，否则返回false
 */
bool parse(int argc, char* argv[])
{
	g_error_message = "";
	char* param;
	std::string param_str;
	std::string name;
	std::string value_str;
	bool with_value;
	for (int i = 1; i < argc; i++)
	{
		param = argv[i];
		value_str.clear();
		with_value = false;
		name.clear();
		//--开始
		if (param[0] == '-' && param[1] == '-')
		{
			param_str = param;
			param_str = param_str.substr(2);
			int index = param_str.find('=');
			if (index)
			{
				name = param_str.substr(0, index);
				value_str = param_str.substr(index + 1);
				with_value = true;
			} else
			{
				name = param_str;
			}
			//名字长度须大于2
			if (name.length() < 2)
			{
				g_error_message += "Error:" + (std::string) param + " the min length of arg name should be 2\r\n";
				return false;
			}
		} else if (param[0] == '-') //-开始

		{
			param_str = param;
			param_str = param_str.substr(1);
			name = param_str;
			if (name.length() != 1)
			{
				g_error_message += "Error:" + (std::string) param + " the length of arg name should be 1\r\n";
				return false;
			}
		}
		if (name.empty())
		{
			g_error_message += "Error:" + (std::string) param + " arg name can not be null\r\n";
			return false;
		}
		//按具体参数的规则判断
		if (g_ArgsInfoMap.find(name) == g_ArgsInfoMap.end())
		{
			g_error_message += "Error:" + (std::string) param + " the command rule not contains:" + name + "\r\n";
			return false;
		}
		//判断value
		if (with_value)
		{
			if (!g_ArgsInfoMap.find(name)->second->validate_value(value_str))
			{
				g_error_message += "Error:" + (std::string) param + " the value of " + name + " is not valid\r\n";
				return false;
			}
			g_ArgsInfoMap.find(name)->second->set_value(value_str);
		}
		g_ArgsInfoMap.find(name)->second->set();
	}
	//检测必填参数是否都填上
	std::map<std::string, IArgInfo*>::iterator it = g_ArgsInfoMap.begin();
	while (it != g_ArgsInfoMap.end())
	{
		if (!it->second->is_optional() && !it->second->is_set())
		{
			g_error_message += "Error: param " + it->second->get_param_name() + " not set\r\n";
			return false;
		}
		it++;
	}
}
/***
 * 注册参数，
 * 不要直接调用它，而应当总是由宏来调用
 */
void register_arg(const std::string param_name, IArgInfo* arg_info)
{
	g_ArgsInfoMap.insert(std::make_pair(param_name, arg_info));
}
/**
 * 获取帮助信息
 */
std::string get_help_info()
{
	std::string info = "";
	std::string name;
	info += "Options:\r\n";
	std::map<std::string, IArgInfo*>::iterator it = g_ArgsInfoMap.begin();
	IArgInfo* argInfo;
	std::string optional;
	while (it != g_ArgsInfoMap.end())
	{
		argInfo = it->second;
		name = argInfo->get_param_name();
		if(argInfo->is_optional())
		{
			optional = "true";
		}
		else
		{
			optional = "false";
		}
		if (name.length() == 1)
		{
			info += "\t-" + name + "\t" + "optional:" + optional + "\t" + argInfo->get_help_string()
					+ "\r\n";
		} else
		{
			info += "\t--" + name + "\t" + "optional:" + optional + "\t" + argInfo->get_help_string()
					+ "\r\n";
		}
		it++;
	}
	return info;
}
} // The end of namespace ArgsParser


UTIL_NAMESPACE_BEGIN

/***
 * 整数类型参数定义
 * @optional: 是否为可选参数，取值为true或false。如果是可选参数，则命令行参数中必须包含它
 * @integer_type: 整数类型，如int或short等
 * @param_name: 参数名，如果命令行参数中有名字和它的相同的，则将值赋给变量ArgsParser::##param_name
 * @default_value: 默认值
 * @min_value: 可取的最小值
 * @max_value: 可取的最大值
 * @help_string: 帮助信息，用来显示参数作用
 * @使用方法: ArgsParser::param_name.get_value()，
 *            假设参数名为port，则为ArgsParser::port.get_value()，
 *            port的数据类型为CArgInfo<IntegerType>
 * @提示: 各种类型整数的最大最小值使用std::numeric_limits<uint16_t>.min()系列来判断
 */
#define INTEGER_ARG_DEFINE(optional, integer_type, param_name, default_value, min_value, max_value, help_string) \
    namespace ArgsParser /** 保证不污染全局空间 */ { \
        CArgInfo<integer_type> *param_name= new CIntArgInfo<integer_type>(optional,#param_name, default_value,min_value,max_value,#help_string);\
    }
/***
 * 字符串类型参数定义
 * @optional: 是否为可选参数
 * @param_name: 参数名，如果命令行参数中有名字和它的相同的，则将值赋给变量ArgsParser::##param_name
 * @default_value: 默认值
 */
#define STRING_ARG_DEFINE(optional, param_name, default_value, help_string)\
    namespace ArgsParser /** 保证不污染全局空间 */ { \
		CArgInfo<std::string> *param_name= new CStringArgInfo<std::string>(optional,#param_name,#default_value,#help_string);\
    }
// 整数类型参数声明
#define INTEGER_ARG_DECLARE(integer_type, param_name) \
    namespace ArgsParser /** 保证不污染全局空间 */ { \
        extern integer_type param_name; \
    }
// 整数类型参数声明
#define STRING_ARG_DECLARE(param_name) \
    namespace ArgsParser /** 保证不污染全局空间 */ { \
        extern std::string param_name; \
    }
////////////下面为参数具体实现///////
/***
 * 参数信息类
 */

template <typename DataType>
class CArgInfo : public IArgInfo
{
public:
	CArgInfo(std::string param_name)
	{
		_param_name = param_name;
		ArgsParser::register_arg(_param_name, this);
	}
	~CArgInfo()
	{}
	/**设置参数值*/
	void set_value(std::string value)
	{
		_has_value = true;
	}
	/**得到参数名*/
	std::string get_param_name()
	{	return _param_name;}
	/** 得到参数的值 */
	DataType get_value()
	{	return _value;}
	/** 得到默认参数的值 */
	DataType get_default_value()
	{	return _default_value;}
	/** 判断命令行中是否有该参数*/
	bool is_set()
	{	return _is_set;}
	/**设置命令行中有该参数*/
	void set()
	{	_is_set = true;}
	/** 判断参数是否有值 */
	bool has_value()
	{	return _has_value;}
	/** 判断是否为可选参数 */
	bool is_optional()
	{
		return _optional;
	}

	std::string to_string()
	{
		return "";
	}
	/**
	 * 验证value是否符合该参数
	 */
	bool validate_value(std::string value_str)
	{
		return false;
	}
	std::string get_help_string()
	{
		return _help_string;
	}
protected:
	DataType _value;
	bool _has_value;
	bool _optional;
	DataType _default_value;
	std::string _help_string;
	bool _is_set;
	std::string _param_name;
};
/***
 * 参数信息类
 */
template <typename DataType>
class CStringArgInfo : public CArgInfo<DataType>
{
public:
	typedef CArgInfo<DataType> parent_cArgInfo;
	CStringArgInfo(bool optional, std::string param_name, std::string default_value, std::string help_string):CArgInfo<DataType>(param_name)
	{
		parent_cArgInfo::_optional = optional;
		parent_cArgInfo::_has_value = false;
		parent_cArgInfo::_is_set = false;
		parent_cArgInfo::_default_value = default_value;
		parent_cArgInfo::_help_string = help_string;
	}
	~CStringArgInfo()
	{
	}
	/**设置参数值*/
	void set_value(std::string value)
	{
		parent_cArgInfo::_has_value = true;
		parent_cArgInfo::_value = value;
	}

	std::string to_string()
	{
		return parent_cArgInfo::_value;
	}
	/**
	 * 验证value是否符合该参数
	 */
	bool validate_value(std::string value)
	{
		if(!value.empty())
		{
			return true;
		}
		else
		{
			return false;
		}
	}
};
/***
 * 参数信息类
 */
template <typename DataType>
class CIntArgInfo : public CArgInfo<DataType>
{
public:
	typedef CArgInfo<DataType> parent_cArgInfo;
	CIntArgInfo(bool optional, std::string param_name, DataType default_value,
			DataType min_value, DataType max_value, std::string help_string):CArgInfo<DataType>(param_name)
	{
		parent_cArgInfo::_optional = optional;
		parent_cArgInfo::_has_value = false;
		parent_cArgInfo::_optional = optional;
		parent_cArgInfo::_is_set = false;
		parent_cArgInfo::_default_value = default_value;
		parent_cArgInfo::_help_string = help_string;
		_max_value = max_value;
		_min_value = min_value;
	}
	~CIntArgInfo()
	{
	}
	/**设置参数值*/
	void set_value(std::string value)
	{
		parent_cArgInfo::_has_value = true;
		CStringUtil::string2int(value.data(), parent_cArgInfo::_value,value.length(), true);
	}

	std::string to_string()
	{
		return CStringUtil::int_tostring(parent_cArgInfo::_value);
	}
	/**
	 * 验证value是否符合该参数
	 */
	bool validate_value(std::string value_str)
	{
		DataType value;
		if(CStringUtil::string2int(value_str.data(),value,value_str.length(), true))
		{
			if(value < _min_value || value > _max_value)
			{
				return false;
			}
			else
			{
				return true;
			}
		}
		return false;
	}
private:
	DataType _min_value;
	DataType _max_value;
};

UTIL_NAMESPACE_END
#endif // UTIL_ARGS_PARSER_H
