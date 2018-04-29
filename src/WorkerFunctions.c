#include "../header/WorkerFunctions.h"


void Worker_Search(int numOFfiles,Trie_node *Root,char **path_array,int fdSnd,char*recieved_content,hash_table* hashTable,char*time,char*logFIle,int*matches){
	int numOfWords=CountWords(recieved_content,strlen(recieved_content));

	char **query_words=malloc(numOfWords*sizeof(char*));
	query_words[0]=strtok(recieved_content," ");
	for(int k=1;k<numOfWords;k++){
		query_words[k]=strtok(NULL," ");
	}
	//pathsForWord is an int array with rows being indexes of query words 
	//and columns being files, so as to know in which files each word is found
	int **pathsForWord=malloc(numOfWords*sizeof(int*)); //-1 if word doesn't have path, 1 otherwise
														//indexes of array are the indexes of
														//query_words and path_array
	for(int i=0;i<numOfWords;i++){
		pathsForWord[i]=malloc(numOFfiles*sizeof(int));
		for(int k=0;k<numOFfiles;k++) pathsForWord[i][k]=-1;
	}
	
	char **answers_array=malloc(numOFfiles*sizeof(char*));
	for(int k=0;k<numOFfiles;k++){
		char *answer=answerForWord(Root,path_array[k],pathsForWord,k,query_words,numOfWords,hashTable,matches);
		if(answer==NULL) {
			answers_array[k]=NULL;
			continue;
		}
		answers_array[k]=malloc((strlen(answer)+1)*sizeof(char));
		strcpy(answers_array[k],answer);
		free(answer);
	}

	int total_len=0;
	for(int i=0;i<numOFfiles;i++){
		if(answers_array[i]!=NULL) {
			total_len+=strlen(answers_array[i])+2;
		}
	}
	char* answer=malloc((total_len+1)*sizeof(char));

	int pos,flag=0;
	for(int i=0;i<numOFfiles;i++){
		if(answers_array[i]!=NULL){
			pos=i;
			flag=1;
			break;
		}
	}

	if(flag==0) {
		write(fdSnd,"-1",3);
	}
	else{
		strcpy(answer,answers_array[pos]);
		for(int i=pos+1;i<numOFfiles;i++){
			if(answers_array[i]!=NULL){
				strcat(answer,"\n");
				strcat(answer,answers_array[i]);
				strcat(answer,"\n");
			}
		}
		int bytes_written=write(fdSnd,answer,strlen(answer)+1);
		while(bytes_written<strlen(answer)+1){
			str_cut(answer,bytes_written,strlen(answer));
			bytes_written=write(fdSnd,answer,strlen(answer)+1);
		}
	}
	free(answer);

	/*----------manipulate log file---------------*/
	for(int i=0;i<numOfWords;i++){
		int total_pathSize=0;
		for(int j=0;j<numOFfiles;j++){
			if(pathsForWord[i][j]==1){
				total_pathSize+=strlen(path_array[j])+1;
			}
		}
		char *paths=malloc((total_pathSize+2)*sizeof(char));
		int flag=0;
		for(int j=0;j<numOFfiles;j++){
			if(pathsForWord[i][j]==1){
				if(flag==0){
					strcpy(paths,path_array[j]);
					flag=1;
				}
				else strcat(paths,path_array[j]);
				strcat(paths," ");
			}
		}
		if(flag==0) strcpy(paths," ");
		WriteLog(logFIle,time,"search",query_words[i],paths); 
		free(paths);

	}	
	/*--------------------------------------------------*/

	for(int i=0;i<numOFfiles;i++){
		free(answers_array[i]);
	}
	free(query_words);
	free(answers_array);
	for(int i=0;i<numOfWords;i++){
		free(pathsForWord[i]);
	}
	free(pathsForWord);
}


void Worker_Maxcount(Trie_node* Root,char *word,int fdSnd,char*time,char*logFIle){
	Trie_node* node =Trie_Search(word,Root);
	if(node==NULL){
		char *answer="0";
		write(fdSnd,answer,strlen(answer)+1);
		WriteLog(logFIle,time,"maxcount",word," ");
		return;
	}
	int max=0;
	char *file;
	PostingList *iterator=node->pl_ptr;
	while(iterator!=NULL){
		if(iterator->word_frequency>max){
			max=iterator->word_frequency;
			file=iterator->key->filePath;
		}
		iterator=iterator->next;
	}

	char *answer=malloc((strlen(file)+getDigits(max)+2)*sizeof(char));
	char *num=malloc((getDigits(max)+1)*sizeof(char));
	sprintf(num,"%d",max);
	strcpy(answer,file);
	strcat(answer," ");
	strcat(answer,num);
	write(fdSnd,answer,strlen(answer)+1);

	WriteLog(logFIle,time,"maxcount",word,file);
	free(num);
	free(answer);
}

void Worker_Mincount(Trie_node* Root,char *word,int fdSnd,char*time,char*logFIle){
	Trie_node* node =Trie_Search(word,Root);
	if(node==NULL){
		char *answer="0";
		write(fdSnd,answer,strlen(answer)+1);
		WriteLog(logFIle,time,"mincount",word," ");
		return;
	}
	int min;
	char *file;
	PostingList *iterator=node->pl_ptr;
	if(iterator==NULL){
		write(fdSnd,"nofiledetected 0",1);
		return;
	}
	min=iterator->word_frequency;
	file=iterator->key->filePath;
	iterator=iterator->next;
	while(iterator!=NULL){
		if(iterator->word_frequency<min){
			min=iterator->word_frequency;
			file=iterator->key->filePath;
		}
		iterator=iterator->next;
	}

	char *answer=malloc((strlen(file)+getDigits(min)+2)*sizeof(char));
	char *num=malloc((getDigits(min)+1)*sizeof(char));
	sprintf(num,"%d",min);
	strcpy(answer,file);
	strcat(answer," ");
	strcat(answer,num);
	write(fdSnd,answer,strlen(answer)+1);

	WriteLog(logFIle,time,"mincount",word,file);
	free(num);
	free(answer);
}

void Worker_Wc(Trie_node* Root,hash_table *hashTable,int fdSnd,char*time,char*logFIle){
	int total_words=Hash_GetWords(hashTable);
	int total_lines=Hash_GetAllLines(hashTable);
	int total_chars=Hash_GetChars(hashTable);
	int size=getDigits(total_words)+getDigits(total_lines)+getDigits(total_chars);
	char *answer=malloc((size+4)*sizeof(char));
	sprintf(answer,"%d %d %d",total_chars,total_words,total_lines);
	write(fdSnd,answer,strlen(answer)+1);

	WriteLog(logFIle,time,"wc",answer," ");
	free(answer);
}

char * answerForWord(Trie_node* Root,char *path,int** wordpath,int pathNum,char** query_words,int numOfWords,hash_table*hashTable,int*matches){
	int data_size=0;
	Queue *myQueue=NULL;
	
	for(int i=0;i<numOfWords;i++){	
		Trie_node * mynode=Trie_Search(query_words[i],Root);
		if(mynode==NULL) continue;
		PostingList *iterator=mynode->pl_ptr;
		while(iterator!=NULL){
			if(strcmp(iterator->key->filePath,path)==0){
				wordpath[i][pathNum]=1;
				for(int k=0;k<iterator->key->size;k++){
				/*---Hash_Search returns the content of the line searched----*/
					char *contex=Hash_Search(path,iterator->key->key[k],hashTable,matches);
					if(contex!=NULL){
						int entrySize=getDigits(iterator->key->key[k])+strlen(contex)+2;
						char *entry=malloc(entrySize*sizeof(char));
						char *key=malloc((getDigits(iterator->key->key[k])+1)*sizeof(char));
						sprintf(key,"%d",iterator->key->key[k]);
						strcpy(entry,key);
						strcat(entry," ");
						strcat(entry,contex);
						//insert to Queue
						myQueue=Queue_Insert(myQueue,entry,&data_size);
						free(key);
						free(entry);
					}
				}

				break;
			}
			iterator=iterator->next;
		}
	}

	char *answer=malloc((data_size+strlen(path)+1)*sizeof(char));
	strcpy(answer,path);
	Queue* iterator=myQueue;
	if(myQueue==NULL) return NULL;
	//pop from queue, merge,return.
	while(iterator!=NULL){
		strcat(answer,"\n");
		strcat(answer,iterator->content);
		iterator=iterator->next;
	}
	Queue_Destroy(myQueue);
	return answer;
}

int CountPaths(char *text){
	int count=0;
	for(int i=0;i<=strlen(text);i++){
		if(text[i]=='\n'|| text[i]=='\0'){
			if(text[i-1]!='\n'){
		 		count++;
	 		}
		}
	}
	return count;
}

//https://stackoverflow.com/questions/1442116/how-to-get-the-date-and-time-values-in-a-c-program?utm_medium=organic&utm_source=google_rich_qa&utm_campaign=google_rich_qa
char* getFullTime(){
	time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    char *time=asctime(tm);
    time[strlen(time)-1]=0;
    return time;
}

void SaveToTrie(char *text,docKey *mykey,Trie_node *Root){
	int ctr=CountWords(text,strlen(text));
	char ** wordSep;
	int *sizes=malloc(ctr*sizeof(int));
	/* counting the length of each word */
	CountWordLength(text,sizes);
	/*allocating the array on which the words will be stored */
	wordSep=malloc(ctr*sizeof(char*));
	for(int i=0;i<ctr;i++){ 
		wordSep[i]=malloc((sizes[i]+1)*sizeof(char));
	}
	SaveWords(wordSep,text); //saving each word to wordSep
	for(int i=0;i<ctr;i++){
		Trie_Insert(wordSep[i],mykey,Root);
	}
	for(int i=0;i<ctr;i++){

		free(wordSep[i]);
	}
		free(wordSep);
		free(sizes);
}