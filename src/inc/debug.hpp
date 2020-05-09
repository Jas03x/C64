#ifndef DEBUG_HPP
#define DEBUG_HPP

#define error(str, ...) printf("[%s]: " str, __FUNCTION__, ##__VA_ARGS__);

#endif