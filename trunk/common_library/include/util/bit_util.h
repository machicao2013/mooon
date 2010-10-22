#ifndef _BIT_UTIL_H
#define _BIT_UTIL_H
#include "util/util_config.h"
UTIL_NAMESPACE_BEGIN

/***
  * 位操作工具类
  */
class CBitUtil
{
public:
    /***
      * 对指定位取反，不做越界检查
      * @bitmap: 位图，其位数不能小于position
      * @position: 在bitmap中的位位置
      */
    static void flip(char* bitmap, uint32_t position);

    /***
      * 测试指定位是否为1，不做越界检查
      * @bitmap: 位图，其位数不能小于position
      * @position: 在bitmap中的位位置
      */
    static bool test(char* bitmap, uint32_t position);

    /***
      * 得到指定位的值，不做越界检查
      * @bitmap: 位图，其位数不能小于position
      * @position: 在bitmap中的位位置
      */
    static uint8_t get_bit(char* bitmap, uint32_t position);

    /***
      * 设置指定位的值，不做越界检查
      * @bitmap: 位图，其位数不能小于position
      * @position: 在bitmap中的位位置
      * @zero: 将position所在位设置为0或1，如果为true则设置为0，否则设置为1
      */
    static void set_bit(char* bitmap, uint32_t position, bool zero);
};

UTIL_NAMESPACE_END
#endif // _BIT_UTIL_H
