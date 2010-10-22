#include "util/bit_util.h"
UTIL_NAMESPACE_BEGIN

void CBitUtil::flip(char* bitmap, uint32_t position)
{
    bitmap[position / 8] ^= 1 << (position % 8);
}

bool CBitUtil::test(char* bitmap, uint32_t position)
{
    return 1 == get_bit(bitmap, position);
}

uint8_t CBitUtil::get_bit(char* bitmap, uint32_t position)
{
    return bitmap[position / 8] >> (position % 8) & 1;
}

void CBitUtil::set_bit(char* bitmap, uint32_t position, bool zero)
{    
    if (zero)
    {
        bitmap[position / 8] &= ~(1 << (position % 8));
    }
    else
    {
        bitmap[position / 8] |= 1 << (position % 8);
    }
}

UTIL_NAMESPACE_END
