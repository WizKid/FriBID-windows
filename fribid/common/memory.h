class Memory
{
public:
    static void *Alloc(int size);
    static void Free(void *ptr);

    static char *AllocString(const char *str);
    static char *AllocStringLower(const char *str);    
};
