#ifndef ASCII_H
#define ASCII_H

// http://www.asciitable.com/

enum
{
    ASCII_NULL            = 0x00,
    ASCII_NEWLINE         = 0x0A,
    ASCII_CARRIAGE_RETURN = 0x0D,
    ASCII_TAB             = 0x09,
    ASCII_DOUBLE_QUOTE    = 0x22,
    ASCII_SINGLE_QUOTE    = 0x27,
    ASCII_BACK_SLASH      = 0x5C
};

#define IS_NUM(x) (((x) >= '0') && ((x) <= '9'))
#define IS_ALPHA(x) ((((x) >= 'a') && ((x) <= 'z')) || (((x) >= 'A') && ((x) <= 'Z')))
#define IS_ALPHA_NUM(x) (IS_ALPHA(x) || IS_NUM(x))
#define IS_SPACE(x) (((x) == ' ') || ((x) == '\t') || ((x) == '\n'))
#define IS_HEXADECIMAL(x) (IS_NUM(x) || ((x >= 'A') && (x <= 'F')))

#endif // ASCII_H