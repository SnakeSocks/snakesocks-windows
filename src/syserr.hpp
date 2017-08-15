#ifndef _SNAKESOCKS_SYSERR_HPP
#define _SNAKESOCKS_SYSERR_HPP 1

#include "fstr.h"

#include <cstring>
#include <string>
#include <exception>
#include <stdexcept>

#include <winsock2.h>

using std::printf;

#define _do_sys_except(msg, ...) do{ \
                            ::std::string errmsg = fstr("sys error at %s:%d | errno=%d:%s | WSAError=%d, WINAPILastError=%d >", __FILE__, __LINE__, errno, strerror(errno), WSAGetLastError(), GetLastError()); \
                            errmsg += fstr(msg, ##__VA_ARGS__); \
                            errmsg += "\n"; \
                            throw std::runtime_error(errmsg); \
                        }while(0)
#define sysdie(msg, ...) _do_sys_except(msg, ##__VA_ARGS__)

#define _do_except(msg, ...) do{ \
                            ::std::string errmsg = fstr("logic error at %s:%d | errno=%d:%s(usually not help) | WSAError=%d, WINAPILastError=%d >", __FILE__, __LINE__, errno, strerror(errno), WSAGetLastError(), GetLastError()); \
                            errmsg += fstr(msg, ##__VA_ARGS__); \
                            errmsg += "\n"; \
                            throw std::runtime_error(errmsg); \
                        }while(0)
#define die(msg, ...) _do_except(msg, ##__VA_ARGS__)
#endif //_SNAKESOCKS_SYSERR_HPP
