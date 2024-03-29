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

void
Advance(String* string, U64 amount)
{
    if (string->size >= amount)
    {
        string->data += amount;
        string->size -= amount;
    }
    
    else ZeroStruct(string);
}

inline bool
StringCompare(String s0, String s1, bool is_case_sensitive = true)
{
    if (is_case_sensitive)
    {
        while (s0.size != 0 && s1.size != 0 && s0.data[0] == s1.data[0])
        {
            Advance(&s0, 1);
            Advance(&s1, 1);
        }
    }
    
    else
    {
        while (s0.size != 0 && s1.size != 0 && ToUpper(s0.data[0]) == ToUpper(s1.data[0]))
        {
            Advance(&s0, 1);
            Advance(&s1, 1);
        }
    }
    
    return (s0.size == 0 && s1.size == 0);
}

inline bool
SubStringCompare(String s0, String s1)
{
    while (s0.size != 0 && s1.size != 0 && s0.data[0] == s1.data[0])
    {
        Advance(&s0, 1);
        Advance(&s1, 1);
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

void
EatAllWhitespace(String* string)
{
    while (string->size != 0)
    {
        if ((*string->data == ' '  ||
             *string->data == '\t' ||
             *string->data == '\v'))
        {
            Advance(string, 1);
        }
        
        else break;
    }
}