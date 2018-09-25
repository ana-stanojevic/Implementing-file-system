#ifndef _kfile_h_
#define _kfile_h_ 

#include "part.h"
#include "fs.h"
#include "of.h"
#include "FAT.h"
#include "rootd.h"
#include "stdio.h"
using namespace std;


class KernelFile { 
public: 
	char write (BytesCnt n, char* buffer); 
	BytesCnt read (BytesCnt n, char* buffer); 
	char seek (BytesCnt);
	BytesCnt filePos();
	char eof (); 
	BytesCnt getFileSize (); 
	char truncate (); 
	~KernelFile(); 
	KernelFile (char c, Partition* p, OpenFiles* o, FAT* ft, RootDir* rd, Entry e, char m, char* fname);

	friend class FS;
	friend class KernelFS;
	friend class RootDir;

private:
	char pChar;
	Partition* p;
	OpenFiles* o;
	FAT* ft;
	Entry e;
	char mode;
	char* fname;
	BytesCnt cursor;
	RootDir* rd;
	ClusterNo cursorCl;
	
};

#endif
