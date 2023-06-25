#ifndef CSTR1_H
#define CSTR1_H

#include <cstring>

inline bool isSpace(int c)
{
    return c == ' ' || c == '\t' || c == '\n' || c == '\v' || c == '\f' || c == '\r';
}

inline void ltrim(char* str)
{
    unsigned i = 0;
    while (isSpace(str[i]))
    {
        ++i;
    }
    unsigned j;
    for (j = 0; j != strlen(str + i); ++j)
    {
        str[j] = str[j+i];
    }
    str[j] = 0;
}

inline void rtrim(char* str)
{
    for (unsigned i = strlen(str); i != 0; --i)
    {
        int j = i - 1;
        if (isSpace(str[j]))
        {
            str[j] = 0;
        }
        else
        {
            return;
        }
    }
}

inline void trim(char* str)
{
    rtrim(str);
    ltrim(str);
}

inline void toFilename(char* filename, size_t n, const char* path)
{
    for (unsigned i = strlen(path); i != 0; --i)
    {
        unsigned j = i - 1;
        if (path[j] == '\\' || path[j] == '/')
        {
            strncpy(filename, path + i, n);
            return;
        }
    }
    strncpy(filename, path, n);
}

inline void toFilenameNoext(char* path)
{
    for (unsigned i = strlen(path); i != 0; --i)
    {
        unsigned j = i - 1;
        if (path[j] == '/' || path[j] == '\\')
        {
            return;
        }
        if (path[j] == '.')
        {
            path[j] = 0;
            return;
        }
    }
}

#endif // CSTR1_H
