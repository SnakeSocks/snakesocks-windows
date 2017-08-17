#include "socks5.hpp"
#include "debug.hpp"
#include "config.hpp"
#include "syserr.hpp"

#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
int _runtime_debugLevel;

using namespace std;

int main_proc(config &conf)
{
    _runtime_debugLevel = conf.debugLevel;
    cout << "Setting debug level = " << _runtime_debugLevel << endl;
    client_module mod(conf.serverIp, conf.serverPort, conf.passphrase, conf.modulePath);
    s5server sss(conf.bindIp, conf.bindPort, tunnel(std::move(mod)));
    sss.run();
    return 0;
}

void display_usage()
{
#define n "\n"
    printf(
            "SnakeSocks Client 1.1.4 Special edition for WIN32GUI | no stderr and no exception" n n
            "Usage: snakesockscli [OPTION...]" n
            "A extensible proxy which supports module-defined protocol behavior." n
            "You can install and run module easily from configuration file," n
            "to determine how packets are encrypted, transmitted, etc." n n
            "Options:" n
            "-s <server address>\tServer ip or domain" n
            "-p <server port>\tServer listening port" n
            "-k <passphrase>\tPassphrase" n
            "-L <listen address>\tLocal address that socks5 server listens on" n
            "-P <listen port>\tLocal port that socks5 server listens on" n
            "-D <debug level>\tDebug level to set" n
            "-m <path to module>\tModule path" n
            n
            "-h\tShow this message" n
            "NEVER launch this program directly!" n n
            "Published on GNU license V3." n
    );
#undef n
}

int ____main(int arglen, char **argv)
{
    int dflag = 0;
    char *confPath = NULL;
    char *daemonLogFilePath = NULL;
    char *serverAddr = NULL, *serverPort = NULL, *passP = NULL, *listenAddr = NULL, *listenPort = NULL;
    char *debugLev = NULL, *modPath = NULL;
    int index;
    int c;

    opterr = 0;

    while ((c = getopt (arglen, argv, "c:dhl:s:p:k:L:P:D:m:")) != -1)
        switch (c)
        {
            case 'd':
                dflag = 1;
                break;
            case 'c':
                confPath = optarg;
                break;
            case 'h':
                display_usage();
                exit(0);
                break;
            case 'l':
                daemonLogFilePath = optarg;
                break;
            case 's':
                serverAddr = optarg;
                break;
            case 'p':
                serverPort = optarg;
                break;
            case 'k':
                passP = optarg;
                break;
            case 'L':
                listenAddr = optarg;
                break;
            case 'P':
                listenPort = optarg;
                break;
            case 'D':
                debugLev = optarg;
                break;
            case 'm':
                modPath = optarg;
                break;
            case '?':
                if (optopt == 'c')
                    printf("Option -%c requires an argument.\n", optopt);
                else if (isprint (optopt))
                    printf("Unknown option `-%c'.\n", optopt);
                else
                    printf("Unknown option character `\\x%x'.\n", optopt);
                display_usage();
                return 1;
            default:
                abort();
        }

    config conf;
    if(daemonLogFilePath) die("Daemon file path not supported.");
    if(!serverAddr) die("servAddr not set."); conf.serverIp = serverAddr;
    if(!serverPort) die("servPort not set."); conf.serverPort = atoi(serverPort);
    if(!passP) die("passphrase not set."); conf.passphrase = passP;
    if(!listenAddr) die("bindAddr not set."); conf.bindIp = listenAddr;
    if(!listenPort) die("bindPort not set."); conf.bindPort = atoi(listenPort);
    if(!debugLev) die("debugLevel not set."); conf.debugLevel = atoi(debugLev);
    if(!modPath) die("modulePath not set."); conf.modulePath = modPath;
	
    if(dflag) die("Daemon mode is not currently supported on WinNT. You can launch it as service by yourself.");

    return main_proc(conf);
}

int main(int l, char **p)
{
	try{
		____main(l, p);
	}
	catch(std::exception &e){
		std::cout << "Exception uncaught: " << e.what() << "\n";
	}
	catch(...){
		std::cout << "Unknown exception...... Some exception doesn't inherit std::exception? qaq...\n";
	}
}
