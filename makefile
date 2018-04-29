CC = gcc # Compiler

all:src/jobExecutor src/Worker

src/jobExecutor: src/main.o src/JobExecutor.o src/myFunctions.o src/Stack.o src/trie.o src/postingList.o src/docKey.o src/HashTable.o src/Queue.o
	$(CC) -o jobExecutor main.o JobExecutor.o myFunctions.o Stack.o trie.o postingList.o docKey.o HashTable.o Queue.o -lm

src/main.o:
	$(CC) -c -g3 src/main.c

src/JobExecutor.o:
	$(CC) -c -g3 src/JobExecutor.c

src/docKey.o:
	$(CC) -c -g3 src/docKey.c

src/Queue.o:
	$(CC) -c -g3 src/Queue.c

src/Stack.o:
	$(CC) -c -g3 src/Stack.c

src/Worker: src/Worker.o src/myFunctions.o src/docKey.o src/HashTable.o src/trie.o src/postingList.o src/Queue.o src/WorkerFunctions.o
	$(CC) -o Worker Worker.o myFunctions.o Stack.o docKey.o HashTable.o trie.o postingList.o Queue.o WorkerFunctions.o -lm

src/WorkerFunctions.o:
	$(CC) -c -g3 src/WorkerFunctions.c

src/trie.o:
	$(CC) -c -g3 src/trie.c


src/myFunctions.o:
	$(CC) -c -g3 src/myFunctions.c

src/postingList.o:
	$(CC) -c -g3 src/postingList.c

src/HashTable.o:
	$(CC) -c -g3 src/HashTable.c

src/Worker.o:
	$(CC) -c -g3 src/Worker.c 

.Phony: clean

clean:
	rm -r jobExecutor main.o JobExecutor.o myFunctions.o Stack.o docKey.o HashTable.o 
	rm -r trie.o Worker.o postingList.o WorkerFunctions.o
