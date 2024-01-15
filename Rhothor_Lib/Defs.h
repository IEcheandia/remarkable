/**************************************************************************************
 This software is developed by NEWSON NV
 Redistribution and use, with or without modification, are permitted
 
 THIS SOFTWARE IS PROVIDED BY NEWSON AS IS.
 ANY EXPRESS OR IMPLIED WARRANTIES ARE DISCLAIMED.
 IN NO EVENT SHALL NEWSON BE LIABLE FOR ANY DAMAGES ARISING
 OUT OF THE USE OF THIS SOFTWARE.
 **************************************************************************************/


#ifdef _WIN32
  #ifdef _MSC_VER
    #if _MSC_VER >= 1600
        #include <cstdint>
    #else
        typedef __int8           int8_t;
        typedef __int16          int16_t;
        typedef __int32          int32_t;
        typedef __int64          int64_t;
        typedef unsigned __int8  uint8_t;
        typedef unsigned __int16 uint16_t;
        typedef unsigned __int32 uint32_t;
        typedef unsigned __int64 uint64_t;
    #endif
  #endif

  #include "winsock2.h"
#else
  #include <cstdint>
  #include <stdbool.h>

  #include <sys/types.h>
  #include <sys/socket.h>
  #include <arpa/inet.h>
  #include <unistd.h>
  #include <netinet/in.h>
  #include <iostream>

  typedef uint32_t SOCKET;
  #define INVALID_SOCKET (SOCKET)(~0)
  #define ZeroMemory(destination, length) memset((destination), 0, (length))
//   using namespace std;
#endif






/* THREADING */

#ifdef __WIN32__
   
    typedef HANDLE mutex_t;
    typedef HANDLE thread_t;
   
    #define mutex_init(m) m = 0
    #define mutex_create(m) m = CreateMutex(NULL, FALSE, NULL)
    #define mutex_lock(m) WaitForSingleObject(m, INFINITE)
    #define mutex_unlock(m) ReleaseMutex(m)
    #define mutex_destroy(m) CloseHandle(m)
    
    #define thread_init(t, func, arg) t = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)func, arg, CREATE_SUSPENDED, NULL)
    #define thread_join(t) WaitForSingleObject(t, INFINITE)
    #define thread_sleep(time) ::Sleep(time);
    
#else

    #include <pthread.h>
    #include <assert.h>

    typedef pthread_mutex_t mutex_t;
    typedef pthread_t thread_t;

    #define mutex_init(m) m = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP
    #define mutex_create(m) 
    #define mutex_lock(m) assert(pthread_mutex_lock(&(m)) == 0)
    #define mutex_unlock(m) pthread_mutex_unlock(&(m))
    #define mutex_destroy(m) pthread_mutex_destroy(&(m))

    #define thread_init(t, func, arg) pthread_create(&t, NULL, func, arg)
    #define thread_join(t) pthread_join(t, NULL) 
    #define thread_sleep(time) usleep(time*1000);

#endif





#define PI 3.1415926535897932
#define EPSILON 1.0e-30

#define Sgn(val) ((val>=0)?1:-1)
#define CHARSWAP(ptr) { char c; c=*ptr; *ptr=*(ptr+1); *(ptr+1)=c; }

#define DELETE_(ptr)  { if (ptr!=NULL) delete ptr; ptr=NULL; }

#define RT_READFLASH             0x3a67bc01
#define RT_STOREFLASH            0x3a67bc02
#define RT_STOP                  0x3a67bc04
#define RT_GETPARAMSET           0x3a67bc05
#define RT_SELECT_CH             0x3a67bc08
#define RT_WRITE_CH              0x3a67bc09
#define RT_READ_CH               0x3a67bc0a
#define RT_TEST_CH               0x3a67bc0e
#define RT_CONNECT               0x3a67bc1a
#define RT_GETRETURNVALUE        0x3a67bc21
#define RT_LOAD_DIRECTORY        0x3a67bc23
#define RT_QUERY_VERSION         0x3a67bc24
#define RT_READFLASH_BOOTSCRIPT  0x3a67bc27
#define RT_WRITEFLASH_BOOTSCRIPT 0x3a67bc28
#define RT_GET_FLASH_FAT         0x3a67bc29
#define RT_GETCALIBRATION        0x3a67bc2b
#define RT_SCANTARGETS           0x3a67bc30
#define RT_MAPTARGET             0x3a67bc31
#define RT_CHREADFLASH           0x3a67bc32
#define RT_CHWRITEFLASH          0x3a67bc33
#define RT_CHSETPOINTS           0x3a67bc34
#define RT_CHACTUALS             0x3a67bc35
#define RT_CHPARAMS              0x3a67bc36
#define RT_FEADDRFLASH           0x3a67bc37
#define RT_FEWRITEFLASH          0x3a67bc38
#define RT_FEREADFLASH           0x3a67bc39

struct ParamSet32
{
// board profile
  int32_t d_VersionID;             // software version
  int32_t d_HwID;
  int32_t d_SerialNr;              // serial number
  int32_t d_Reserved;

// ID string
  char s_ID[64];

  float lf_FieldSize[8];       // (d_Unit)
  float lf_CalSize[8];
  float lf_FieldSizeZ[8];      // (d_Unit)
  int32_t d_SetpointFIR[8];
  int32_t hd_DragError[8];         // µsec

  int32_t d_FctIO1[8];             // Function I/O
  int32_t d_FctIO2[8];             // Function I/O
  int32_t d_FctIO3[8];             // Function I/O
  int32_t d_FctIO4[8];             // Function I/O
  int32_t d_FctIO5[8];             // Function I/O
  int32_t d_FctIO6[8];             // Function I/O
  int32_t d_FctIO7[8];             // Function I/O
  int32_t d_FctIO8[8];             // Function I/O
  int32_t d_FctIO9[8];             // Function I/O
  int32_t d_FctIO10[8];            // Function I/O
  int32_t d_FctIO11[8];            // Function I/O
  int32_t d_FctIO12[8];            // Function I/O
  int32_t d_FctIO13[8];            // Function I/O
  int32_t d_FctIO14[8];            // Function I/O
  int32_t d_FctIO15[8];            // Function I/O
  int32_t d_FctIO16[8];            // Function I/O
  int32_t d_FctIO17[8];            // Function I/O (laser channel)
  int32_t d_FctCH1[8];             // Function channel 1
  int32_t d_FctCH2[8];             // Function channel 2
  int32_t d_FctCH3[8];             // Function channel 3

  int32_t d_Mac[8];                // Mac address slave board

  char s_IP[4];                // IP configuration
  char s_Mask[4];

  int32_t d_CRC;
};

// memory map flash
#define SECTOR_SIZE32 0x40000
#define SECTOR_SETTINGS 1
#define SECTOR_STDIO 3
#define SECTOR_TEMP 4
#define SECTOR_CALIBRATION 5
#define SECTOR_FILES 6
#define SECTOR_BOOT 6
#define FILES 250
#define SECTOR_INFO 8191

struct FileEntry
{
  bool b_Valid;
  long ld_Size;
  char Name[256];
};

// list mode used when loading file
#define FILE_LOAD 99

//software versions
#define CU_SWID 108
