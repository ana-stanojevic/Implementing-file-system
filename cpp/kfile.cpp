#include "kfile.h"
#include <stdio.h>
#include <iostream>
using namespace std;
KernelFile::KernelFile (char c, Partition * pp, OpenFiles *oo, FAT* ftt, RootDir* rrd, Entry ee, char m, char* ff) {
	pChar = c;
	p = pp;
	ft=ftt;
	o = oo;
	rd = rrd;
	e = ee;
	mode = m;
	cursorCl = e.firstCluster;
	if (mode == 'a') {
		cursor = ee.size;
		while (cursor >= 2048) {
			cursorCl = ft->getNextCluster(cursorCl);
			cursor -= 2048;
		}
		cursor = ee.size;
	}
	else cursor = 0;
	fname = ff;	
}


KernelFile::~KernelFile() {
	if (ft->d == 1) {p->writeCluster(ft->cw, ft->bw); ft->d = 0; }
	
	Entry* ee = new Entry(); int* nuum = new int(); ClusterNo* cll = new ClusterNo();
	if (rd->doesExist(fname, ee, nuum, cll)) rd->updateEntry(fname, e);
	
	o->closeOF(fname);

    delete nuum; delete cll;
	delete ee;
	
}


char KernelFile::write (BytesCnt n, char* buffer){
   if (mode == 'r') return 0;
   if (ft->getFreeb() == 0) return 0;
   Entry ee = e;
   long tmpc = cursor % 2048, tmps = ee.size;
   tmps -= (cursor / 2048) * 2048; 
   if (ee.firstCluster == 0) { 
	   e.firstCluster = ft->getFreeCluster(e.firstCluster, 0); 
	   if (e.firstCluster == -1) return 0;
	   e.size = 0; ee = e; cursorCl = e.firstCluster; }
   ee.firstCluster = cursorCl; 
    long j=0, i=2048;
	wait(ft->sw);
	if (ee.firstCluster != ft->cw) {
		if (ft->d == 1){ p->writeCluster(ft->cw, ft->bw); ft->d = 0; }
		ft->cw = ee.firstCluster;
		p->readCluster(ft->cw, ft->bw);
	}
	  for (i = tmpc; i < 2048 && n != j; i++)  { 
		  ft->bw[i] = buffer[j]; 
		  if (ft->d == 0) ft->d = 1;
		  j++;
		  cursor++; 
		  if (i >= tmps) ee.size++; }
    if (n == j) {
			if (i == 2048){
				tmps -= 2048;
				if (ft->d == 1){p->writeCluster(ft->cw, ft->bw); ft->d = 0; }
				if (tmps <= 0)
				{
					ft->cw = ft->getFreeCluster(ft->cw, 0);
					if (ft->cw == -1) {
						signal(ft->sw);  return 0;
					}
				}
				else ft->cw = ft->getNextCluster(ft->cw);
				p->readCluster(ft->cw, ft->bw);
				cursorCl = ft->cw;
			}
		e.size = ee.size;
		signal(ft->sw); 
		return 1;
	}
	while (n != j){
		for (; i != 2048 && n!=j; i++) {
			ft->bw[i] = buffer[j++];
			if (ft->d == 0) ft->d = 1;
	     	cursor++;
			if (i >= tmps) ee.size++;
		} 
		tmps -= 2048;
		if (n != j){
			if (ft->d == 1){ p->writeCluster(ft->cw, ft->bw); ft->d = 0; }
			if (tmps <= 0)
			{
				ft->cw = ft->getFreeCluster(ft->cw, 0); if (ft->cw == -1) {
					signal(ft->sw);  return 0;
				}
			}
			else ft->cw = ft->getNextCluster(ft->cw);
			p->readCluster(ft->cw, ft->bw);
			cursorCl = ft->cw;
			i = 0;
		}
	}
	
	e.size = ee.size;
	signal(ft->sw);
	return 1;
    
}


BytesCnt KernelFile::read(BytesCnt n, char* buffer){

	if (e.firstCluster == 0) return 0;
	if (cursor == e.size) return 0;
 
	Entry ee = e;
	long tmpc = cursor%2048, tmpcc = cursor, tmps = ee.size;
	tmps -= (cursor / 2048) * 2048;
	ee.firstCluster = cursorCl; 
	wait(ft->sr);
	if (ee.firstCluster != ft->cr) {
		ft->cr = ee.firstCluster;
		wait(ft->sw);
		if (ee.firstCluster == ft->cw && ft->d == 1) {	
			p->writeCluster(ft->cw, ft->bw); ft->d = 0;
		}
		p->readCluster(ft->cr, ft->br);
		signal(ft->sw);
	}
	
	long j = 0, i;
	for (i = tmpc; i<2048 && n != j && i<tmps; i++) { buffer[j] = ft->br[i]; cursor++; j++; }
	if (n == j || i==tmps) { 
		if (i == 2048){
			wait(ft->sw);
			ft->cr = ft->getNextCluster(ft->cr);
			if (ft->cr == ft->cw && ft->d == 1) {
				p->writeCluster(ft->cw, ft->bw); ft->d = 0;
			}
			p->readCluster(ft->cr, ft->br);
			cursorCl = ft->cr;
			signal(ft->sw);
		}
		signal(ft->sr); 
		if (n==j) return n; 	
	    return tmps - tmpc + 1; }
	tmps -= 2048;
	while (n != j && tmps>0){
		for (; i != 2048 && n != j && tmps>0; i++) {
			buffer[j++] = ft->br[i];
			tmps--;
			cursor++;
		}
		if (n != j && tmps > 0){
			wait(ft->sw);
			ft->cr = ft->getNextCluster(ft->cr);
			if (ft->cr == ft->cw && ft->d == 1) {
				p->writeCluster(ft->cw, ft->bw); ft->d = 0;
				
			}
			p->readCluster(ft->cr, ft->br);
			cursorCl = ft->cr;
			signal(ft->sw);
				i = 0;
			}
		}
		if (n == j) { signal(ft->sr);  return n; }
		signal(ft->sr);
		return ee.size - tmpcc;
	}


char KernelFile::seek(BytesCnt cnt) {
	if (cnt > e.size)  return 0;
	cursor = cnt;
	cursorCl = e.firstCluster;
	while (cnt >= 2048) { 
		cursorCl = ft->getNextCluster(cursorCl); 
		cnt -= 2048; }
	return 1;
}

BytesCnt KernelFile::filePos() {
	return cursor;
}

char KernelFile::eof() {
	if (cursor == e.size) return 2;
	else return 0;
}

BytesCnt KernelFile::getFileSize() {
	return e.size;
}

char KernelFile::truncate() {

	if (mode == 'r') return 0;
	if (e.firstCluster == 0) return 1;
	if (e.size == cursor) return 1;
	char* tmpbuf = new char[2048]; long tmpc = cursor, tmps=e.size; Entry ee = e;

    while (tmpc>= 2048){
    ee.firstCluster = ft->getNextCluster(ee.firstCluster);
    tmpc-=2048;
    tmps-=2048;
    }
	ClusterNo num; bool b=false;
	if (tmps > 2048){ ee.firstCluster = ft->getNextCluster(ee.firstCluster); tmps -= 2048; b = true; }
    while (tmps>0) {
    ClusterNo num = ft->getNextCluster(ee.firstCluster);
    ft->freeCluster(ee.firstCluster, 0);
    ee.firstCluster = num;
    tmps-=2048;
    }

    e.size = cursor;
    if (e.size == 0) {
		ft->freeCluster(e.firstCluster, 0);
		e.firstCluster = 0;
	}

	delete [] tmpbuf;
	return 1;
}

