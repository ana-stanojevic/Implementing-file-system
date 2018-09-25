#include "of.h"


OpenFiles::OpenFiles() {
	sem = CreateEvent(NULL, 1, 1, NULL); 
	mutex = CreateSemaphore(NULL, 1, 32, NULL); 
	first = 0;                                  
	last = 0;
}

OpenFiles::~OpenFiles() {
	CloseHandle(sem); 
	CloseHandle(mutex);
	
	while (first) {
		Node *current = first;
		first = first->next;
		delete current;
	}
	first = 0;
	last = 0;
}

Node* OpenFiles::add(char *fn) {
	Node *node = find(fn);
	if (node != 0) return node;

	if (first) {
		last->next = new Node(fn);
		last = last->next;
	} else {
		first = new Node(fn);
		last = first;
	}
	return last;
}

Node* OpenFiles::find(char *fn) {
	Node *curr = first;
	while (curr) {
		if (strcmp(curr->fname, fn) == 0) return curr;
		curr = curr->next;
	}
	return 0;
}

void OpenFiles::updateOF(char* fn) {
	wait(mutex);
	Node *node = add(fn);
	ResetEvent(sem);
	signal(mutex); 
	node->locker->Enter();
	node->isUsed = true;
}


void OpenFiles::closeOF(char* fn) {
	wait(mutex);
	Node *node = find(fn);
	node->isUsed = false;
	SetEvent(sem);
	node->locker->Leave();
	signal(mutex);
}

bool OpenFiles::allClosedOF() {
	wait(mutex); 
	Node *curr = first;
	while (curr) {
		if (curr->isUsed != 0) { 
			signal(mutex);
			return 0;
		}
		curr = curr->next;
	}
	signal(mutex);
	return 1;
}