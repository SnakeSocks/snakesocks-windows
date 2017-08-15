#include "socks5.hpp"

#include "syserr.hpp"
#include "sio.hpp"
#include "scope_guard.hpp"
#include "NetLib.hpp"
#include "threadPool.hpp"

#include <functional>
#include <thread>
#include <string>
#include <iostream>
#include <csignal>
#include <cstdlib>
#include <thread>
#include "../mingw-thread-fix/mingw.thread.h"

#include <windows.h>
#include <winsock2.h>
#include <Ws2tcpip.h>
#include <boost/asio.hpp>

using std::string;
using std::cout;
using std::endl;
using fd = int;

void std_string_to_bin_safe(const string &src, binary_safe_string &dst)
{
    dst.length = src.size();
    dst.null_terminated = false;
    dst.str = (char *)malloc(src.size());
    memcpy(dst.str, src.data(), src.size());
}

[[noreturn]] void s5server::run()
{
	WSADATA wsaData;
	SOCKET listenfd = INVALID_SOCKET;
	int iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0) sysdie("WSAStartup failed with error: %d\n", iResult);
	
    addrinfo *psaddr;
    addrinfo hints { 0 };
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;    /* For wildcard IP address */
	hints.ai_protocol = IPPROTO_TCP;
    auto _ = getaddrinfo(bindIp.c_str(), std::to_string(bindPort).c_str(), &hints, &psaddr);
    if(_ != 0) {
		WSACleanup();
		sysdie("Failed to getaddrinfo. returnval=%d, check `man getaddrinfo`'s return value.", _);
	}

    bool success = false;
    for (addrinfo *rp = psaddr; rp != NULL; rp = rp->ai_next) {
        listenfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (listenfd == INVALID_SOCKET)
            continue;
        int reuse = 1;
        if(setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse, sizeof(int)) < 0) sysdie("setsockopt(SO_REUSEADDR) failed");
        //if(setsockopt(listenfd, SOL_SOCKET, SO_REUSEPORT, (const char*)&reuse, sizeof(int)) < 0) sysdie("setsockopt(SO_REUSEPORT) failed");
        if (bind(listenfd, rp->ai_addr, rp->ai_addrlen) != SOCKET_ERROR) {
            success = true;
            break; /* Success */
        }
        closesocket(listenfd);
    }
    if(!success) sysdie("Failed to bind to any of these addr.");

    if(SOCKET_ERROR == listen(listenfd, 16)) sysdie("listen failed.");

    cout << "Listening on " << bindIp << ":" << bindPort << endl;
#ifndef ___NO_THREAD_POOL
    ThreadPool pool;
#endif
    while(true)
    {
        SOCKET connfd = accept(listenfd, nullptr, nullptr);
        if(connfd == INVALID_SOCKET) continue;
#ifdef ___NO_THREAD_POOL
        std::thread(&s5server::dealConnection, this, connfd).detach();
#else
        pool.addTask(std::bind(&s5server::dealConnection, this, connfd));
#endif
    }
    freeaddrinfo(psaddr);
	WSACleanup();
	sysdie("Returning from nonreturn function.");
}

auto s5server::unpackConnectionPacket(const char *pkgStr)
{ //remoteInfo.dstIp is deprecated. remoteInfo.str must be freed.
    if(pkgStr[0] != 5) die("Broken package. Incorrect ver.");
    char CMD = pkgStr[1];
    char ATYP = pkgStr[3];
    client_query remoteInfo;
    memset(remoteInfo.destination_ip, 0, 16);
    if(ATYP == 1)
    {
        //memcpy(remoteInfo.destination_ip, pkgStr + 4, 4);
        std::array<unsigned char, 4> bArr;
        memcpy(bArr.data(), pkgStr+4, 4);
        string v4addr = boost::asio::ip::address_v4(bArr).to_string();
        std_string_to_bin_safe(v4addr, remoteInfo.payload);

        remoteInfo.destination_port = *reinterpret_cast<const uint16_t *>(&pkgStr[8]);
    }
    else if(ATYP == 4)
    {
        //memcpy(remoteInfo.destination_ip, pkgStr + 4, 16);
        std::array<unsigned char, 16> bArr;
        memcpy(bArr.data(), pkgStr+4, 16);
        string v6addr = boost::asio::ip::address_v6(bArr).to_string();
        std_string_to_bin_safe(v6addr, remoteInfo.payload);
        remoteInfo.destination_port = *reinterpret_cast<const uint16_t *>(&pkgStr[20]);
    }
    else if(ATYP == 3)
    {
        size_t addrlen = (size_t)pkgStr[4];
        remoteInfo.destination_port = *reinterpret_cast<const uint16_t *>(&pkgStr[5+addrlen]);

        remoteInfo.payload.length = addrlen;
        remoteInfo.payload.null_terminated = false;
        remoteInfo.payload.str = (char *)malloc(addrlen);
        memcpy(remoteInfo.payload.str, pkgStr+5, addrlen);
    }
    else
        die("Broken package.Invalid ATYP");

    remoteInfo.destination_port = htons(remoteInfo.destination_port);
    return std::make_tuple(CMD, ATYP, remoteInfo);
}

void s5server::handshake(SOCKET connfd)
{
    char headBuf[258];
    if(readn(connfd, headBuf, 2) == SOCKET_ERROR) sysdie("readn socket failed");

    if(headBuf[0] != (char)5) die("Broken protocol. (Incorrect protocol number)");
    size_t methodSize = (size_t)headBuf[1];

    if(readn(connfd, headBuf + 2, methodSize) == SOCKET_ERROR) sysdie("readn socket failed");

    bool methodOk = false;
    for(size_t cter = 0; cter < methodSize; ++cter)
    {
        if(headBuf[2+cter] == 0)
        {
            methodOk = true;
            break;
        }
    }
    headBuf[0] = 5; headBuf[1] = 0;
    if(!methodOk) headBuf[1] = (char)255;
    if(writen(connfd, headBuf, 2) == SOCKET_ERROR) sysdie("Failed to write socket.");
}

client_query s5server::doConnect(SOCKET connfd)
{
    handshake(connfd);
    char buf[263];
    if(readn(connfd, buf, 5) == SOCKET_ERROR) sysdie("readn failed.");
    size_t addrlen = 0;
    bool useDomain = false;
    switch(buf[3])
    {
        case 1:
            addrlen = 4;
            break;
        case 3:
            addrlen = (size_t)buf[4] + 1; // mogic logic.
            useDomain = true;
            break;
        case 4:
            addrlen = 16;
            break;
        default:
            die("Wrong ATYP in connect packet.");
    }
    if(readn(connfd, buf + 5, addrlen + 1) == SOCKET_ERROR) sysdie("readn failed.");

    char CMD, ATYP;
    client_query remoteInfo;
    std::tie(CMD, ATYP, remoteInfo) = unpackConnectionPacket(buf);
    if(CMD != 01) { free(remoteInfo.payload.str); die("Request can not be processed.(CMD=%d)", (int)CMD); }

    //Make success reply.
    buf[1] = 0;
    auto pkgLen = addrlen + 6;
    if(writen(connfd, buf, pkgLen) == SOCKET_ERROR) { free(remoteInfo.payload.str); sysdie("write failed."); }

    return remoteInfo;
}

#define max(a,b) ((a)>(b)?(a):(b))
using unique_cstring_ptr=std::unique_ptr<char, decltype(&std::free)>;

void s5server::passPackets(SOCKET connfd, client_query dnsPackToQuery)
{
    fd thisHop = connfd;
    fd nextHop = ssserver.newConnection();
    reinforce_scope_begin(___s5_pspk, [nh=nextHop](){closesocket(nh);})

    // First data pack (DNS)
    binary_safe_string firstPack = ssserver.mod.encode(nullptr, 0, connfd, dnsPackToQuery);
    unique_cstring_ptr firstPackStr(firstPack.str, &std::free);
    auto lengthBackup = firstPack.length;
    firstPack.length = htonl(firstPack.length);
    debug(3) std::cout << "First data pack to send: len=" << lengthBackup << std::endl;
    if(writen(nextHop, &firstPack.length, sizeof(firstPack.length)) == SOCKET_ERROR) { sysdie("writen failed.");}
    if(writen(nextHop, firstPackStr.get(), lengthBackup) == SOCKET_ERROR) { sysdie("writen failed.");}

    binary_safe_string firstReturnPack;
    if(readn(nextHop, &firstReturnPack.length, sizeof(firstReturnPack.length)) == SOCKET_ERROR) sysdie("read failed");
    firstReturnPack.length = ntohl(firstReturnPack.length);
    firstReturnPack.str = (char *)malloc(firstReturnPack.length);
    if(readn(nextHop, firstReturnPack.str, firstReturnPack.length) == SOCKET_ERROR) { free(firstReturnPack.str); sysdie("read failed"); }
    client_query realTemplate = ssserver.mod.decode(firstReturnPack.str, firstReturnPack.length, nextHop);

    // First data pack done.

    fd_set rset;
    FD_ZERO(&rset);
    while(true)
    {
        FD_SET(thisHop, &rset);
        FD_SET(nextHop, &rset);
        if(SOCKET_ERROR == select(max(thisHop, nextHop) + 1, &rset, NULL, NULL, NULL))
            sysdie("Select failed");
        if(FD_ISSET(thisHop, &rset))
        {
            //read from thisHop and write to nextHop
            void *bridgeBuffer = NULL;
            auto ret = readall(thisHop, &bridgeBuffer, 0);
            if(ret == SOCKET_ERROR) { free(bridgeBuffer); sysdie("read failed"); }
            if(ret == 0) goto _close_conn;
            auto encodedDat = ssserver.mod.encode((char *)bridgeBuffer, ret, nextHop, realTemplate);
            unique_cstring_ptr encodedDatStr(encodedDat.str, &std::free);
            auto lengthBackup = encodedDat.length;
            encodedDat.length = htonl(encodedDat.length);
            debug(3) std::cout << "Data pack to send: len=" << lengthBackup << std::endl;
            debug(4) std::cout << NetLib::printData(encodedDatStr.get(), lengthBackup) << std::endl;
            if(send(nextHop, (char *)&encodedDat.length, sizeof(encodedDat.length), 0/*MSG_NOSIGNAL*/) == SOCKET_ERROR) sysdie("write failed");
            if(send(nextHop, encodedDatStr.get(), lengthBackup, 0/*MSG_NOSIGNAL*/) == SOCKET_ERROR) sysdie("write failed");
        }
        if(FD_ISSET(nextHop, &rset))
        {
            //read from nextHop and write to thisHop
            uint32_t binLen = 0;
            if(readn(nextHop, &binLen, 4) == SOCKET_ERROR) sysdie("Failed to read");
            binLen = ntohl(binLen);
            if(binLen == 0) goto _close_conn;
            void *bridgeBuffer = malloc(binLen);
            if(readn(nextHop, bridgeBuffer, binLen) == SOCKET_ERROR) { free(bridgeBuffer); sysdie("Failed to read"); }
            debug(3) std::cout << "Data pack recv:len=" << binLen << std::endl;
            debug(4) std::cout << NetLib::printData(bridgeBuffer, binLen) << std::endl;
            auto decodedDat = ssserver.mod.decode((char *)bridgeBuffer, binLen, nextHop).payload;
            auto ret = send(thisHop, decodedDat.str, decodedDat.length, 0/*MSG_NOSIGNAL*/);
            free(decodedDat.str);
            if(ret == SOCKET_ERROR) sysdie("write failed");
        }
    }
    _close_conn:
        ;
    reinforce_scope_end(___s5_pspk)
}

void s5server::_dealConnection(SOCKET connfd)
{
    auto addrPortTemplate = doConnect(connfd);
    //Now it's ok to io connfd.
    debug(1) printf("Socks5: Connection %d established. RemotePort=%d\n", connfd, addrPortTemplate.destination_port);
    passPackets(connfd, addrPortTemplate);
    return;
}

void s5server::dealConnection(SOCKET connfd)
{
    try
    {
        _dealConnection(connfd);
    }
    catch(std::exception &e)
    {
        debug(2) printf("Exception caught from child thread:");
        debug(2) printf(e.what());
        debug(2) printf("\n");
    }
    debug(1) printf("Socks5: Connection %d closed.\n", connfd);
    closesocket(connfd);
    return;
}

