#include "memory.h"
#include "npfunctions.h"

void *Memory::Alloc(int size)
{
    return NPN_MemAlloc(size);
}

char *Memory::AllocString(const char *str)
{
    int len = strlen(str) +1;
    char* ret = (char *)Memory::Alloc(len);
    if (!ret)
        return NULL;
    strcpy_s(ret, len, str);
    return ret;
}

char *Memory::AllocStringLower(const char *str)
{
    int len = strlen(str) +1;
    char *ret = (char *)Memory::Alloc(len);
    if (!ret)
        return NULL;

    for (int i = 0; i < len; i++)
        ret[i] = tolower(str[i]);
    return ret;
}

void Memory::Free(void *ptr)
{
    NPN_MemFree(ptr);
}