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
            "SnakeSocks Client 1.1.3 Windows edition (nogui)" n n
            "Usage: snakesockscli [OPTION...]" n
            "A extensible proxy which supports module-defined protocol behavior." n
            "You can install and run module easily from configuration file," n
            "to determine how packets are encrypted, transmitted, etc." n n
            "Options:" n
            "-c <config file path>\tPath to config file(default: /etc/snakesocks/conf/client.conf)" n
            "-h\tShow this message" n
            "You can not run client as daemon on windows. You can launch it as service by yourself." n n
            "Published on GNU license V2." n
    );
#undef n
}

int main(int arglen, char **argv)
{
    int dflag = 0;
    char *confPath = NULL;
    char *daemonLogFilePath = NULL;
    int index;
    int c;

    opterr = 0;

    while ((c = getopt (arglen, argv, "c:dhl:")) != -1)
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
    conf.load(confPath == NULL ? "/etc/snakesocks/conf/client.conf" : confPath);
    if(daemonLogFilePath != NULL) conf.daemonLogFile = daemonLogFilePath;

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
