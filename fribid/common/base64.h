#ifndef __BASE64_H__
#define __BASE64_H__

class Base64
{
public:
    static char *encode(const char *input, int size);
    static char *decode(const char *input, int size);
};

#endif