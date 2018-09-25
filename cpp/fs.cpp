#include "fs.h"
#include "kfs.h"

KernelFS* FS::myImpl = 0;

FS::FS () {
	myImpl = new KernelFS();
}

FS::~FS () {
	delete myImpl;
}

char FS::mount(Partition* partition) {
	if (myImpl == 0) myImpl = new KernelFS();
	return myImpl->mount(partition);
}

char FS::unmount(char part) {
	return myImpl->unmount(part);
}

char FS::format(char part) {
	return myImpl->format(part);
}
	
char FS::doesExist(char* fname) {
	return myImpl->doesExist(fname);
}

File* FS::open(char* fname, char mode)  {
	return myImpl->open(fname, mode);
}
	
char FS::deleteFile(char* fname) {
	return myImpl->deleteFile(fname);
}
char FS::createDir(char* dirname){
	return myImpl->createDir(dirname);
}
char FS::deleteDir(char* dirname){
	return myImpl->deleteDir(dirname);
}
char FS::readDir(char* dirname, EntryNum n, Entry &e){
    return myImpl->readDir(dirname, n, e);
}

 

