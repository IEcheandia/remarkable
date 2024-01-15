/*-----------------------------------------------------------------------------
 * EcType.h
 * Copyright                acontis technologies GmbH, Weingarten, Germany
 * Response                 Stefan Zintgraf
 * Description              EtherCAT Master type definitions
 *---------------------------------------------------------------------------*/

#if defined __linux__

#ifndef INC_ECTYPE
#define INC_ECTYPE
#include <stdint.h>

/*-TYPEDEFS------------------------------------------------------------------*/
#define EC_T_VOID void
typedef void            *EC_T_PVOID;

/*
typedef long            EC_T_BOOL;
*/
typedef int32_t         EC_T_BOOL;

typedef char            EC_T_CHAR;
typedef unsigned short  EC_T_WCHAR;

typedef unsigned char   EC_T_BYTE, *EC_T_PBYTE;
typedef unsigned short  EC_T_WORD;
/*
typedef unsigned long   EC_T_DWORD;
*/
typedef uint32_t        EC_T_DWORD;

typedef signed char	    EC_T_SBYTE;
typedef signed short    EC_T_SWORD;
/*
typedef signed long	    EC_T_SDWORD;
*/
typedef int32_t	        EC_T_SDWORD;

typedef int             EC_T_INT;
typedef unsigned int    EC_T_UINT;

typedef short           EC_T_SHORT;
typedef unsigned short  EC_T_USHORT;

/*typedef char*           EC_T_VALIST;*/

/*typedef __int64         EC_T_UINT64;*/    /* see EcOsPlatform.h */

/* type of lock */
typedef enum
{
    eLockType_DEFAULT= 1,                           /*< Default mutex           */
    eLockType_SPIN,                                 /*< only jobs --> spin lock */
    eLockType_INTERFACE,                            /*< interface and jobs      */

    /* Borland C++ datatype alignment correction */
    eLockType_BCppDummy   = 0xFFFFFFFF
} EC_T_OS_LOCK_TYPE;


/*-MACROS--------------------------------------------------------------------*/
#define EC_FALSE            0
#define EC_TRUE             1
#define EC_NULL             0

#define EC_NOWAIT           ((EC_T_DWORD)0x00000000)
#define EC_WAITINFINITE     ((EC_T_DWORD)0xFFFFFFFF)


/*-MASTER FEATURES-----------------------------------------------------------*/
#define INCLUDE_MASTER_SYNC         /* Distributed Clocks: Sync between DC clock master and EtherCAT master */

#define INCLUDE_MASTER_OBD          /* EtherCAT master object dictionary */
#define SLAVE_OBJECT_AMOUNT         ((EC_T_WORD)0x200)     /* Object elements (Max. 4096)*/

#define INCLUDE_SLAVE_STATISTICS    /* Cyclic reading of slave error registers */

#define  INCLUDE_RED_DEVICE         /* Redundancy Support */

#define  INCLUDE_SOE_SUPPORT        /* Currently not supported: ServoDrive over EtherCAT */
/*#define  SIMULATION_MODE_DEFAULT   */ /* default simulation mode (run the stack w/o slaves) */

#undef  INCLUDE_COE_PDO_SUPPORT     /* currently not supported */

#undef  INCLUDE_FOE_LOGICAL_STATE_MBOX_POLLING  /* currently not supported */

#define INCLUDE_FOE_SUPPORT         /* File access over EtherCAT support */
#define INCLUDE_EOE_ENDPOINT        /* Ethernet over EtherCAT end point support */
/* #define INCLUDE_VOE_SUPPORT         / * Support for Vendor specific access over EtherCAT * / */


#undef VLAN_FRAME_SUPPORT           /* VLAN Frame support */
#undef INCLUDE_VARREAD              /* Read PD variable tags from XML support */

#define INCLUDE_HOTCONNECT

#undef INCLUDE_OSPERF

#if (defined __MET__)
#undef INCLUDE_MASTER_SYNC
#undef INCLUDE_MASTER_OBD
#undef INCLUDE_SLAVE_STATISTICS
#undef INCLUDE_FOE_SUPPORT
#undef INCLUDE_TIMEMSRMT
#undef VLAN_FRAME_SUPPORT
#endif

#endif /* INC_ECTYPE */

#endif

/*-END OF SOURCE FILE--------------------------------------------------------*/
