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
            "SnakeSocks Client 1.1.4 Windows edition (nogui)" n n
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
            "-c <config file path>\tPath to config file(default: conf\\client.conf)" n
            "-h\tShow this message" n
            "You can not run client as daemon on windows. Try launch it as service by yourself if necessary." n n
            "Published on GNU license V2." n
    );
#undef n
}

int main(int arglen, char **argv)
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
    conf.load(confPath == NULL ? "conf\\client.conf" : confPath);
    if(daemonLogFilePath) conf.daemonLogFile = daemonLogFilePath;
    if(serverAddr) conf.serverIp = serverAddr;
    if(serverPort) conf.serverPort = atoi(serverPort);
    if(passP) conf.passphrase = passP;
    if(listenAddr) conf.bindIp = listenAddr;
    if(listenPort) conf.bindPort = atoi(listenPort);
    if(debugLev) conf.debugLevel = atoi(debugLev);
    if(modPath) conf.modulePath = modPath;
	
    if(dflag)
    {
		die("Daemon mode is not currently supported on WinNT. You can launch it as service by yourself.");/*
        const char *logFile = conf.daemonLogFile.c_str();

        //redirect stdout/stderr to file
        int out = open(logFile, O_RDWR|O_CREAT|O_APPEND, 0600);
        if (-1 == out) { sysdie("Failed to open log file %s", logFile); }

        int err = open(logFile, O_RDWR|O_CREAT|O_APPEND, 0600);
        if (-1 == err) { sysdie("Failed to open log file %s", logFile); }

        int save_out = dup(fileno(stdout));
        int save_err = dup(fileno(stderr));

        if (-1 == dup2(out, fileno(stdout))) { sysdie("Failed to redirect stdout"); }
        if (-1 == dup2(err, fileno(stderr))) { sysdie("Failed to redirect stderr"); }

        if(daemon(0, 1) == -1) sysdie("Failed to launch daemon.");*/
    }

    return main_proc(conf);
}
