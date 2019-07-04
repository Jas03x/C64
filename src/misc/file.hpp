#ifndef FILE_HPP
#define FILE_HPP

class File
{
public:
    static char* Read(const char* path, unsigned int& buffer_size);
};

#endif // FILE_HPP