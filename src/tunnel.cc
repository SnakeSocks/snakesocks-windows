#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include "tunnel.hpp"
#include "syserr.hpp"
#include "sio.hpp"
#include "scope_guard.hpp"
#include "NetLib.hpp"
#include <memory>

using unique_cstring_ptr=std::unique_ptr<char, decltype(&std::free)>;

fd tunnel::newConnection()
{
	WSADATA wsaData;
	SOCKET sockfd = INVALID_SOCKET;
	int iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0) sysdie("WSAStartup failed with error: %d\n", iResult);
	
    addrinfo *paddr;
    addrinfo hints { 0 };

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    auto _ = getaddrinfo(mod.serverIp.c_str(), std::to_string(mod.serverPort).c_str(), &hints, &paddr);
    if(_ != 0) {
		WSACleanup();
		sysdie("getaddrinfo failed. Check network connection to %s:%d; returnval=%d, check `man getaddrinfo`'s return value.", mod.serverIp.c_str(), mod.serverPort, _);
	}
    reinforce_scope_begin(____gd_fc3, [p=paddr](){WSACleanup();freeaddrinfo(p);})

    bool success = false;
    for (addrinfo *rp = paddr; rp != NULL; rp = rp->ai_next) {
        sockfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		debug(3) printf("sockfd=%d, lstErr=%d\n", sockfd, WSAGetLastError());
        if (sockfd == INVALID_SOCKET)
            continue;
        int reuse = 1;
        if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse, sizeof(int)) < 0) sysdie("setsockopt(SO_REUSEADDR) failed");
        //if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, (const char*)&reuse, sizeof(int)) < 0) sysdie("setsockopt(SO_REUSEPORT) failed");
        if (connect(sockfd, rp->ai_addr, rp->ai_addrlen) != SOCKET_ERROR) {
            success = true;
            break; /* Success */
        }
		_ = WSAGetLastError(); //If there're some error on connect, preserve WSALastError sothat line50sysdie will print it.
        closesocket(sockfd);
		WSASetLastError(_);
    }
    if(!success) sysdie("Failed to connect to any of these addr.");

    reinforce_scope_end(____gd_fc3)

    if(doAuth(sockfd))
        return sockfd;
    else
        die("Failed to do auth.");
}

bool tunnel::doAuth(fd connfd)
{
    binary_safe_string authdat = mod.makeAuthQuery(connfd);
    unique_cstring_ptr authdatStr(authdat.str, &std::free);
    auto lengthBackup = authdat.length;
    authdat.length = htonl(authdat.length);
    debug(3) printf("Authdat: length=%u\n", lengthBackup);
    if(SOCKET_ERROR == writen(connfd, &authdat.length, sizeof(authdat.length))) sysdie("Write failed.");
    if(SOCKET_ERROR == writen(connfd, authdatStr.get(), lengthBackup)) sysdie("Write failed.");

    uint32_t replyLen;
    if(readn(connfd, &replyLen, 4) == SOCKET_ERROR) sysdie("readn failed.");
    replyLen = ntohl(replyLen);

    if(replyLen > 1024 * 1024 * 1024) sysdie("Fatal: memory out of range. ");
    unique_cstring_ptr rbuf((char *)malloc(replyLen), &std::free);
    if(SOCKET_ERROR == readn(connfd, rbuf.get(), replyLen)) sysdie("read failed.");

    return mod.onAuthReply(rbuf.get(), replyLen, connfd);
}
