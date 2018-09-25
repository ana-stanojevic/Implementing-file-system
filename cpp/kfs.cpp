#include "kfs.h"

allPartition ** KernelFS::allP = 0; 
HANDLE KernelFS::mutex = 0;

KernelFS::KernelFS () { 
	if (allP == 0){ 
		allP = new allPartition* [26];
	for (int i = 0; i < 26; i++) 
		allP[i] = new allPartition();
	 mutex = CreateSemaphore(NULL, 1, 32, NULL);
  }  
}
KernelFS::~KernelFS () {
	for (int i = 0; i < 26; i++) {
		delete allP[i];
	}
	delete allP;
}    

char KernelFS::mount(Partition* p) { 
	wait(mutex);
	int i = 0;
	while (i < 26) {
		if (allP[i]->p == 0) {
			allP[i]->p = p;
			char *buf = new char[ClusterSize];
			allP[i]->p->readCluster(0, buf);
			ClusterNo* bufF = (ClusterNo*)(buf); 
			ClusterNo free = bufF[0], number = bufF[1], rstart = bufF[2], rsize = bufF[3];
			ClusterNo r=0,g;
			if (number >= 508) { g = number - 508; r = g / 512; }
			if (r>0) r+=((g % 512) != 0) ? 1 : 0;
			ClusterNo* bufferFAT; ClusterNo nn = number;
			bufferFAT = new ClusterNo[512*(r + 1)]; 
			for (int i = 0; i<508 && nn>0; i++) {bufferFAT[i] = bufF[i + 4]; nn--;}
			delete [] buf; 
			ClusterNo x=0, pom=r+1; 
			while (r>0) {
				allP[i]->p->readCluster(x + 1, (char*)(bufferFAT + 512 * x + 508));
			x++; r--;
			}
            
			allP[i]->ft = new FAT (p, pom, number, free, bufferFAT);

			allP[i]->rd = new RootDir (i + 'A', allP[i]->p, allP[i]->ft, rstart, rsize);

			allP[i]->o = new OpenFiles(); 

			char c = i + 'A';
			signal(mutex);
			return c;
		}
		i++;
	}
	signal(mutex);
	return 0;
}


char KernelFS::unmount(char part) {
	wait(mutex);
	int i = part - 'A';
	if (i < 0 || i >= 26 || allP[i]->p == 0) {
		signal(mutex);  return 0;
	}
	wait(allP[i]->mutex);
	allP[i]->unmountMode = true;
	while (allP[i]->o->allClosedOF() == 0) { 
		WaitForSingleObject(allP[i]->o->sem, INFINITE); 
	}

	allP[i]->p = 0;
	delete allP[i]->rd;
	delete allP[i]->ft;
	delete allP[i]->o;

	allP[i]->unmountMode = false;
	signal(allP[i]->mutex);
	signal(mutex);
	return 1;
}

char KernelFS::format(char part) {
	int i = part - 'A';
	if (i < 0 || i >= 26 || allP[i]->p == 0) return 0;
	wait(allP[i]->mutex);
    allP[i]->formatMode = true; 
	while (allP[i]->o->allClosedOF() == 0) {
		WaitForSingleObject(allP[i]->o->sem, INFINITE); 
	} 
	ClusterNo r = 0;
	ClusterNo g = allP[i]->p->getNumOfClusters();
	if (g > 508){
		ClusterNo g = allP[i]->p->getNumOfClusters() - 508;
		r = g / 512;
		r += ((g % 512) != 0) ? 1 : 0;
	}
	ClusterNo *bufF = new ClusterNo[(r + 1) * 512];
	bufF[1] = allP[i]->p->getNumOfClusters();
	bufF[0] = r + 2;
	bufF[2] = r + 1;
	bufF[3] = 0;
	for (ClusterNo i = 4 + r + 2; i<bufF[1] - 1; i++) bufF[i] = (i + 1)-4; 
	bufF[bufF[1] - 1] = 0;
	allP[i]->ft->formatFAT(r + 1, bufF[1], bufF[0], (bufF+4));
	allP[i]->rd->formatRootDir(allP[i]->ft, bufF[2], bufF[3]);
	char* buf = (char*)(bufF);
	allP[i]->p->writeCluster(0, buf);
	ClusterNo k = 1; 
	while (r>0) {
		allP[i]->p->writeCluster(k, buf + 2048 * k);
			k++; r--;
			}
	allP[i]->formatMode = false;
	delete [] bufF;
	signal(allP[i]->mutex);
	return 1;
}


char KernelFS::doesExist(char* name) {
	int i = name[0] - 'A';
	if (i < 0 || i >= 26 || allP[i]->p == 0) return 0;
	wait(allP[i]->mutex);
	Entry* ee = new Entry();
    
	int* nuum = new int(); ClusterNo* cll = new ClusterNo();
	int yes = allP[i]->rd->doesExist(name, ee, nuum, cll); 
	
	delete nuum; delete cll;
	delete ee;
	signal (allP[i]->mutex);
	return yes;
}

File* KernelFS::open(char* fname, char mode)  {
	int i = fname[0] - 'A'; 
	if (i < 0 || i >= 26 || allP[i]->p == 0) return 0;
    if (allP[i]->formatMode == true) return 0;
	if (allP[i]->unmountMode == true) return 0;
	wait(allP[i]->mutex);
	Entry* e = new Entry(); 
	int* nuum = new int(); ClusterNo* cll = new ClusterNo();
	int more, yes = allP[i]->rd->doesExist(fname, e, nuum, cll);
	Entry e2;
	if (!yes)
	{
		if (mode == 'r' || mode == 'a') {
			delete e; delete nuum; delete cll; signal(allP[i]->mutex);  return 0;
		}
		allP[i]->rd->addEntry(fname);
	}
	else {
			if (mode == 'w') {
				signal(allP[i]->mutex);
				allP[i]->o->updateOF(fname);
				wait(allP[i]->mutex);
				allP[i]->rd->doesExist(fname, e,nuum, cll);
				e2 = *e;
				KernelFile *kF = new KernelFile(fname[0], allP[i]->p, allP[i]->o, allP[i]->ft, allP[i]->rd, e2, 'w', fname);
				kF->truncate(); 
				e2=kF->e; 
				delete kF;
				
			}
		}
	signal(allP[i]->mutex);
	allP[i]->o->updateOF(fname);
	wait(allP[i]->mutex);
	more, yes = allP[i]->rd->doesExist(fname, e, nuum, cll);
	delete nuum; delete cll;
	e2 = *e;
	KernelFile *kernelFile = new KernelFile(fname[0], allP[i]->p, allP[i]->o, allP[i]->ft, allP[i]->rd, e2, mode, fname);
	File *file = new File();
	file->myImpl = kernelFile;
	delete e; 
	signal(allP[i]->mutex);
	return file;
}


char KernelFS::deleteFile(char* fname) {
	int i = fname[0] - 'A';
	if (i < 0 || i >= 26 || allP[i]->p == 0) return 0;
	wait(allP[i]->mutex);
	Entry* e = new Entry(); 
	int* nuum = new int(); ClusterNo* cll = new ClusterNo();
	int more, yes = allP[i]->rd->doesExist(fname,e, nuum, cll);
	Entry e2 = *e;
	if (!yes) { delete e; delete nuum; delete cll; signal(allP[i]->mutex);  return 0; }
	allP[i]->rd->removeEntry(fname);
	signal(allP[i]->mutex);
    allP[i]->o->updateOF(fname); 
	wait(allP[i]->mutex);
	KernelFile *kF = new KernelFile(fname[0], allP[i]->p, allP[i]->o, allP[i]->ft, allP[i]->rd, e2, 'w', fname);
	kF->truncate();
	delete kF;
	delete e;  delete nuum; delete cll;
	signal(allP[i]->mutex);
	return 1;
}


char KernelFS::createDir(char* dirname){
    int i = dirname[0] - 'A';
	if (i < 0 || i >= 26 || allP[i]->p == 0) return 0;
	if (doesExist(dirname)) {  return 1;}
	wait(allP[i]->mutex);
	char c = allP[i]->rd->createDir(dirname);
	signal(allP[i]->mutex);
	return c;
    } 

char KernelFS::deleteDir(char* dirname){
    	 int i = dirname[0] - 'A';
	if (i < 0 || i >= 26 || allP[i]->p == 0) return 0;
	wait(allP[i]->mutex);
	if (allP[i]->rd->dirEmpty(dirname))  {
		char c = allP[i]->rd->deleteDir(dirname);
		signal(allP[i]->mutex);
		return c;
	}
	signal(allP[i]->mutex);
	return 0;
    } 

char KernelFS::readDir(char* dirname, EntryNum n, Entry &e){
	int i = dirname[0] - 'A';
	if (i < 0 || i >= 26 || allP[i]->p == 0) return 0;
	if (!doesExist(dirname)) { return 0; }
	    wait(allP[i]->mutex);
		char c= allP[i]->rd->readDir(dirname, n, e);
		signal(allP[i]->mutex);
		return c;
    }
	
