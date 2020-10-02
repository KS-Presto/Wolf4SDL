// WL_UTILS.C

#include "wl_utils.h"


fixed FixedMul (fixed a, fixed b)
{
	return (fixed)(((int64_t)a * b + 0x8000) >> 16);
}

word READWORD (byte *ptr)
{
    word val = ptr[0] | ptr[1] << 8;

    return val;
}

longword READLONGWORD (byte *ptr)
{
    longword val = ptr[0] | ptr[1] << 8 | ptr[2] << 16 | ptr[3] << 24;

    return val;
}
