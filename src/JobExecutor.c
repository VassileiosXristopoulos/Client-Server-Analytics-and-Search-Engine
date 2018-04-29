#include "../header/JobExecutor.h"

void JobExecutor(int lines,char **readPipes,int Workers,char * docName,char **WritePipes){
    struct timeval tv;
    int retval;
    fd_set myset;
    FD_ZERO(&myset);
    int *fdRcv=malloc(Workers*sizeof(int));
    int * fdSnd=malloc(Workers*sizeof(int));
    for(int i=0;i<Workers;i++) {
    	fdSnd[i]=open(WritePipes[i],0666,O_WRONLY | O_NONBLOCK); //set up write-pipes
    	fdRcv[i]=open(readPipes[i],0666,O_RDONLY | O_NONBLOCK); //set up recieve-pipes
    	FD_SET(fdRcv[i],&myset);
    }
	int linesPerProcess=lines/Workers;
	char *content=readFile(docName);  // get whole file as one string

	/*content is "sample-text\n" so we remove the \n*/
	int length=strlen(content);
	content[length-1]='\0';
	/*-----calculate how many lines each Worker gets------*/
	int *linesPerProcess_array=malloc(Workers*sizeof(int));
	for(int k=0;k<Workers-1;k++){
		linesPerProcess_array[k]=linesPerProcess;

	}
	int difference=lines%linesPerProcess;
	if(difference==0) linesPerProcess_array[Workers-1]=linesPerProcess;
	else linesPerProcess_array[Workers-1]=linesPerProcess+difference;//give more to last worker

	char **contentPerProcess=getContent(linesPerProcess_array,Workers,content,lines);
	free(content);
	
	for(int i=0;i<Workers;i++){
		write(fdSnd[i],contentPerProcess[i],strlen(contentPerProcess[i])+1);
	}
	int nfds=fdRcv[Workers-1]+1; //biggest file desc we have plus one
	Message* command; //message is a struct wich consists of the type of query and
	// the deadline given from user (if query is search, otherwise 0) 

	int *came=malloc(Workers*sizeof(int)); //this array represents
	// i) if a respone didn't come at all (dead worker) came[i]=-1
	// ii) if a repospne came but out f deadline came[i]=0
	// iii) if a respone came in deadline came[i]=1
	int *max_array=malloc(Workers*sizeof(int)); //the max returned from each worker
	int *min_array=malloc(Workers*sizeof(int)); //the min returned from each worker 
	char **file_array=malloc(Workers*sizeof(char*)); 
	for(int i=0;i<Workers;i++){
		max_array[i]=-1;
		min_array[i]=-1;
		file_array[i]=NULL;
	}
	int wc_chars=0;
	int wc_lines=0;
	int wc_words=0;
	for(int i=0;i<Workers;i++) came[i]=-1;
		command=Get_Command();
		while(strcmp(command->words,"/exit")!=0){

			for(int i=0;i<Workers;i++) write(fdSnd[i],command->words,strlen(command->words)+1);
			kill(0,SIGCONT); //wake up all workers
			tv.tv_sec=5;
			tv.tv_usec = 0;
			double end;
			double elapsed;
			double start =my_clock();
			char *command_species=strtok(command->words," ");
			for(int k=0;k<Workers;k++){ //loop Worker-times to catch every event
				retval=select(nfds,&myset,NULL,NULL,&tv);
				end =my_clock();
				elapsed=end-start;
				if(retval>0){ // if no error is returned
					for(int i=0;i<Workers;i++){
						if(FD_ISSET(fdRcv[i],&myset)){
							int pipe_size=fpathconf(fdRcv[i], _PC_PIPE_BUF);
							int new_pipe_size=pipe_size;
							char *recieved_content=malloc((pipe_size+1)*sizeof(char));
							int r=read(fdRcv[i],recieved_content,pipe_size);
							recieved_content[pipe_size]='\0';
				/*------------------ manipulate the case of message bigger than pipe-size------------*/			
							while(r==pipe_size){
								new_pipe_size+=pipe_size;
								char* new_recieved=malloc((new_pipe_size+1)*sizeof(char));
								strcpy(new_recieved,recieved_content);
								new_recieved[new_pipe_size]='\0';
								memset(recieved_content,0,strlen(recieved_content));
								r=read(fdRcv[i],recieved_content,pipe_size);
								recieved_content[pipe_size]='\0';
								strcat(new_recieved,recieved_content);
								free(recieved_content);
								recieved_content=malloc((new_pipe_size+1)*sizeof(char));
								strcpy(recieved_content,new_recieved);
								free(new_recieved);
							}
				/*------------------------------------------------------------------------------------*/

							if(strcmp(command_species,"/maxcount")==0){
								 came[i]=1;
								if(strcmp(recieved_content,"0")!=0){
									char *filename=strtok(recieved_content," ");
									char* num=strtok(NULL," ");
									file_array[i]=malloc((strlen(filename)+1)*sizeof(char));
									strcpy(file_array[i],filename);
									max_array[i]=atoi(num);
								}

							}
							else if(strcmp(command_species,"/mincount")==0){
								came[i]=1;
								if(strcmp(recieved_content,"0")!=0){
									char *filename=strtok(recieved_content," ");
									char* num=strtok(NULL," ");
									file_array[i]=malloc((strlen(filename)+1)*sizeof(char));
									strcpy(file_array[i],filename);
									if(atoi(num)!=0){
										min_array[i]=atoi(num);
									}
								}
							}
							else if(strcmp(command_species,"/wc")==0){
								came[i]=1;
								char *chars=strtok(recieved_content," ");
								char *words=strtok(NULL," ");
								char *lines=strtok(NULL,"\n");
								wc_words+=atoi(words);
								wc_lines+=atoi(lines);
								wc_chars+=atoi(chars);
							}	
							else{
								if(strcmp(recieved_content,"-1")!=0){
									if(elapsed<command->deadline) came[i]=1; //consider that worker is in deadline
									
									printf("\n%s\n\n",recieved_content);
								}
								else came[i]=0;
							}
							
							free(recieved_content);
						}
						
					}
				}
				FD_ZERO(&myset);
    			for(int i=0;i<Workers;i++) FD_SET(fdRcv[i],&myset);
    		}

	    	int arrivals=0;
	    	for(int i=0;i<Workers;i++){
	    		if(came[i]==-1){ //if someone didn't respond at all
	    			reloadWorker(Workers,i,WritePipes,readPipes,contentPerProcess,fdSnd);
	    		}
	    		else if(came[i]==1) arrivals++;
	    	}
			if(strcmp(command_species,"/maxcount")==0){
				int max=0;
				int pos;
				for(int i=0;i<Workers;i++){
					if(max_array[i]>max){
					 max=max_array[i];
					 pos=i;
					}
					else if(max_array[i]==max){
						if(max>0){
							if(strcmp(file_array[pos],file_array[i])>0){
								max=max_array[i];
								pos=i;
							}
						}
					}
				}
				if(max>0)printf("file:%s with frequency:%d\n",file_array[pos],max );
				else printf("no response for /maxcount\n");
			}
			else if(strcmp(command_species,"/mincount")==0){
				int min,pos,count=0,flag=0;
	
				for(int i=0;i<Workers;i++){
					if(min_array[i]>0){
						min=min_array[i];
						pos=i;
						flag=1;
						break;
					}
				}
				if(flag==0) printf("no response for mincount\n");
				else{
					for(int i=1;i<Workers;i++){
						if(min_array[i]<min){
							pos=i;
							 min=min_array[i];
						}
						else if(min_array[i]==min){
							if(strcmp(file_array[pos],file_array[i])>0){
								min=min_array[i];
								pos=i;
							}
						}
					}
					printf("file:%s with frequency:%d\n",file_array[pos],min );
				}
				
			}
			else if(strcmp(command_species,"/wc")==0){
				printf("words:%d lines:%d characters:%d\n",wc_words,wc_lines,wc_chars);
				wc_chars=0;
				wc_words=0;
				wc_lines=0;
			}
			else{ //search 
				printf("%d responses\n",arrivals );
			}
			printf("---------continued\n");
			
			free(command->words);
			free(command);

			command=Get_Command();

			FD_ZERO(&myset);
	    	for(int i=0;i<Workers;i++){
	    		came[i]=-1;
	    		FD_SET(fdRcv[i],&myset);
				max_array[i]=0;
				min_array[i]=0;
				if(file_array[i]!=NULL) free(file_array[i]);
				file_array[i]=NULL;
			}

    	}
    for(int i=0;i<Workers;i++) write(fdSnd[i],"/exit",strlen("/exit")+1);

    kill(0,SIGCONT);
	/*----------------Get matches from each Worker---------------------*/
	tv.tv_sec=5;
	tv.tv_usec = 0;
	for(int i=0;i<Workers;i++){
		retval=select(nfds,&myset,NULL,NULL,&tv);
		if(retval>0){
			for(int j=0;j<Workers;j++){
				if(FD_ISSET(fdRcv[j],&myset)){
					int pipe_size=fpathconf(fdRcv[j], _PC_PIPE_BUF);
					char *recieved_content=malloc((pipe_size+1)*sizeof(char));
					int r=read(fdRcv[j],recieved_content,pipe_size);
					printf("Worker:%d made %d matches\n",j,atoi(recieved_content) );
					free(recieved_content);
				}
			}
		}
		FD_ZERO(&myset);
    	for(int i=0;i<Workers;i++) FD_SET(fdRcv[i],&myset);
	}
	/*----------------------------------------------------------------*/
    waitpid(-1,NULL,0);

    free(command);
    for(int i=0;i<Workers;i++) free(contentPerProcess[i]);
    free(contentPerProcess);
	for(int i=0;i<Workers;i++) {
    	close(fdRcv[i]); 
    	close(fdSnd[i]);
    }
    free(fdRcv);
	free(fdSnd);
	free(max_array);
	free(min_array);
	free(linesPerProcess_array);
	free(came);
	free(file_array);
	return ;
}
	



