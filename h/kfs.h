#ifndef _kfs_h_
#define _kfs_h 

#include "part.h" 
#include "fat.h" 
#include "rootd.h" 
#include "of.h"
#include "file.h" 
#include "kfile.h" 
#include "fs.h"

struct allPartition {
	RootDir* rd;
	FAT *ft;
	OpenFiles *o;
	bool formatMode;
	bool unmountMode;
	Partition *p;
	HANDLE mutex;

	allPartition () {
		p = 0;
		ft = 0; 
		rd = 0; 
		o = 0; 
		formatMode = false;
		unmountMode = false;
		mutex = CreateSemaphore(NULL, 1, 32, NULL);
	}

	~allPartition () {
		delete p;
		delete ft; 
		delete rd; 
		delete o; 
		CloseHandle(mutex);
	}
};


class KernelFS { 
public:
	~KernelFS ();

	static void createAllPartition(); 
	static HANDLE mutex;

	static char mount(Partition* partition);
	static char unmount(char part);
	static char format(char part);
	
	static char doesExist(char* fname);
	static File* open(char* fname, char mode);
	static char deleteFile(char* fname);

    static char createDir(char* dirname);
    static char deleteDir(char* dirname);
    static char readDir(char* dirname, EntryNum n, Entry &e);

protected:
	KernelFS ();
	friend FS;

private:
	static allPartition** allP;

};

#endif