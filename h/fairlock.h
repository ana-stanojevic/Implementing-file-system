#ifndef _rwlock_h_
#define _rwlock_h_

#define signal(x) ReleaseSemaphore(x,1,NULL)
#define wait(x) WaitForSingleObject(x,INFINITE)

#include "windows.h"

class FairLock{
private:
    CRITICAL_SECTION crit;
    HANDLE Turn;
    long ticket, next;
public:
    FairLock(){
        ticket=0; next=0;
        InitializeCriticalSection(&crit); 
        Turn = CreateEvent(NULL,TRUE,TRUE,NULL); 
    }

    ~FairLock(){
        CloseHandle(Turn);
        DeleteCriticalSection(&crit); 
    }
 
    void Enter(){
        EnterCriticalSection(&crit);
		long turn = ticket;
		ticket++;
		if (next != turn) {
			while (next != turn) { 
				LeaveCriticalSection(&crit); 
				wait(Turn); 
				EnterCriticalSection(&crit);
			}
		}                                        
 
		ResetEvent(Turn);
        LeaveCriticalSection(&crit);
    }

    void Leave(){
    	EnterCriticalSection(&crit);
    	SetEvent(Turn);
		next++;
    	LeaveCriticalSection(&crit);
    }
   
}; 

#endif