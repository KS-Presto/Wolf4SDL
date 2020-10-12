// WL_UTILS.H

#ifndef __WL_UTILS_H_
#define __WL_UTILS_H_

#include "wl_def.h"

#define SafeMalloc(s)    safe_malloc ((s),__FILE__,__LINE__)
#define FRACBITS         16

void     *safe_malloc (size_t size, const char *fname, uint32_t line);
fixed    FixedMul (fixed a, fixed b);
fixed    FixedDiv (fixed a, fixed b);

word     READWORD (byte *ptr);
longword READLONGWORD (byte *ptr);

#endif
