#include "memory.h"
#include <stdarg.h>
#include <stdio.h>

char *CreateFormatString(const char *format, ...) {
    va_list args;
    va_start(args, format);
    int len = _vscprintf(format, args) +1;
    char *ret = (char *)Memory::Alloc(len * sizeof(char));
    vsprintf_s(ret, len, format, args);
    va_end(args);

    return ret;
}
