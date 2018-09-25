#ifndef _fat_h_
#define _fat_h_

#include "part.h"
#include "fs.h"
#include "fairlock.h"
#include <windows.h>

class FAT{
private:
	Partition* p;
	ClusterNo* table;
	ClusterNo num, freeb;
	HANDLE sFAT;
public:
	ClusterNo cw, cr, ce, cf; 
	char* bw, *br, *be, *bf; 
	HANDLE sw, sr, se, sf; 
	bool d, de, df;

	FAT(Partition* pp, ClusterNo num, ClusterNo number, ClusterNo free, ClusterNo* t);
	~FAT ();
    ClusterNo getNextCluster(ClusterNo number);
	void formatFAT(ClusterNo num, ClusterNo number, ClusterNo free, ClusterNo* t);
    char freeCluster(ClusterNo number, bool yes); 
	ClusterNo getFreeb(){ return freeb; }
    ClusterNo getFreeCluster(ClusterNo number, bool yes);
};

#endif
