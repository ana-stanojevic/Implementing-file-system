#ifndef _rootd_h_
#define _rootd_h_

#include "part.h"
#include "fs.h"
#include "fat.h"
#include "file.h"
#include <string.h>
#include <stdio.h>


class RootDir {
public:
	RootDir(char pc, Partition *pp, FAT* ft, ClusterNo s, ClusterNo ss);
	~RootDir();

	static int changeToEntry(char* name, Entry* e, long* pos);
	int doesExist(char* name, Entry* e, int* num, ClusterNo* cl);
	void formatRootDir(FAT* ft, ClusterNo s, ClusterNo ss);
	
	char addEntry(char* fname);
	char removeEntry(char *fname);
	char updateEntry(char* name, Entry &e);
	
	char createDir(char *dirname);
	char deleteDir(char* dirname);
	char readDir(char* dirname, EntryNum n, Entry& e);
	bool dirEmpty (char* dirname);
	
private:
	char pChar;
	Partition *p;
	FAT* ft;
	ClusterNo start;
	ClusterNo size;
	HANDLE mutex;
	friend class KernelFS;
};

#endif
