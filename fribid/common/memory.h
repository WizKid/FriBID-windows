class Memory
{
public:
    static void *Alloc(int size);
    static char *AllocString(const char *str);
    static char *AllocStringLower(const char *str);
    static void Free(void *ptr);
};
