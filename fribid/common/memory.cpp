#include "memory.h"
#include "npfunctions.h"

void *Memory::Alloc(int size)
{
    return NPN_MemAlloc(size);
}

void Memory::Free(void *ptr)
{
    NPN_MemFree(ptr);
}

char *Memory::AllocString(const char *str)
{
    if (!str)
        return NULL;
    int len = strlen(str) +1;
    char* ret = (char *)Memory::Alloc(len);
    if (!ret)
        return NULL;
    strcpy_s(ret, len, str);
    return ret;
}

char *Memory::AllocStringLower(const char *str)
{
    if (!str)
        return NULL;
    int len = strlen(str) +1;
    char *ret = (char *)Memory::Alloc(len);
    if (!ret)
        return NULL;

    for (int i = 0; i < len; i++)
        ret[i] = tolower(str[i]);
    return ret;
}
