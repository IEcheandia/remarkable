
#ifndef _CLSER
#define _CLSER

#ifdef __cplusplus
extern "C" {
#endif

int  clSerialInit	(unsigned long serialIndex, void** serialRefPtr);
int  clSerialRead	(void* serialRef, char * buffer,unsigned long * bufferSize,unsigned long serialTimeOut);
int  clSerialWrite	(void* serialRef, char * buffer,unsigned long * bufferSize,unsigned long serialTimeOut);
void clSerialClose	(void* serialRef);

int  clSetParity	(void* serialRef,unsigned int parityOn);

#ifdef __cplusplus
}
#endif


#endif
