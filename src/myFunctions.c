#include "../header/myFunctions.h"
void ThrowError(char *message){
	printf("%s\n",message);
	exit(0);
}
int getlines(char* docName){
	/*source:
	https://stackoverflow.com/questions/14002954/c-programming-how-to-read-the-whole-file-contents-into-a-buffer*/
	FILE* myfile = fopen(docName, "r");
	int ch, lines = 0;
	do{
	    ch = fgetc(myfile);
	    if(ch == '\n') lines++;
	}while (ch != EOF);
	fclose(myfile);
	return lines;
}
int getDigits(int num){
	int _ret=1;
	if(num>0) _ret=floor(log10(abs(num)))+1;
	return _ret;
}
//source: https://stackoverflow.com/questions/174531/easiest-way-to-get-files-contents-in-c
char *readFile(char *filename) {
    FILE *f = fopen(filename, "rt");
    fseek(f, 0, SEEK_END);
    long length = ftell(f);
    fseek(f, 0, SEEK_SET);
    char *buffer = (char *) malloc(length + 1);
    buffer[length] = '\0';
    fread(buffer, 1, length, f);
    fclose(f);
    return buffer;
}

/*set up the names for each pipe, ascending order*/
char ** makeNumberedArray(char *base,int num,int start){
	int digits;
	char **array=malloc(num*sizeof(char*));
	for( int i=0;i<num;i++) {
		digits=getDigits(i);
		array[i]=malloc((strlen(base)+digits+1)*sizeof(char));
	}
	for(int i=0;i<num;i++){
		digits=getDigits(i);
		sprintf(array[i], "%s%d",base, ++start);
	}
	return array;
}

char ** checkOptions(char *argc[],char* option1,char*option2){
	long val=0;
	char **retArray=malloc(2*sizeof(char*));
	int Workers=3; //default value 3
	char *tempPtr=NULL,*docName;
	if(strcmp(argc[1],"-d")==0){
		if(strcmp(argc[3],"-w")!=0) ThrowError("Execution command failed");
		val = strtol(argc[2],&tempPtr,10);
		/*if not string*/
		if(! ((tempPtr == argc[2]) || (*tempPtr != '\0')) ) ThrowError("Invalid file name"); /*not a string*/
		docName= argc[2];
		if(argc[4]!=NULL){ //if limit is given 
			val= strtol(argc[4],&tempPtr,10);
			/*if not number*/
			/*source: 
			https://stackoverflow.com/questions/17292545/how-to-check-if-the-input-is-a-number-or-not-in-c*/
			if ((tempPtr == argc[4]) || (*tempPtr != '\0'))	ThrowError("Workers number invalid");
			Workers= atoi(argc[4]);
			if(Workers<=0) ThrowError("Workers must be at least 1");
		}
	}
	else if(strcmp(argc[1],"-w")==0){
		if(strcmp(argc[2],"-d")==0){ //no limit is given, default 10
			val = strtol(argc[3],&tempPtr,10);
			if(! ((tempPtr == argc[3]) || (*tempPtr != '\0')) ) ThrowError("Invalid file name"); /*not a string*/
			docName= argc[3];
		}
		else{ //limit is given
			if(strcmp(argc[3],"-d")!=0) ThrowError("Execution command failed");
			/*check if arguments are ok*/
			val = strtol(argc[4],&tempPtr,10);
			/*if not string*/
			if(! ((tempPtr == argc[4]) || (*tempPtr != '\0')) ) ThrowError("Invalid file name"); /*not a string*/
			docName= argc[4];

			val= strtol(argc[2],&tempPtr,10);
			/*if not number*/
			if ((tempPtr == argc[2]) || (*tempPtr != '\0')) ThrowError("Workers number invalid");
			Workers= atoi(argc[2]);
			if(Workers<=0) ThrowError("Workers must be at least 1");
		}

	}
	else ThrowError("Execution command failed3");
	retArray[0]=malloc((strlen(docName)+1)*sizeof(char));
	strcpy(retArray[0],docName);
	retArray[1]=malloc((getDigits(Workers)+1)*sizeof(char));
	sprintf(retArray[1],"%d",Workers);
	return retArray;
}


char ** getContent(int* linesPerProcess,int Workers,char *content,int lines){
	char **retContent=malloc(Workers*sizeof(char*));
	char *copy_content=malloc((strlen(content)+1)*sizeof(char));
	strcpy(copy_content,content);
	for(int i=0;i<Workers;i++){
		char **temp_content=malloc(linesPerProcess[i]*sizeof(char*));
		temp_content[0]=strtok(content,"\n");

		for(int k=1;k<linesPerProcess[i];k++){
			temp_content[k]=strtok(NULL,"\n");
		}
		char *tmp;
		/*each line in temp_content..merge them*/
		int total_size=1;
		for(int k=0;k<linesPerProcess[i];k++) total_size+=strlen(temp_content[k])+1;
		retContent[i]=malloc((total_size+1)*sizeof(char));
		strcpy(retContent[i],temp_content[0]);

		for(int k=1;k<linesPerProcess[i];k++){
			strcat(retContent[i],"\n");
			strcat(retContent[i],temp_content[k]);
		}

		if(i<Workers-1){ 
			//i dont care for modification at last loop
			strcpy(content,copy_content);
			for(int k=0;k<linesPerProcess[i];k++){
				tmp=strchr(content,'\n');
				content=tmp;
	    		memmove(content, content+1, strlen(content));
			}
			memcpy(copy_content,content,strlen(content)+1);
			copy_content[strlen(content)]='\0';
		}

		free(temp_content);
	}

	free(copy_content);
	return retContent;

}

void reloadWorker(int Workers,int pos,char **ReadPipes,char **Writepipes,char**contentPerProcess,int*fdSnd){
	pid_t pid=fork();
	if(pid==0){		
		TriggerWorker(Writepipes[pos],ReadPipes[pos],pos);
	}
	else if(pid>0){
		write(fdSnd[pos],contentPerProcess[pos],strlen(contentPerProcess[pos])+1);
	}

}

void TriggerWorker(char* Writepipe,char* ReadPipe,int num){
	char *number=malloc((getDigits(num)+1)*sizeof(char));
	sprintf(number,"%d",num);
	char *array[]={"./Worker",Writepipe,ReadPipe,number,NULL};
	execvp(array[0],array);
}


Message* Get_Command(){
	char* mystring;
	int i=0,cnt=0;
	printf("please give command..\n");
	/*at input after given arguments must be NO space */
	Stack *mystack=Stack_CreateStack(12);//10 is the maximum amount of elements
	do{
		if(cnt>0) {
			printf("your command is invalid, please give again..\n");
			Stack_Destroy(mystack);
			mystack=Stack_CreateStack(12);
		}
		if(scanf("%ms",&mystring)!=1){
			 free(mystring);
			 ThrowError("Scanf failed");
		}
		if(strcmp(mystring,"/exit")==0) {
			Message *exitMessg;
			exitMessg=malloc(sizeof(Message));
			exitMessg->deadline=-1.0;
			exitMessg->words="/exit";
			free(mystring);
			Stack_Destroy(mystack);
			return exitMessg;
		}
		if(strcmp(mystring,"/maxcount")==0) {
			free(mystring);
			if(scanf("%ms",&mystring)!=1){
					free(mystring);
					ThrowError("Scanf failed");
			}
			Message *retMessg;
			retMessg=malloc(sizeof(Message));
			retMessg->deadline=0;
			retMessg->words=malloc((strlen("/maxcount")+strlen(mystring)+2)*sizeof(char));
			strcpy(retMessg->words,"/maxcount");
			strcat(retMessg->words," ");
			strcat(retMessg->words,mystring);
			free(mystring);
			Stack_Destroy(mystack);
			return retMessg;

		}
		else if(strcmp(mystring,"/mincount")==0){
			free(mystring);
			if(scanf("%ms",&mystring)!=1){
					free(mystring);
					ThrowError("Scanf failed");
			}
			Message *retMessg;
			retMessg=malloc(sizeof(Message));
			retMessg->deadline=0;
			retMessg->words=malloc((strlen("/mincount")+strlen(mystring)+2)*sizeof(char));
			strcpy(retMessg->words,"/mincount");
			strcat(retMessg->words," ");
			strcat(retMessg->words,mystring);
			free(mystring);
			Stack_Destroy(mystack);
			return retMessg;
		}
		else if(strcmp(mystring,"/wc")==0){
			free(mystring);
			Message *retMessg;
			retMessg=malloc(sizeof(Message));
			retMessg->deadline=0;
			retMessg->words=malloc((strlen("/wc")+1)*sizeof(char));
			strcpy(retMessg->words,"/wc");
			Stack_Destroy(mystack);
			return retMessg;
		}
		else{ //search
			while(i++<10){//if no entries given program keeps waiting 
				Stack_push(mystack,mystring);	
				char ch=getchar();
				free(mystring);
				if(ch=='\n') break;
				if(scanf("%ms",&mystring)!=1){
					free(mystring);
					ThrowError("Scanf failed");
				}
			}
		}
		
		cnt++;
	}while( CheckStack(mystack)==-1 );
	cnt=0;
/*--------------------- here we have search --------------------------*/
	char * popped;
	Message* mymessage; //initialize the message to be sent
	mymessage=malloc(sizeof(Message));

	Stack_pop(mystack,&popped); //pop deadline
	char *deadline_char=malloc((strlen(popped)+1)*sizeof(char));
	strcpy(deadline_char,popped);
	mymessage->deadline=atof(deadline_char);
	free(popped);

	Stack_pop(mystack,&popped); //pop -d
	free(deadline_char);
	free(popped);

	int numOfWords=Stack_GetSize(mystack); //how many words given to query
	char ** query_words=malloc(numOfWords*sizeof(char*)); //array containing the words
	int loops=Stack_GetSize(mystack);
	while((Stack_pop(mystack,&popped)==1)&&(cnt<loops)){// cnt>0 to ignore "/command"
		size_t char_len = strlen(popped);
		query_words[cnt]=malloc((strlen(popped)+1)*sizeof(char));
		strcpy(query_words[cnt],popped);
		cnt++;
		free(popped);
	}
	free(popped);
	Stack_Destroy(mystack);
	int total_size=0;
/*----------merge all words in order to give them to Workers---------*/
	for(int i=0;i<cnt;i++) total_size+=strlen(query_words[i])+1;
	char *merged_doc=malloc((total_size+1)*sizeof(char));
	strcpy(merged_doc,query_words[0]);
	for(int i=1;i<cnt;i++){
		strcat(merged_doc," ");
		strcat(merged_doc,query_words[i]);
	}
	mymessage->words=malloc((strlen(merged_doc)+1)*sizeof(char));
	strcpy(mymessage->words,merged_doc);

	for(int i=0;i<numOfWords;i++) free(query_words[i]);
	free(query_words);
	free(merged_doc);
	return mymessage;
}

int isNum(char*word){
	char ptr;
	ptr=word[0];
	int _ret=1;
	for(int i=0;i<strlen(word);i++){
		if((ptr < '0' || ptr > '9')) _ret=0;
		ptr++;
	}
	return _ret;
}

double my_clock(void) {
  struct timeval t;
  gettimeofday(&t, NULL);
  return (1.0e-6*t.tv_usec + t.tv_sec);
}

int CountWords(char *text,int len){
	int count=0;
	for(int i=0;i<=len;i++){
		if(text[i]==' '|| text[i]=='\0' || text[i]=='\t'){
			if(i==0) continue;
			if(text[i-1]!=' ' && text[i-1]!='\0' && text[i-1]!='\t'){
		 		count++;
	 		}
		}
	}
	return count;
}




void CountWordLength(char *text,int*sizes){
	/*source:
	http://www.includehelp.com/c-programs/read-a-string-and-print-the-length-of-the-each-word.aspx*/ 
	/*take a whole text, and count length of each word, so as to allocate the
	proper amount of space of each word*/
	int new_ctr=0,j=0;
	for(int i=0;i<=strlen(text);i++){
		if(text[i]==' '|| text[i]=='\0'){
				if(text[i-1]!=' ' && text[i-1]!='\0'){
					sizes[new_ctr++]=j;
					j=0; 
				}
		}
		else j++;
	}	
}

void SaveWords(char ** wordSep,char* doc){
	int i,j=0,ctr=0;
	/*source:
	https://www.w3resource.com/c-programming-exercises/string/c-string-exercise-31.php*/
//the technique i use is i read characters and define a word when 
//whitespace is found, therefore there is the danger of having
//consequtive whitespaces and allocating a word, hence the condition
	for(i=0;i<=strlen(doc);i++){
		if(doc[i]==' '|| doc[i]=='\0' || doc[i]=='\t'){
			if(doc[i-1]!=' ' && doc[i-1]!='\0' && doc[i]!='\t'){ //avoid allocating spaces as words
				wordSep[ctr++][j]='\0';
				j=0; //set index 0 for next word
			}
		}
		else{
			wordSep[ctr][j]=doc[i];
			j++;
		}
	}
}

void removeLogs(){
	remove("log");
	mkdir("log",0700);
	DIR *dir;
	struct dirent *ent;
	char *directory="log";
	if((dir=opendir(directory))!=NULL){
		while((ent=readdir(dir))!=NULL){
			if (strcmp(ent->d_name, ".")!=0 && strcmp(ent->d_name, "..")!=0 ){
				char *filePath=malloc((strlen("log/")+strlen(ent->d_name)+1)*sizeof(char));
				strcpy(filePath,"log/");
				strcat(filePath,ent->d_name);
				int ret=remove(filePath);
				free(filePath);
			}
		}
	}
	free(dir);
}

void WriteLog(char *fileName,char *time,char *query_type,char *word,char *paths){
	FILE* f=fopen(fileName,"a");
	//case for wc, one less entry
	if(strcmp(paths," ")==0)fprintf(f,"%s : %s : %s\n",time,query_type,word);
	else fprintf(f,"%s : %s : %s : %s\n",time,query_type,word,paths);
	fclose(f);
}

/*https://stackoverflow.com/questions/20342559/how-to-cut-part-of-a-string-in-c*/
int str_cut(char *str, int begin, int len){
    int l = strlen(str);
    if (len < 0) len = l - begin;
    if (begin + len > l) len = l - begin;
    memmove(str + begin, str + begin + len, l - len + 1);

    return len;
}