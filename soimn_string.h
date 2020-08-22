struct String
{
    U8* data;
    U64 size;
};

#define CONST_STRING(string) {(U8*)string, sizeof(string) - 1}

inline char
ToLower(char c)
{
    return (c >= 'A' && c <= 'Z' ? (c - 'A') + 'a' : c);
}

inline char
ToUpper(char c)
{
    return (c >= 'a' && c <= 'z' ? (c - 'a') + 'A' : c);
}

inline bool
IsAlpha(char c)
{
    return ((c >= 'A' && c <= 'Z') ||
            (c >= 'a' && c <= 'z'));
}

inline bool
IsDigit(char c)
{
    return (c >= '0' && c <= '9');
}

inline bool
StringCompare(String s0, String s1)
{
    while (s0.size != 0 && s1.size != 0 && s0.data[0] == s1.data[0])
    {
        s0.data += 1;
        s0.size -= 1;
        s1.data += 1;
        s1.size -= 1;
    }
    
    return (s0.size == 0 && s1.size == 0);
}

inline bool
SubStringCompare(String s0, String s1)
{
    while (s0.size != 0 && s1.size != 0 && s0.data[0] == s1.data[0])
    {
        s0.data += 1;
        s0.size -= 1;
        s1.data += 1;
        s1.size -= 1;
    }
    
    return (s0.size == 0 || s1.size == 0);
}

inline U64
CStringLength(const char* cstring)
{
    U64 result = 0;
    for (char* scan = (char*)cstring; *scan; ++scan, ++result);
    return result;
}