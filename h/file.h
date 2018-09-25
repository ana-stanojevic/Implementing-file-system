#ifndef _file_h_
#define _file_h_

#include "fs.h"


class KernelFile; 

class File { 

	public: 
		char write (BytesCnt, char* buffer); 
	    BytesCnt read (BytesCnt, char* buffer); 
        BytesCnt filePos();
		char seek(BytesCnt cnt);
        char eof (); 
        BytesCnt getFileSize (); 
        char truncate (); //** opciono
        ~File(); //zatvaranje fajla 
    private: 
        friend class FS;
        friend class KernelFS; 
        File (); //objekat fajla se moze kreirati samo otvaranjem 
        KernelFile *myImpl; 

};

#endif