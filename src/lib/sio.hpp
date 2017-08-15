#ifndef _SIO_HPP
#define _SIO_HPP 1

#include <winsock2.h>
using ssize_t=int;
ssize_t                         /* Write "n" bytes to a descriptor. */
writen(SOCKET fd, const void *vptr, size_t n) noexcept
;

ssize_t                         /* Read "n" bytes from a descriptor. */
readn(SOCKET fd, void *vptr, size_t n) noexcept
;

ssize_t                        //Return read bytes, -1 on error.
readall(SOCKET fd, void **pvptr, size_t initSize) noexcept
;

#endif
