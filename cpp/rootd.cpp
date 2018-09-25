#include "rootd.h"

using namespace std;

RootDir::RootDir(char pc, Partition *pp, FAT* ftt, ClusterNo s, ClusterNo ss)  {
	pChar = pc;
	p = pp;
	ft = ftt;
	start = s;
	size = ss;
}

RootDir::~RootDir() {
	if (ft->de == 1) { p->writeCluster(ft->ce, ft->be); ft->de = 0;}
	}

int RootDir::changeToEntry(char* name, Entry* e, long* pos) { 
	  int j=0;
	  while (name[*pos]!='\0') {
	  if (name[*pos]=='.') break;
      if (name[*pos]== '\\') {
      	(*pos)++; e->attributes=2; 
        while (j<8) e->name[j++]=' ';
        e->ext[0]='\0';
      	return 1;}
      else {e->name[j++] = name[*pos]; (*pos)++;}
	  } 
	if (name[*pos]=='.') {
		while (j<8) e->name[j++] = ' ';
		e->attributes = 1;
		j=0; (*pos)++;
		while (name[*pos]!='\0')
		{
			e->ext[j++] = name[*pos]; (*pos)++;
		}
		while (j<3) e->ext[j++]=' ';
    }
	else {
		while (j<8) e->name[j++] = ' ';
		j = 0; while (j<3) e->ext[j++] = ' ';
		e->attributes=2;} 
    	
    return 0;
    }

bool charCmp (char*c1, char* c2, int d) {
	for (int i = 0; i < d; i++) {
		if (c1[i] != c2[i]) return false;
	}
	return true;
}

void RootDir::formatRootDir (FAT* fft, ClusterNo s, ClusterNo ss){
	ft = fft;
	start = s;
	size = ss;

}
int RootDir::doesExist(char* name, Entry* e, int* num, ClusterNo* starttmp) {
	wait(mutex);
	if (strlen(name) == 3) { e->size = size; e->firstCluster = start; signal(mutex);  return 1; }
	bool found = true, first; long* pos =new long(); *pos = 3;
	char* tmpbuff = new char[2050];
	ClusterNo sizetmp = size; (*starttmp) = start;
    while (found){
    found = false;
    first = true;
	ClusterNo i = sizetmp;
    int more = changeToEntry(name,e, pos);
    while (i>0 && !found){
    	if (first) first = false;
		else *starttmp = ft->getNextCluster(*starttmp);
		wait(ft->se);
		if (ft->ce == *starttmp && ft->de == 1) { p->writeCluster(ft->ce, ft->be); ft->de = 0; }
		signal(ft->se);
		p->readCluster(*starttmp, tmpbuff);	
	*num=0;
	while (i>0 && *num < 102 && !found) { 
    Entry* etmp = (Entry*)(tmpbuff) + (*num);
    if (charCmp (etmp->name, e->name,8) && e->attributes == etmp->attributes)
    	if (!more)
		{if ( e->attributes == 2 || (etmp->attributes == 1 && charCmp(etmp->ext, e->ext,3))) 
    		{
    			*e = *etmp; 
				delete pos;
				delete[] tmpbuff;
				signal(mutex);
    			return 1;
    		}
		else { delete pos;  signal(mutex);  return 0; }
		}
    	else { 
    		found = true;
    		*starttmp = etmp->firstCluster; 
    		sizetmp = etmp->size;}   
    i--; (*num) ++;
	}
} }
	delete pos;
	delete[] tmpbuff;
	signal(mutex);
return 0;
} 

char RootDir::createDir(char* dirname){
	if (strlen(dirname) == 3) return 1;
return addEntry(dirname);
}


char RootDir::deleteDir(char* dirname){
if (strlen(dirname) == 3) return 0;
return removeEntry(dirname);
}

char RootDir::readDir(char* dirname, EntryNum n, Entry &e){
Entry* etmp = new Entry (); 
int* nuum = new int(); ClusterNo* cll = new ClusterNo(); 
if (!(doesExist(dirname, etmp, nuum, cll))) return 0;
delete nuum; delete cll;
if (etmp->size <= n) { delete etmp; return 2; }
while (n>=102){
	etmp->firstCluster = ft->getNextCluster(etmp->firstCluster);
	n-=102;}
wait(ft->se);
if (ft->ce != etmp->firstCluster){
	if (ft->de == 1) { p->writeCluster(ft->ce, ft->be); ft->de = 0; }
	ft->ce = etmp->firstCluster;
	p->readCluster(ft->ce, ft->be);
}
			Entry * ex = (Entry*)(ft->be + n*20);
			e=*ex; 
delete etmp;
signal(ft->se);
return 1;
}

bool RootDir::dirEmpty (char* dirname){
	Entry* e = new Entry();
    int* nuum = new int(); ClusterNo* cll = new ClusterNo();
	if (!doesExist(dirname,e, nuum, cll)) return 0;
	delete nuum; delete cll;
	if (e->size == 0) { delete e;  return 1; }
	{delete e;  return 0; }
}


char RootDir::addEntry(char* name) {
	wait(mutex);
	if (strlen(name) == 3) {
		signal(mutex);  return 1;
	}
	long j=0, k = 0, i = 0, numm=0, n = strlen(name);
	while (name[i] != '\0') { if (name[i] == '\\') numm++; i++; }
char* tmpbuf = new char [n+1]; Entry* e = new Entry();
while (k != numm) {
	while (name[j] != '\\'){ tmpbuf[j] = name[j]; j++; }
	k++;
	if (k == numm){
		if (j == 2)  { tmpbuf[j] = name[j]; j++; }
	tmpbuf[j] = '\0';
 }
	else { tmpbuf[j] = name[j]; j++; }
   }
 int* nuum = new int(); ClusterNo* cll = new ClusterNo();
if (doesExist(name, e, nuum, cll)) {  delete nuum; delete cll;  delete[] tmpbuf; delete e; signal(mutex); return 1; }
if (!doesExist(tmpbuf, e, nuum, cll)) {  delete nuum; delete cll;  delete[] tmpbuf; delete e; signal(mutex);  return 0; }

Entry * mainE = new Entry (); Entry tmpE = *e;
 long* pos = new long(); *pos = 3;
 int more = changeToEntry(name, mainE, pos);
 while (more) more = changeToEntry(name, mainE, pos);
mainE -> size = 0; mainE -> firstCluster = 0;

while (tmpE.size>102){
tmpE.firstCluster = ft->getNextCluster(tmpE.firstCluster);
tmpE.size = tmpE.size - 102;
}
wait(ft->se);
if (tmpE.size % 102 != 0 || ((tmpE.size == 0 || tmpE.size%102!=0) && strlen(tmpbuf) == 3)) {	
	if (ft->ce != tmpE.firstCluster){
		if (ft->de == 1) { p->writeCluster(ft->ce, ft->be); ft->de = 0; }
		ft->ce = tmpE.firstCluster;
		p->readCluster(ft->ce, ft->be);
	}
Entry* ee = (Entry*)(ft->be);
ee += (tmpE.size % 102);
*ee = *mainE; ft->de = 1;
}
else {
	tmpE.firstCluster = ft->getFreeCluster(tmpE.firstCluster, 1);
	if (tmpE.firstCluster == -1) {
		signal(mutex);  signal(ft->se);  return 0;
	}
	if (ft->ce != tmpE.firstCluster){
		if (ft->de == 1) { p->writeCluster(ft->ce, ft->be); ft->de = 0; }
		ft->ce = tmpE.firstCluster;
		p->readCluster(ft->ce, ft->be);
	}
	Entry* ee = (Entry*)(ft->be);
	*ee = *mainE; ft->de = 1;
}
if (strlen(tmpbuf) == 3) {
	size++;
	if (ft->ce !=0){
		if (ft->de == 1) { p->writeCluster(ft->ce, ft->be); ft->de = 0; }
		ft->ce = 0;
		wait(ft->sf);
		if (ft->ce == ft->cf && ft->df == 1) { p->writeCluster(ft->cf, ft->bf); ft->df = 0; }
		signal(ft->sf);
		p->readCluster(ft->ce, ft->be);
	}
	*((ClusterNo*)(ft->be) + 3) = size; ft->de = 1;
}
else {
	if (ft->ce != *cll){
		if (ft->de == 1) { p->writeCluster(ft->ce, ft->be); ft->de = 0; }
		ft->ce = *cll;
		p->readCluster(ft->ce, ft->be);
	}
		Entry* y = (Entry*)(ft->be + (*nuum) * 20);
		y->size++;
		if (y->firstCluster == 0) y->firstCluster = tmpE.firstCluster;
		ft->de = 1;
	}
delete nuum; delete cll;
delete[] tmpbuf;
delete e; 
delete mainE; delete pos;
signal(ft->se);
signal(mutex);
return 1;
}


char RootDir::removeEntry(char* name) {
	wait(mutex);
	long j = 0, k = 0, i = 0, numm = 0, n = strlen(name);
	while (name[i] != '\0') { if (name[i] == '\\') numm++; i++;}
char* tmpbuf = new char [n+1]; Entry* e = new Entry();
while (k!=numm) {
	while (name[j] != '\\'){ tmpbuf[j] = name[j]; j++;  }
	k++;
	if (k == numm){
		if (j == 2)  { tmpbuf[j] = name[j]; j++; }
		tmpbuf[j] = '\0';
	}
	else { tmpbuf[j] = name[j]; j++; }
}
int* nuum = new int(); ClusterNo* cll = new ClusterNo();
if (!doesExist(name, e, nuum, cll)) { delete cll; delete nuum;  delete[] tmpbuf; delete e; signal(mutex);  return 0; }
if (!doesExist(tmpbuf, e, nuum, cll)) { delete cll; delete nuum;  delete[]tmpbuf; delete e; signal(mutex);  return -1; }
wait(ft->se);
if (strlen(tmpbuf) == 3) {
	size--;
	if (ft->ce != 0){
		if (ft->de == 1) { p->writeCluster(ft->ce, ft->be); ft->de = 0; }
		ft->ce = 0;
		wait(ft->sf);
		if (ft->ce == ft->cf && ft->df == 1) { p->writeCluster(ft->cf, ft->bf); ft->df = 0; }
		signal(ft->sf);
		p->readCluster(ft->ce, ft->be);
	}
	*((ClusterNo*)(ft->be) + 3) = size; ft->de = 1;
}
else
{
		if (ft->ce != *cll){
			if (ft->de == 1) { p->writeCluster(ft->ce, ft->be); ft->de = 0; }
			ft->ce = *cll;
			p->readCluster(ft->ce, ft->be);
		}
		Entry* y = (Entry*)(ft->be + (*nuum) * 20);
		y->size--;
		ft->de = 1;	
}
Entry* mainE = new Entry(); Entry tmpE = *e; Entry * etmp; int num;
long* pos = new long(); *pos = 3; ClusterNo bb;
int more = changeToEntry(name, mainE, pos);
while (more) { more = changeToEntry(name, mainE, pos); }

bool found = false, first = true;
i = tmpE.size; 
while (i>0 && !found){
    	if (first) first = false;
		else tmpE.firstCluster = ft->getNextCluster(tmpE.firstCluster);
		if (ft->ce != tmpE.firstCluster){
			if (ft->de == 1) { p->writeCluster(ft->ce, ft->be); ft->de = 0; }
			ft->ce = tmpE.firstCluster;
			p->readCluster(ft->ce, ft->be);
		}
	num=0;
	while (i>0 && num < 102 && !found) { 
    etmp = (Entry*)(ft->be + num*20);
    if (charCmp (etmp->name, mainE->name,8) && etmp->attributes == mainE->attributes)
	if (etmp->attributes == 2 || (etmp->attributes == 1 && charCmp(etmp->ext, mainE->ext,3)))
		found = true;   
	if (!found) { i--; num++; }
	}
	if (!found) tmpE.size-=102;
}  
if (etmp->size != 0) { signal(ft->se);  delete nuum; delete cll; delete[] tmpbuf; delete pos; delete mainE; delete e;  signal(mutex);  return -1; }

if (tmpE.size > num + 1)
		if (tmpE.size <= 102) { 
			Entry* e1 = (Entry*)(ft->be + (tmpE.size-1)*20); 
			Entry* e2 = (Entry*) (ft->be + (num)*20);
			*e2 = *e1; ft->de = 1;
		}
		else {
		    while (tmpE.size>102){
			   bb=tmpE.firstCluster;
			   bb = ft->getNextCluster(bb);
			   tmpE.size-=102;
			}
			char* tmpbuf1 = new char [2048];
			if (bb == ft->ce) { p->writeCluster(ft->ce, ft->be); ft->de = 0;}
			p->readCluster(bb, tmpbuf1);
			Entry* e1 = (Entry*)(tmpbuf1 + (tmpE.size-1)*20); 
			Entry* e2 = (Entry*) (ft->be + (num)*20);
			*e2 = *e1; ft->de = 1;
			if (tmpE.size==1) {ft->freeCluster(bb, 1);}
			delete [] tmpbuf1; 
	       }
else if (num == 0) {
	if (strlen(tmpbuf) != 3 || (strlen(tmpbuf) == 3 && tmpE.firstCluster!=start)) ft->freeCluster(tmpE.firstCluster, 1);
	if (strlen(tmpbuf) != 3){
		if (ft->ce != *cll){
			if (ft->de == 1) { p->writeCluster(ft->ce, ft->be); ft->de = 0; }
			ft->ce = *cll;
			p->readCluster(ft->ce, ft->be);
		}
		Entry* y = (Entry*)(ft->be + (*nuum) * 20);
		if (y->size == 0) {y->firstCluster = 0; ft->de = 1;}
	}
}
delete nuum; delete cll;
signal(mutex);
signal(ft->se);
delete e; delete mainE;
delete[] tmpbuf;
delete pos; 
return 1;
}

char RootDir::updateEntry(char* name, Entry &ep) {
	wait(mutex);
	long j=0, k = 0, i = 0, numm = 0, n = strlen(name);
	while (name[i] != '\0') { if (name[i] == '\\') numm++; i++; }
char* tmpbuf = new char [n+1]; 
while (k != numm) {
	while (name[j] != '\\'){ tmpbuf[j] = name[j]; j++; }
	k++;
	if (k == numm){
		if (j == 2)  { tmpbuf[j] = name[j]; j++; }
		tmpbuf[j] = '\0';
	}
	else  { tmpbuf[j] = name[j]; j++; }
}
Entry* entry = new Entry();  int num;
int* nuum = new int(); ClusterNo* cll = new ClusterNo();
if (!doesExist(tmpbuf, entry, nuum, cll)) { delete nuum; delete cll;  delete[] tmpbuf; delete entry; signal(mutex);  return 0; }

delete nuum; delete cll;
Entry tmpE = *entry;
bool found = false, first = true;
i = tmpE.size;
wait(ft->se);
while (i>0 && !found){
    	if (first) first = false;
		else tmpE.firstCluster = ft->getNextCluster(tmpE.firstCluster);
		if (ft->ce != tmpE.firstCluster){
			if (ft->de == 1) { p->writeCluster(ft->ce, ft->be); ft->de = 0; }
			ft->ce = tmpE.firstCluster;
			p->readCluster(ft->ce, ft->be);
		}
	num=0;
	while (i > 0 && num < 102 && !found) { 
		Entry* etmp = (Entry*)(ft->be + num * 20);
		if (charCmp(etmp->name, ep.name,8) && etmp->attributes == ep.attributes)
		if (etmp->attributes == 2 || (etmp->attributes == 1 && charCmp(etmp->ext, ep.ext,3)))
			found = true;
		if (!found) {
			i--; 
			num++; }
	}
	if (!found) tmpE.size-=102;
}  
Entry* e1 = (Entry*) (ft->be + num*20);
*e1 = ep; ft->de = 1;

delete[] tmpbuf;
delete entry;
signal(mutex);
signal(ft->se);
return 1;
}



