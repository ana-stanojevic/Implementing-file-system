#include "fat.h"

FAT::FAT(Partition* pp, ClusterNo num, ClusterNo number, ClusterNo free, ClusterNo* t){
   
       p = pp;
	   if (number == 0) table = 0;  else table = new ClusterNo[number+1];
       freeb = free;       
       for (ClusterNo i = num; i<number; i++) table[i] = t[i];
	   sFAT = CreateSemaphore(NULL, 1, 32, NULL);
	   cw = -1; cr = -1; ce = -1; cf = -1; d = 0; de = 0; df = 0;
	   bw = new char[2050]; br = new char[2050]; be = new char[2050]; bf = new char[2050];
	   sw = CreateSemaphore(NULL, 1, 32, NULL); sr = CreateSemaphore(NULL, 1, 32, NULL); se = CreateSemaphore(NULL, 1, 32, NULL);
	   sf = CreateSemaphore(NULL, 1, 32, NULL);
	}

FAT::~FAT (){
		delete table;
		delete[] bw; delete[] br; delete[] be;
		CloseHandle(sw); CloseHandle(sr); CloseHandle(se); CloseHandle(sFAT);
	}
    
ClusterNo FAT::getNextCluster(ClusterNo number){
	    wait(sFAT);
    	ClusterNo o = table[number];
		signal(sFAT);
		return o;
    }

void FAT::formatFAT(ClusterNo num, ClusterNo number, ClusterNo free, ClusterNo* t){
	if (table != 0) delete table;
	table = new ClusterNo[number];
	freeb = free;
	for (ClusterNo i = num; i<number; i++) table[i] = t[i];
}

char FAT::freeCluster(ClusterNo number, bool yes){
	    wait(sFAT);
        table[number]=freeb;
        freeb=number;
		wait(sf);
        ClusterNo t = (number + 4)/512;
		if (cf != t){
			if (df == 1){ p->writeCluster(cf, bf); df = 0; }
			cf = t;
			if (!yes) wait(se);
			if (cf == ce && de == 1) { p->writeCluster(ce, be); de = 0; }
			if (!yes) signal(se);
			p->readCluster(cf, bf);
		}
        ClusterNo* cn = ((ClusterNo*)bf) + (number + 4) % 512; 
		*cn = table[number]; df = 1;
		signal(sf);
		signal(sFAT);
		return 1;
    }  
    
ClusterNo FAT::getFreeCluster(ClusterNo number, bool yes){
	    wait(sFAT);
		if (freeb == 0) { signal(sFAT); return -1; }
    	ClusterNo t = freeb;
    	freeb = table[freeb];
    	table [number] = t;
    	ClusterNo tmp = (number + 4)/512;
		wait(sf);
		if (cf != tmp){
			if (df == 1){ p->writeCluster(cf, bf); df = 0; }
			cf = tmp;
			if (!yes) wait(se);
			if (cf == ce && de == 1) { p->writeCluster(ce, be); de = 0; }
			if (!yes) signal(se);
			p->readCluster(cf, bf);
		}
        ClusterNo* cn = (ClusterNo*)bf + (number + 4) % 512; 
		*cn = table[number]; df = 1;
		if (cf != 0){
			if (df == 1){ p->writeCluster(cf, bf); df = 0; }
			cf = 0;
			if (!yes) wait(se);
			if (cf == ce && de == 1) { p->writeCluster(ce, be); de = 0; }
			if (!yes) signal(se);
			p->readCluster(cf, bf);
		}
		cn = (ClusterNo*)bf; 
		*cn = freeb; df = 1;
		signal(sf);
		signal(sFAT);
		return t;
    }


