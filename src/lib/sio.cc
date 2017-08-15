#include "sio.hpp"
#include <errno.h>
#include <stdlib.h>

#include <winsock2.h>

#define _u_read(fd,p,n) recv(fd,(char *)p,n,0)
#define _u_write(fd,p,n) send(fd,(const char *)p,n,0)

ssize_t                         /* Read "n" bytes from a WSA descriptor. */
readn(SOCKET fd, void *vptr, size_t n) noexcept
{
    size_t  nleft;
    ssize_t nread;
    char   *ptr;

    ptr = (char *)vptr;
    nleft = n;
    while (nleft > 0) {
        if ( (nread = _u_read(fd, ptr, nleft)) == SOCKET_ERROR) {
            if (WSAGetLastError() == WSAEINTR)
                nread = 0;      /* and call read() again */
            else
                return (SOCKET_ERROR);
        } else if (nread == 0)
            break;              /* EOF */

        nleft -= nread;
        ptr += nread;
    }
    return (n - nleft);         /* return >= 0 */
}

ssize_t                         /* Write "n" bytes to a WSA descriptor. */
writen(SOCKET fd, const void *vptr, size_t n) noexcept
{
    size_t nleft;
    ssize_t nwritten;
    const char *ptr;

    ptr = (const char *)vptr;
    nleft = n;
    while (nleft > 0) {
        if ( (nwritten = _u_write(fd, ptr, nleft)) <= 0) {
            if (nwritten < 0 && WSAGetLastError() == WSAEINTR)
                nwritten = 0;   /* and call write() again */
            else
                return (SOCKET_ERROR);    /* error */
         }

         nleft -= nwritten;
         ptr += nwritten;
    }
    return (n);
}

ssize_t                        //Return read bytes, SOCKET_ERROR on error. Give pvptr a ptr pointing to a nullptr, I'll malloc some space and do my job.
readall(SOCKET fd, void **pvptr, size_t initSize) noexcept
{
    size_t current = initSize ? initSize : 1024;
    void *vptr = *pvptr;
    if(vptr == NULL)
        vptr = malloc(current);
    void *currvptr = vptr;

    {
        ssize_t ret = _u_read(fd, currvptr, current / 2);
        if(ret == SOCKET_ERROR) return SOCKET_ERROR; //TODO: What if WSAEINTR?
        if(ret < current / 2)
        {
            *pvptr = vptr;
            return ret;
        }
        currvptr = (char *)vptr + current / 2;
    }

    while(true)
    {
        ssize_t ret = _u_read(fd, currvptr, current / 2);
        if(ret == SOCKET_ERROR) return SOCKET_ERROR;
        if(ret < current)
        {
            *pvptr = vptr;
            return ret + current / 2;
        }

        current *= 2;
        void *vptrBackup = vptr;
        if((vptr = realloc(vptr, current)) == NULL) {
            free(vptrBackup);
            WSASetLastError(WSAEMSGSIZE);
            return SOCKET_ERROR;
        }
        currvptr = (char *)vptr + current / 2;
    }
}