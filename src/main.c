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
#include "../header/myFunctions.h"
#include "../header/JobExecutor.h"

int main(int argv,char *argc[]){
	removeLogs(); //removing previous logfiles if any
	char **returned=checkOptions(argc,"-d","-w"); //checking for errors in execution command
	char *docName=returned[0];
	int Workers=atoi(returned[1]);
	
	int lines=getlines(docName); //counting how many lines the path file has 
	/*generate so many workers, so as to give one directory to each one*/
	if(lines<Workers) Workers=lines;
	remove("pipes");
	mkdir("pipes",0700);
	char **fifo_array=makeNumberedArray("./pipes/f_",Workers,1);//Workers send-executor receives
	char **fifo_array2=makeNumberedArray("./pipes/fn_",Workers,1);//Executor sends-Workers receive

	pid_t pid;
	for(int i=0;i<Workers;i++) mkfifo(fifo_array[i], 0666);
	for(int i=0;i<Workers;i++) mkfifo(fifo_array2[i], 0666);

	pid_t temp_pid=fork();
	if(temp_pid!=0){ //   Job Executor
		for(int i=1;i<Workers;i++){
			pid=fork();
			if(pid==0){
				TriggerWorker(fifo_array[i],fifo_array2[i],i);
				exit(0);
			}
		}
	}
	else{
		TriggerWorker(fifo_array[0],fifo_array2[0],0);
		exit(0);
	}
	JobExecutor(lines,fifo_array,Workers,docName,fifo_array2);
	for(int i=0;i<Workers;i++){
		free(fifo_array[i]);
		free(fifo_array2[i]);
	}
	free(returned[0]);
	free(returned[1]);
	free(returned);
	free(fifo_array);
	free(fifo_array2);
}