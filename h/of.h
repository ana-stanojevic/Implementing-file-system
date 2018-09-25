#ifndef _of_h_
#define _of_h_

#include "fairlock.h"

#include "fairlock.h"
#include <Windows.h>

struct Node {
	char *fname;
	bool isUsed;
	FairLock* locker;
	Node *next;

	Node(char *fn) {
		fname = new char[strlen(fn) + 1];
		strcpy(fname, fn);
		isUsed = false;
		locker = new FairLock();
		next = 0;
	}

	~Node() {
		delete fname;
		delete locker;
	}
};

class OpenFiles { 
public:
	OpenFiles();
	~OpenFiles();

	Node* add(char *fn);
	Node* find(char *fn);
	
	void updateOF(char *fn);
	void closeOF(char *fn);

	bool allClosedOF();

	HANDLE sem, mutex;

private:
	Node *first, *last;
};

#endif
