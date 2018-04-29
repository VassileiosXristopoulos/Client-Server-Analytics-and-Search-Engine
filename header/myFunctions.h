#ifndef MYFUNCTIONS_H
#define MYFUNCTIONS_H
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
#include <math.h>
#include <sys/time.h>
#include <time.h>
#include <dirent.h>
#include "Stack.h"
#include "trie.h"
#include "hashTable.h"
#include "Queue.h"
typedef struct Message Message;
struct Message{
	char * words;
	double deadline;
};
int getDigits(int);
int getlines(char*);
char** makeNumberedArray(char*,int,int);
char** checkOptions(char **,char*,char*);
void ThrowError(char*);
char* readFile(char*);
char ** getContent(int*,int,char*,int);
void reloadWorker(int,int,char**,char**,char**,int*);
void TriggerWorker(char* ,char*,int);
int isNum(char*);
Message* Get_Command(void);
double my_clock(void);
int CountWords(char *,int );
void CountWordLength(char *,int*);
void SaveWords(char ** ,char* );
void removeLogs();
void WriteLog(char*,char*,char*,char*,char*);
int str_cut(char *, int , int );
#endif