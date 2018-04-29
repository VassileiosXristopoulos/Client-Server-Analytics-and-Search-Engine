#ifndef JOBEXECUTOR_H
#define JOBEXECUTOR_H
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/signalfd.h>
#include <signal.h>
#include <poll.h>
 #include <sys/epoll.h>
 #include <sys/wait.h>
#include <time.h>
#include "myFunctions.h"
void JobExecutor(int,char**,int,char*,char **);
#endif