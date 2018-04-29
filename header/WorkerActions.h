#ifndef WorkerFunctions_H
#define WorkerFunctions_H
#include "hashTable.h"
#include "myFunctions.h"
void Worker_Search(int ,Trie_node *,char **,int ,char *,hash_table*,char*,char*,int*);
void Worker_Maxcount(Trie_node*,char *,int,char*,char*);
void Worker_Mincount(Trie_node*,char *,int,char*,char*);
void Worker_Wc(Trie_node*,hash_table*,int,char*,char*);
char * answerForWord(Trie_node*,char*,int**,int,char**,int,hash_table*,int*);
int CountPaths(char*);
char* getFullTime();
void SaveToTrie(char *,docKey*,Trie_node*);
#endif
