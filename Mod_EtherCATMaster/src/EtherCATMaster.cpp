/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		EA
 * 	@date		2016
 * 	@brief		Controls the EtherCAT bus
 */

#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <sched.h>
#include <sys/time.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/prctl.h>

#include "EtherCATMaster/EtherCATMaster.h"
#include "module/moduleLogger.h"

#include "common/connectionConfiguration.h"
#include "common/systemConfiguration.h"
#include "system/realTimeSupport.h"

using Poco::XML::DOMParser;
using Poco::XML::InputSource;
using Poco::XML::Document;
using Poco::XML::NodeIterator;
using Poco::XML::NodeFilter;
using Poco::XML::Node;
using Poco::XML::AutoPtr;
using Poco::Exception;
using Poco::XML::NodeList;
using Poco::XML::NamedNodeMap;
using Poco::XML::SAXParser;
using Poco::XML::XMLReader;
using Poco::XML::LexicalHandler;
using Poco::FastMutex;

using namespace precitec::interface;

// folgende CLOCK... verwenden
#define CLOCK_TO_USE CLOCK_MONOTONIC

// folgendes definiert die Anzahl ns pro Sekunde
#define NSEC_PER_SEC    (1000000000)

// Folgendes definiert eine Zykluszeit von 1ms
#define CYCLE_TIME_NS   (1000000)

// folgendes definiert den Faktor fuer das Oversampling (maximal 100 !)
#define EL3702_OVERSAMP_FACTOR       100
#define FRONTEND_OVERSAMP_FACTOR     50

// Hilfs-Makros fuer Berechnungen im Nanosekunden-Bereich
#define TIMESPEC2NS(T) ((uint64_t) (T).tv_sec * NSEC_PER_SEC + (T).tv_nsec)
#define DIFF_NS(A, B) (((B).tv_sec - (A).tv_sec) * NSEC_PER_SEC + (B).tv_nsec - (A).tv_nsec)

// Debug Funktionalitaeten Ein- bzw. Aus-Schalten
#define ECAT_DEBUG_OUTPUTS           0
#define ECAT_CYCLE_TIMING_PRINTOUTS  0
#define ECAT_CYCLE_VIA_SERIAL_PORT   0
#define ECAT_WITH_DEBUG_DATA_THREAD  0
#define ECAT_TEST_JUMP_BETW_CYCLES   0
#define WITH_SYNC_REF_COUNTER        0

#define PROCESS_MONITOR_INIT_DEBUG   0
#define PROCESS_MONITOR_DEBUG        0

namespace precitec
{

using namespace interface;

namespace ethercat
{

///////////////////////////////////////////////////////////
// EtherCAT slave definitions
///////////////////////////////////////////////////////////

// Kubus Gateway --------------------------------

static ec_pdo_entry_info_t KUNBUS_pdo_entries_input[] = {
    {0x2000,  1, 8}, // Input Byte 1
    {0x2000,  2, 8}, // Input Byte 2
    {0x2000,  3, 8}, // Input Byte 3
    {0x2000,  4, 8}, // Input Byte 4
    {0x2000,  5, 8}, // Input Byte 5
    {0x2000,  6, 8}, // Input Byte 6
    {0x2000,  7, 8}, // Input Byte 7
    {0x2000,  8, 8}, // Input Byte 8
    {0x2000,  9, 8}, // Input Byte 9
    {0x2000, 10, 8}, // Input Byte 10
    {0x2000, 11, 8}, // Input Byte 11
    {0x2000, 12, 8}, // Input Byte 12
    {0x2000, 13, 8}, // Input Byte 13
    {0x2000, 14, 8}, // Input Byte 14
    {0x2000, 15, 8}, // Input Byte 15
    {0x2000, 16, 8}, // Input Byte 16
    {0x2000, 17, 8}, // Input Byte 17
    {0x2000, 18, 8}, // Input Byte 18
    {0x2000, 19, 8}, // Input Byte 19
    {0x2000, 20, 8}  // Input Byte 20
};

static ec_pdo_entry_info_t KUNBUS_pdo_entries_output[] = {
    {0x2100,  1, 8}, // Output Byte 1
    {0x2100,  2, 8}, // Output Byte 2
    {0x2100,  3, 8}, // Output Byte 3
    {0x2100,  4, 8}, // Output Byte 4
    {0x2100,  5, 8}, // Output Byte 5
    {0x2100,  6, 8}, // Output Byte 6
    {0x2100,  7, 8}, // Output Byte 7
    {0x2100,  8, 8}, // Output Byte 8
    {0x2100,  9, 8}, // Output Byte 9
    {0x2100, 10, 8}, // Output Byte 10
    {0x2100, 11, 8}, // Output Byte 11
    {0x2100, 12, 8}, // Output Byte 12
    {0x2100, 13, 8}, // Output Byte 13
    {0x2100, 14, 8}, // Output Byte 14
    {0x2100, 15, 8}, // Output Byte 15
    {0x2100, 16, 8}, // Output Byte 16
    {0x2100, 17, 8}, // Output Byte 17
    {0x2100, 18, 8}, // Output Byte 18
    {0x2100, 19, 8}, // Output Byte 19
    {0x2100, 20, 8}  // Output Byte 20
};

static ec_pdo_info_t KUNBUS_pdos[] = {
    {0x1600, 20, &KUNBUS_pdo_entries_output[0]},
    {0x1A00, 20, &KUNBUS_pdo_entries_input[0]}
};

static ec_sync_info_t KUNBUS_syncs[] = {
    {2, EC_DIR_OUTPUT, 1, &KUNBUS_pdos[0]},
    {3, EC_DIR_INPUT,  1, &KUNBUS_pdos[1]},
    {0xff}
};

// Anybus Gateway -------------------------------

static ec_pdo_entry_info_t ANYBUS_pdo_entries_input[] = {
    {0x2000,  1, 8}, // Input Byte 1
    {0x2000,  2, 8}, // Input Byte 2
    {0x2000,  3, 8}, // Input Byte 3
    {0x2000,  4, 8}, // Input Byte 4
    {0x2000,  5, 8}, // Input Byte 5
    {0x2000,  6, 8}, // Input Byte 6
    {0x2000,  7, 8}, // Input Byte 7
    {0x2000,  8, 8}, // Input Byte 8
    {0x2000,  9, 8}, // Input Byte 9
    {0x2000, 10, 8}, // Input Byte 10
    {0x2000, 11, 8}, // Input Byte 11
    {0x2000, 12, 8}, // Input Byte 12
    {0x2000, 13, 8}, // Input Byte 13
    {0x2000, 14, 8}, // Input Byte 14
    {0x2000, 15, 8}, // Input Byte 15
    {0x2000, 16, 8}, // Input Byte 16
    {0x2000, 17, 8}, // Input Byte 17
    {0x2000, 18, 8}, // Input Byte 18
    {0x2000, 19, 8}, // Input Byte 19
    {0x2000, 20, 8}  // Input Byte 20
};

static ec_pdo_entry_info_t ANYBUS_pdo_entries_output[] = {
    {0x2100,  1, 8}, // Output Byte 1
    {0x2100,  2, 8}, // Output Byte 2
    {0x2100,  3, 8}, // Output Byte 3
    {0x2100,  4, 8}, // Output Byte 4
    {0x2100,  5, 8}, // Output Byte 5
    {0x2100,  6, 8}, // Output Byte 6
    {0x2100,  7, 8}, // Output Byte 7
    {0x2100,  8, 8}, // Output Byte 8
    {0x2100,  9, 8}, // Output Byte 9
    {0x2100, 10, 8}, // Output Byte 10
    {0x2100, 11, 8}, // Output Byte 11
    {0x2100, 12, 8}, // Output Byte 12
    {0x2100, 13, 8}, // Output Byte 13
    {0x2100, 14, 8}, // Output Byte 14
    {0x2100, 15, 8}, // Output Byte 15
    {0x2100, 16, 8}, // Output Byte 16
    {0x2100, 17, 8}, // Output Byte 17
    {0x2100, 18, 8}, // Output Byte 18
    {0x2100, 19, 8}, // Output Byte 19
    {0x2100, 20, 8}  // Output Byte 20
};

static ec_pdo_info_t ANYBUS_pdos[] = {
    {0x1600, 20, &ANYBUS_pdo_entries_output[0]},
    {0x1A00, 20, &ANYBUS_pdo_entries_input[0]}
};

static ec_sync_info_t ANYBUS_syncs[] = {
    {2, EC_DIR_OUTPUT, 1, &ANYBUS_pdos[0]},
    {3, EC_DIR_INPUT,  1, &ANYBUS_pdos[1]},
    {0xff}
};

// EL1018 Digital in ----------------------------

static ec_pdo_entry_info_t EL1018_pdo_entries_input[] = {
    {0x6000, 1, 1}, // Value 1
    {0x6010, 1, 1}, // Value 2
    {0x6020, 1, 1}, // Value 3
    {0x6030, 1, 1}, // Value 4
    {0x6040, 1, 1}, // Value 5
    {0x6050, 1, 1}, // Value 6
    {0x6060, 1, 1}, // Value 7
    {0x6070, 1, 1}  // Value 8
};

static ec_pdo_info_t EL1018_pdos[] = {
    {0x1A00, 1, &EL1018_pdo_entries_input[0]},
    {0x1A01, 1, &EL1018_pdo_entries_input[1]},
    {0x1A02, 1, &EL1018_pdo_entries_input[2]},
    {0x1A03, 1, &EL1018_pdo_entries_input[3]},
    {0x1A04, 1, &EL1018_pdo_entries_input[4]},
    {0x1A05, 1, &EL1018_pdo_entries_input[5]},
    {0x1A06, 1, &EL1018_pdo_entries_input[6]},
    {0x1A07, 1, &EL1018_pdo_entries_input[7]}
};

static ec_sync_info_t EL1018_syncs[] = {
    {0, EC_DIR_INPUT, 8, EL1018_pdos},
    {0xff}
};

// EL2008 Digital out ---------------------------

static ec_pdo_entry_info_t EL2008_pdo_entries_output[] = {
    {0x7000, 1, 1}, // Value 1
    {0x7010, 1, 1}, // Value 2
    {0x7020, 1, 1}, // Value 3
    {0x7030, 1, 1}, // Value 4
    {0x7040, 1, 1}, // Value 5
    {0x7050, 1, 1}, // Value 6
    {0x7060, 1, 1}, // Value 7
    {0x7070, 1, 1}  // Value 8
};

static ec_pdo_info_t EL2008_pdos[] = {
    {0x1600, 1, &EL2008_pdo_entries_output[0]},
    {0x1601, 1, &EL2008_pdo_entries_output[1]},
    {0x1602, 1, &EL2008_pdo_entries_output[2]},
    {0x1603, 1, &EL2008_pdo_entries_output[3]},
    {0x1604, 1, &EL2008_pdo_entries_output[4]},
    {0x1605, 1, &EL2008_pdo_entries_output[5]},
    {0x1606, 1, &EL2008_pdo_entries_output[6]},
    {0x1607, 1, &EL2008_pdo_entries_output[7]}
};

static ec_sync_info_t EL2008_syncs[] = {
    {0, EC_DIR_OUTPUT, 8, EL2008_pdos},
    {0xff}
};

// EL3102 Analog in -----------------------------

static ec_pdo_entry_info_t EL3102_pdo_entries_input[] = {
    {0x3101, 1,  8}, // channel 1 status
    {0x3101, 2, 16}, // channel 1 value
    {0x3102, 1,  8}, // channel 2 status
    {0x3102, 2, 16}  // channel 2 value
};

static ec_pdo_info_t EL3102_pdos[] = {
    {0x1A00, 2, &EL3102_pdo_entries_input[0]},
    {0x1A01, 2, &EL3102_pdo_entries_input[2]}
};

static ec_sync_info_t EL3102_syncs[] = {
    {2, EC_DIR_OUTPUT},
    {3, EC_DIR_INPUT, 2, EL3102_pdos},
    {0xff}
};

// EL3702 Analog in -----------------------------

// structs for PDO configuration are defined dynamically, dependent on EL3702_OVERSAMP_FACTOR
// this is done after recognizing a EL3702 terminal in function InitializeEtherCATMaster

static ec_pdo_entry_info_t EL3702_pdo_entries_input_chan1[101];
static ec_pdo_entry_info_t EL3702_pdo_entries_input_chan2[101];
static ec_pdo_entry_info_t EL3702_pdo_entries_input_add[1];
static ec_pdo_info_t EL3702_pdos_chan1[101];
static ec_pdo_info_t EL3702_pdos_chan2[101];
static ec_pdo_info_t EL3702_pdos_add[1];
static ec_sync_info_t EL3702_syncs[10];

// EL4102 Analog out ----------------------------

static ec_pdo_entry_info_t EL4102_pdo_entries_output[] = {
    {0x3001, 1, 16}, // channel 1 value
    {0x3002, 1, 16}, // channel 2 value
};

static ec_pdo_info_t EL4102_pdos[] = {
    {0x1600, 1, &EL4102_pdo_entries_output[0]},
    {0x1601, 1, &EL4102_pdo_entries_output[1]}
};

static ec_sync_info_t EL4102_syncs[] = {
    {2, EC_DIR_OUTPUT, 2, EL4102_pdos},
    {3, EC_DIR_INPUT},
    {0xff}
};

// EL4132 Analog out ----------------------------

static ec_pdo_entry_info_t EL4132_pdo_entries_output[] = {
    {0x3001, 1, 16}, // channel 1 value
    {0x3002, 1, 16}, // channel 2 value
};

static ec_pdo_info_t EL4132_pdos[] = {
    {0x1600, 1, &EL4132_pdo_entries_output[0]},
    {0x1601, 1, &EL4132_pdo_entries_output[1]}
};

static ec_sync_info_t EL4132_syncs[] = {
    {2, EC_DIR_OUTPUT, 2, EL4132_pdos},
    {3, EC_DIR_INPUT},
    {0xff}
};

// EL5101 Incremental encoder input -------------

static ec_pdo_entry_info_t EL5101_pdo_entries_input[] = {
    {0x6010,  1, 1},  // Status Bit 1
    {0x6010,  2, 1},  // Status Bit 2
    {0x6010,  3, 1},  // Status Bit 3
    {0x6010,  4, 1},  // Status Bit 4
    {0x6010,  5, 1},  // Status Bit 5
    {0x6010,  6, 1},  // Status Bit 6
    {0x6010,  7, 1},  // Status Bit 7
    {0x6010,  8, 1},  // Status Bit 8
    {0x6010,  9, 1},  // Status Bit 9
    {0x6010, 10, 1},  // Status Bit 10
    {0x6010, 11, 1},  // Status Bit 11
    {0x6010, 12, 1},  // Status Bit 12
    {0x6010, 13, 1},  // Status Bit 13
    {0x6010, 14, 1},  // Status Bit 14
    {0x6010, 15, 1},  // Status Bit 15
    {0x6010, 16, 1},  // Status Bit 16
    {0x6010, 17, 32}, // Counter value
    {0x6010, 18, 32}  // Latch value
};

static ec_pdo_entry_info_t EL5101_pdo_entries_output[] = {
    {0x7010,  1, 1},  // Control_Enable latch C
    {0x7010,  2, 1},  // Control_Enable latch extern on
    {0x7010,  3, 1},  // Control_Set counter
    {0x7010,  4, 1},  // Control_Enable latch extern on
    {0x7010,  5, 4},  // 
    {0x7010,  6, 8},  // 
    {0x7010, 11, 32}  // Set counter value
};

static ec_pdo_info_t EL5101_pdos[] = {
    {0x1603,  7, &EL5101_pdo_entries_output[0]},
    {0x1A04, 18, &EL5101_pdo_entries_input[0]}
};

static ec_sync_info_t EL5101_syncs[] = {
    {2, EC_DIR_OUTPUT, 1, &EL5101_pdos[0]},
    {3, EC_DIR_INPUT,  1, &EL5101_pdos[1]},
    {0xff}
};

// EL5151 Incremental encoder input -------------

static ec_pdo_entry_info_t EL5151PdoEntriesInput[] =
{
    {0x6000,  1, 1},  // Status Bit 1
    {0x6000,  2, 1},  // Status Bit 2
    {0x6000,  3, 1},  // Status Bit 3
    {0x6000,  4, 1},  // Status Bit 4
    {0x6000,  5, 1},  // Status Bit 5
    {0x6000,  6, 1},  // Status Bit 6
    {0x6000,  7, 1},  // Status Bit 7
    {0x6000,  8, 1},  // Status Bit 8
    {0x6000,  9, 1},  // Status Bit 9
    {0x6000, 10, 1},  // Status Bit 10
    {0x6000, 11, 1},  // Status Bit 11
    {0x6000, 12, 1},  // Status Bit 12
    {0x6000, 13, 1},  // Status Bit 13
    {0x6000, 14, 1},  // Status Bit 14
    {0x6000, 15, 1},  // Status Bit 15
    {0x6000, 16, 1},  // Status Bit 16
    {0x6000, 17, 32}, // Counter value
    {0x6000, 18, 32}  // Latch value
};

static ec_pdo_entry_info_t EL5151PdoEntriesOutput[] =
{
    {0x7000,  1, 1},  // Control_Enable latch C
    {0x7000,  2, 1},  // Control_Enable latch extern pos. edge on
    {0x7000,  3, 1},  // Control_Set counter
    {0x7000,  4, 1},  // Control_Enable latch extern neg. edge on
    {0x7000,  5, 4},  //
    {0x7000,  6, 8},  //
    {0x7000, 11, 32}  // Set counter value
};

static ec_pdo_info_t EL5151Pdos[] =
{
    {0x1600,  7, &EL5151PdoEntriesOutput[0]},
    {0x1A00, 18, &EL5151PdoEntriesInput[0]}
};

static ec_sync_info_t EL5151Syncs[] =
{
    {2, EC_DIR_OUTPUT, 1, &EL5151Pdos[0]},
    {3, EC_DIR_INPUT,  1, &EL5151Pdos[1]},
    {0xff}
};

// COPLEY ACCELNET axis controller --------------

static ec_pdo_entry_info_t ACCELNET_pdo_entries_input[] = {
    {0x6041,  0, 16}, // Status Word
    {0x6061,  0, 8},  // Modes Of Operation Display
    {0x1001,  0, 8},  // Error Register
    {0x1002,  0, 32}, // Manufacturer Status Register
    {0x6064,  0, 32}, // Position Actual Value
    {0x606C,  0, 32}, // Actual Motor Velocity
    {0x6077,  0, 16}  // Torque Actual Value
};

static ec_pdo_entry_info_t ACCELNET_pdo_entries_output[] = {
    {0x6040,  0, 16}, // Control Word
    {0x6060,  0, 8},  // Modes Of Operation
    {0x607A,  0, 32}, // Profile Target Position
    {0x6081,  0, 32}, // Profile Velocity
    {0x6083,  0, 32}, // Profile Acceleration
    {0x6084,  0, 32}, // Profile Deceleration
    {0x6098,  0, 8},  // Homing Method
    {0x607C,  0, 32}, // Home Offset
    {0x6099,  1, 32}, // Home Velocity Fast
    {0x6099,  2, 32}  // Home Velocity Slow
};

static ec_pdo_info_t ACCELNET_pdos[] = {
    {0x1600, 2, &ACCELNET_pdo_entries_output[0]},
    {0x1601, 2, &ACCELNET_pdo_entries_output[2]},
    {0x1602, 2, &ACCELNET_pdo_entries_output[4]},
    {0x1603, 2, &ACCELNET_pdo_entries_output[6]},
    {0x1604, 2, &ACCELNET_pdo_entries_output[8]},
    {0x1A00, 4, &ACCELNET_pdo_entries_input[0]},
    {0x1A01, 2, &ACCELNET_pdo_entries_input[4]},
    {0x1A02, 1, &ACCELNET_pdo_entries_input[6]}
};

static ec_sync_info_t ACCELNET_syncs[] = {
    {2, EC_DIR_OUTPUT, 5, &ACCELNET_pdos[0]},
    {3, EC_DIR_INPUT,  3, &ACCELNET_pdos[5]},
    {0xff}
};

// MAXON EPOS4 axis controller --------------

// max. 12 entries, max. 40 bytes in sum
static ec_pdo_entry_info_t EPOS4_pdo_entries_input[] = {
    {0x6041,  0, 16}, // Status Word
    {0x6061,  0, 8},  // Modes Of Operation Display
    {0x603F,  0, 16}, // Error Code
    {0x6064,  0, 32}, // Position Actual Value
    {0x606C,  0, 32}, // Actual Motor Velocity
    {0x6077,  0, 16}, // Torque Actual Value
    {0x3141,  1, 16}, // Digital inputs logic state, Pin state
    {0x3150,  1, 16}, // Digital outputs logic state, Pin state
    {0x60F4,  0, 32}  // Following error actual value
};

// max. 12 entries, max. 40 bytes in sum
static ec_pdo_entry_info_t EPOS4_pdo_entries_output[] = {
    {0x6040,  0, 16}, // Control Word
    {0x6060,  0, 8},  // Modes Of Operation
    {0x607A,  0, 32}, // Profile Target Position
    {0x6081,  0, 32}, // Profile Velocity
    {0x6083,  0, 32}, // Profile Acceleration
    {0x6084,  0, 32}, // Profile Deceleration
    {0x6098,  0, 8},  // Homing Method
    {0x6099,  1, 32}, // Home Velocity Fast
    {0x6099,  2, 32}  // Home Velocity Slow
};

static ec_pdo_info_t EPOS4_pdos_input[] = {
    {0x1A00, 9, &EPOS4_pdo_entries_input[0]}
};

static ec_pdo_info_t EPOS4_pdos_output[] = {
    {0x1600, 9, &EPOS4_pdo_entries_output[0]}
};

// only 1 RxPDO/TxPDO per Syncmanager possible !
static ec_sync_info_t EPOS4_syncs[] = {
    {0, EC_DIR_OUTPUT, 0, NULL, EC_WD_DISABLE},
    {1, EC_DIR_INPUT,  0, NULL, EC_WD_DISABLE},
    {2, EC_DIR_OUTPUT, 1, &EPOS4_pdos_output[0], EC_WD_ENABLE},
    {3, EC_DIR_INPUT,  1, &EPOS4_pdos_input[0], EC_WD_DISABLE},
    {0xff}
};

// EK1310 EtherCAT-P coupler  -------------------------

static ec_pdo_entry_info_t EK1310_pdo_entries_input[] = {
    {0x6000, 0x01, 1}, /* Undervoltage */
    {0x6010, 0x01, 1}  /* Undervoltage */
};

static ec_pdo_info_t EK1310_pdos[] = {
    {0x1a00, 1, &EK1310_pdo_entries_input[0]}, /* Status Us */
    {0x1a01, 1, &EK1310_pdo_entries_input[1]}  /* Status Up */
};

static ec_sync_info_t EK1310_syncs[] = {
    {0, EC_DIR_INPUT, 2, EK1310_pdos, EC_WD_DISABLE},
    {0xff}
};

// FRONTEND --------------------------------

// structs for PDO configuration are defined dynamically, dependent on FRONTEND_OVERSAMP_FACTOR
// this is done after recognizing a FRONTEND terminal in function InitializeEtherCATMaster

static ec_pdo_entry_info_t FRONTEND_pdo_entries_output[5];
static ec_pdo_entry_info_t FRONTEND_pdo_entries_input[401];
static ec_pdo_info_t FRONTEND_pdos[25];
static ec_sync_info_t FRONTEND_syncs[10];

///////////////////////////////////////////////////////////
// Prototyp fuer Thread Funktions
///////////////////////////////////////////////////////////

// Thread Funktion muss ausserhalb der Klasse sein

void* ECATDebugDataThread(void *p_pArg);

void* ECATCycleTaskThread(void *p_pArg);

void* CheckProcessesThread(void *p_pArg);

///////////////////////////////////////////////////////////
// global variables for debugging purposes
///////////////////////////////////////////////////////////

int g_oDebugSerialFd;
int g_oDTR01_flag;
int g_oRTS02_flag;

ec_slave_config_t *g_pFirstEK1100Slave = nullptr;
ec_slave_config_t *g_pFirstEL1018Slave = nullptr;
ec_slave_config_t *g_pFirstEL2008Slave = nullptr;
ec_slave_config_t *g_pFirstEL3702Slave = nullptr;
ec_slave_config_t *g_pFirstFRONTENDSlave = nullptr;

static bool s_threadsStopped = false;

#if WITH_SYNC_REF_COUNTER
static unsigned int g_oSyncRefCounter = 0;
#endif

///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////

EtherCATMaster::EtherCATMaster(TEthercatInputs<EventProxy>& p_rEthercatInputsProxy, TEthercatInputsToService<EventProxy> &p_rEthercatInputsToServiceProxy):
				m_rEthercatInputsProxy(p_rEthercatInputsProxy),
				m_rEthercatInputsToServiceProxy(p_rEthercatInputsToServiceProxy),
				m_cStateFd(-1),
                m_oSystemReadyOffset(-1),
                m_oSystemReadyMask(0x00),
                m_oSystemReadyState(true),
                m_oSystemReadyOffsetFull(-1),
                m_oSystemReadyMaskFull(0x00),
                m_oSystemErrorFieldOffset(-1),
                m_oSystemErrorFieldMask1(0x00),
                m_oSystemErrorFieldMask2(0x00),
                m_oSystemErrorFieldValue(0),
                m_oSystemErrorFieldOffsetFull(-1),
                m_oSystemErrorFieldMask1Full(0x00),
                m_oSystemErrorFieldMask2Full(0x00)
{
	SAXParser parser;
	parser.setFeature(XMLReader::FEATURE_NAMESPACES, true);
	parser.setFeature(XMLReader::FEATURE_NAMESPACE_PREFIXES, true);
	parser.setContentHandler(&m_oConfigParser);
	parser.setProperty(XMLReader::PROPERTY_LEXICAL_HANDLER, static_cast<LexicalHandler*> (&m_oConfigParser));

	try
	{
		std::string ConfigFilename(getenv("WM_BASE_DIR"));
		ConfigFilename += "/config/VI_Config.xml";
		Poco::Path configPath( ConfigFilename );

		std::cout << "parsing configuration file: " << configPath.toString() << std::endl;
		wmLogTr(eInfo, "QnxMsg.VI.XMLFileRead", "parsing configuration file: %s\n", configPath.toString().c_str());
		parser.parse( configPath.toString() );
		std::cout << "configuration file successful parsed: " << configPath.toString() << std::endl;
		wmLogTr(eInfo, "QnxMsg.VI.XMLFileReadOK", "configuration file successful parsed: %s\n", configPath.toString().c_str());
	}
	catch (Poco::Exception& e)
	{
		// Fehler beim Parsen der VI_Config.xml
		std::cerr << "error while parsing configuration file: " << e.displayText() << std::endl;
		wmLogTr(eError, "QnxMsg.VI.XMLFileReadErr", "error while parsing configuration file: %s\n", e.displayText().c_str());

		// VI_Config_Standard.xml als Notbehelf in VI_Config.xml kopieren
		std::string oSrcConfigFilename(getenv("WM_BASE_DIR"));
		oSrcConfigFilename += "/config/VI_Config_Standard.xml";
		std::string oDestConfigFilename(getenv("WM_BASE_DIR"));
		oDestConfigFilename += "/config/VI_Config.xml";
		std::string oCommandString = "cp " + oSrcConfigFilename + " " + oDestConfigFilename;
		if (std::system(oCommandString.c_str()) == -1)
		{
			std::cerr << "error while copying Input/Output Configuration file" << std::endl;
			wmLogTr(eError, "QnxMsg.Misc.VIConfCopyErr", "error while copying Input/Output Configuration file\n");
		}

		// Meldung ausgeben, dass VI_Config.xml nicht korrekt ist und System in Nicht-Betriebsbereit-Zustand versetzen
		std::cerr << "Input/Output Configuration file is damaged or missing" << std::endl;
		wmFatal(eDataConsistency, "QnxMsg.Misc.VIConfIsNew1", "Input/Output Configuration file is damaged or missing\n");
		std::cerr << "Input/Output Configuration is lost and must be set again" << std::endl;
		wmFatal(eDataConsistency, "QnxMsg.Misc.VIConfIsNew2", "Input/Output Configuration is lost and must be set again\n");

		// Neue (nicht korrekte) VI_Config.xml parsen, damit Prozess in lauffaehigen Zustand uebergeht
		Poco::Path configPath( oDestConfigFilename );
		parser.parse( configPath.toString() );
	}
	catch (...)
	{
		// Fehler beim Einlesen der VI_Config.xml
		std::string oGenError = "general error";
		std::cerr << "error while parsing configuration file: " << oGenError << std::endl;
		wmLogTr(eError, "QnxMsg.VI.XMLFileReadErr", "error while parsing configuration file: %s\n", oGenError.c_str());

		// VI_Config_Standard.xml als Notbehelf in VI_Config.xml kopieren
		std::string oSrcConfigFilename(getenv("WM_BASE_DIR"));
		oSrcConfigFilename += "/config/VI_Config_Standard.xml";
		std::string oDestConfigFilename(getenv("WM_BASE_DIR"));
		oDestConfigFilename += "/config/VI_Config.xml";
		std::string oCommandString = "cp " + oSrcConfigFilename + " " + oDestConfigFilename;
		if (std::system(oCommandString.c_str()) == -1)
		{
			std::cerr << "error while copying Input/Output Configuration file" << std::endl;
			wmLogTr(eError, "QnxMsg.Misc.VIConfCopyErr", "error while copying Input/Output Configuration file\n");
		}

		// Meldung ausgeben, dass VI_Config.xml nicht korrekt ist und System in Nicht-Betriebsbereit-Zustand versetzen
		std::cerr << "Input/Output Configuration file is damaged or missing" << std::endl;
		wmFatal(eDataConsistency, "QnxMsg.Misc.VIConfIsNew1", "Input/Output Configuration file is damaged or missing\n");
		std::cerr << "Input/Output Configuration is lost and must be set again" << std::endl;
		wmFatal(eDataConsistency, "QnxMsg.Misc.VIConfIsNew2", "Input/Output Configuration is lost and must be set again\n");

		// Neue (nicht korrekte) VI_Config.xml parsen, damit Prozess in lauffaehigen Zustand uebergeht
		Poco::Path configPath( oDestConfigFilename );
		parser.parse( configPath.toString() );
	}

	// check if SystemConfig.xml is newly written
	bool tempBool = SystemConfiguration::instance().getBool("File_New_Created", false);
	if (tempBool)
	{
		wmFatal(eDataConsistency, "QnxMsg.Misc.SysConfIsNew1", "System Configuration file is damaged or missing\n");
		wmFatal(eDataConsistency, "QnxMsg.Misc.SysConfIsNew2", "System Configuration is lost and must be set again\n");
		SystemConfiguration::instance().setBool("File_New_Created", false);
	}

    // SystemConfig Switches for separate Fieldbus board
    m_oFieldbusViaSeparateFieldbusBoard = SystemConfiguration::instance().getBool("FieldbusViaSeparateFieldbusBoard", false);
    wmLog(eDebug, "m_oFieldbusViaSeparateFieldbusBoard (bool): %d\n", m_oFieldbusViaSeparateFieldbusBoard);

	///////////////////////////////////////////////////////
	// Inits
	///////////////////////////////////////////////////////

	for(int i = 0;i < MAX_NBR_PER_SLAVE;i++)
    {
        GATEWAY_Input_Offset[i] = 0;
        GATEWAY_Output_Offset[i] = 0;
        EL1018_Input_Offset[i] = 0;
        EL2008_Output_Offset[i] = 0;
        EL3102_Input_Offset[i] = 0;
        EL3702_CH1_Input_Offset[i] = 0;
        EL3702_CH2_Input_Offset[i] = 0;
        EL3702_Add_Input_Offset[i] = 0;
        EL4102_Output_Offset[i] = 0;
        EL4132_Output_Offset[i] = 0;
        EL5101_Input_Offset[i] = 0;
        EL5101_Output_Offset[i] = 0;
        EL5151InputOffset[i] = 0;
        EL5151OutputOffset[i] = 0;
        ACCELNET_Input_Offset[i] = 0;
        ACCELNET_Output_Offset[i] = 0;
        EPOS4_Input_Offset[i] = 0;
        EPOS4_Output_Offset[i] = 0;
        EK1310_Input_Offset[i] = 0;
        FRONTEND_Input_Offset[i] = 0;
        FRONTEND_Output_Offset[i] = 0;
    }

    pthread_mutexattr_t mutexAttribute;
    pthread_mutexattr_init(&mutexAttribute);
    pthread_mutexattr_setprotocol(&mutexAttribute, PTHREAD_PRIO_INHERIT);

	int retValue = pthread_mutex_init(&oEL2008OutLock, &mutexAttribute);
	if (retValue != 0)
	{
        wmLogTr(eError, "QnxMsg.VI.ECMMutexInitFailed", "Initialization of Mutex %s failed (%s) !\n", "oEL2008OutLock", strerror(retValue));
	}
	for(int i = 0;i < MAX_EL2008_COUNT;i++)
	{
		m_oEL2008OutputBuffer[i] = 0x00;
	}

	retValue = pthread_mutex_init(&oEL4102OutLock, &mutexAttribute);
	if (retValue != 0)
	{
        wmLogTr(eError, "QnxMsg.VI.ECMMutexInitFailed", "Initialization of Mutex %s failed (%s) !\n", "oEL4102OutLock", strerror(retValue));
	}
	for(int i = 0;i < MAX_EL4102_COUNT;i++)
	{
		m_oEL4102OutputBuffer[i][0] = 0x00; // Channel 1
		m_oEL4102OutputBuffer[i][1] = 0x00; // Channel 2
	}

	retValue = pthread_mutex_init(&oEL4132OutLock, &mutexAttribute);
	if (retValue != 0)
	{
       wmLogTr(eError, "QnxMsg.VI.ECMMutexInitFailed", "Initialization of Mutex %s failed (%s) !\n", "oEL4132OutLock", strerror(retValue));
	}
	for(int i = 0;i < MAX_EL4132_COUNT;i++)
	{
		m_oEL4132OutputBuffer[i][0] = 0x00; // Channel 1
		m_oEL4132OutputBuffer[i][1] = 0x00; // Channel 2
	}

	retValue = pthread_mutex_init(&oEL5101OutLock, &mutexAttribute);
	if (retValue != 0)
	{
       wmLogTr(eError, "QnxMsg.VI.ECMMutexInitFailed", "Initialization of Mutex %s failed (%s) !\n", "oEL5101OutLock", strerror(retValue));
	}
	for(int i = 0;i < MAX_EL5101_COUNT;i++)
	{
		m_oEL5101OutputBuffer[i][0] = (uint32_t)0x00; // command
		m_oEL5101OutputBuffer[i][1] = (uint32_t)0x00; // setCounterValue
	}

    retValue = pthread_mutex_init(&m_EL5151OutputLock, &mutexAttribute);
    if (retValue != 0)
    {
        wmLogTr(eError, "QnxMsg.VI.ECMMutexInitFailed", "Initialization of Mutex %s failed (%s) !\n", "m_EL5151OutputLock", strerror(retValue));
    }
    for (int i = 0; i < MAX_EL5151_COUNT; i++)
    {
        m_EL5151OutputBuffer[i][0] = (uint32_t)0x00; // command
        m_EL5151OutputBuffer[i][1] = (uint32_t)0x00; // setCounterValue
    }

	retValue = pthread_mutex_init(&oGatewayOutLock, &mutexAttribute);
	if (retValue != 0)
	{
        wmLogTr(eError, "QnxMsg.VI.ECMMutexInitFailed", "Initialization of Mutex %s failed (%s) !\n", "oGatewayOutLock", strerror(retValue));
	}
	for(int i = 0;i < MAX_GATEWAY_COUNT;i++)
	{
		for(int j = 0;j < MAX_GATEWAY_OUTPUT_LENGTH;j++)
		{
			m_oGatewayOutputBuffer[i][j] = (uint8_t)0x00;
		}
	}

	retValue = pthread_mutex_init(&oACCELNETOutLock, &mutexAttribute);
	if (retValue != 0)
	{
        wmLogTr(eError, "QnxMsg.VI.ECMMutexInitFailed", "Initialization of Mutex %s failed (%s) !\n", "oACCELNETOutLock", strerror(retValue));
	}
	for(int i = 0;i < MAX_ACCELNET_COUNT;i++)
	{
        memset(&m_oACCELNETOutputBuffer[i], 0x00, sizeof(EcatAxisOutput));
	}

    retValue = pthread_mutex_init(&oEPOS4OutLock, &mutexAttribute);
    if (retValue != 0)
    {
        wmLogTr(eError, "QnxMsg.VI.ECMMutexInitFailed", "Initialization of Mutex %s failed (%s) !\n", "oEPOS4OutLock", strerror(retValue));
    }
    for(int i = 0;i < MAX_EPOS4_COUNT;i++)
    {
        memset(&m_oEPOS4OutputBuffer[i], 0x00, sizeof(EcatAxisOutput));
        m_pEPOS4SlaveConfig[i] = nullptr;
        m_oEPOS4SlavePosition[i] = 9999;
    }

    retValue = pthread_mutex_init(&oFRONTENDOutLock, &mutexAttribute);
    if (retValue != 0)
    {
        wmLogTr(eError, "QnxMsg.VI.ECMMutexInitFailed", "Initialization of Mutex %s failed (%s) !\n", "oFRONTENDOutLock", strerror(retValue));
    }
    for(int i = 0;i < MAX_FRONTEND_COUNT;i++)
    {
        memset(&m_oFRONTENDOutputBuffer[i], 0x00, sizeof(EcatFRONTENDOutput));
        m_oFRONTENDOutputBuffer[i].m_oAmpCH1 = 1;
        m_oFRONTENDOutputBuffer[i].m_oAmpCH2 = 1;
        m_oFRONTENDOutputBuffer[i].m_oAmpCH3 = 1;
        m_oFRONTENDOutputBuffer[i].m_oAmpCH4 = 1;
        m_oFRONTENDOutputBuffer[i].m_oOversampling = 50;
    }

	retValue = pthread_mutex_init(&m_oCheckProcessesMutex, &mutexAttribute);
	if (retValue != 0)
	{
        wmLogTr(eError, "QnxMsg.VI.ECMMutexInitFailed", "Initialization of Mutex %s failed (%s) !\n", "m_oCheckProcessesMutex", strerror(retValue));
	}

    for(int i = 0;i < ANZ_BUFFER;i++)
    {
        for(int j = 0;j < 100;j++)
        {
            m_oOversampDebugBuffer[i][j] = 0;
        }
        m_oOversampDebugBufferCycCnt[i] = 0;
        m_oOversampDebugBufferTime1[i] = 0;
        m_oOversampDebugBufferTime2[i] = 0;
    }
    m_oWriteIndex = 0;
    m_oReadIndex = 0;
    m_oDebugFileFull = false;

    disableCStates();

 	///////////////////////////////////////////////////////
	// EtherCAT starten
	///////////////////////////////////////////////////////

	int retVal = InitializeEtherCATMaster();
	if (retVal == 0)
	{
        wmLogTr(eInfo, "QnxMsg.VI.ECMInitOk", "Initialization of EtherCATMaster was successful\n");
	}
	else
	{
        wmLogTr(eError, "QnxMsg.VI.ECMInitNotOk", "Initialization of EtherCATMaster was NOT successful\n");
	}

	retVal = StartupEtherCATMaster();
	if (retVal == 0)
	{
        wmLogTr(eInfo, "QnxMsg.VI.ECMStartOk", "Startup of EtherCATMaster was successful\n");
	}
	else
	{
        wmLogTr(eError, "QnxMsg.VI.ECMStartNotOk", "Startup of EtherCATMaster was NOT successful\n");
	}

	usleep(1000 * 1000);

#if ECAT_CYCLE_VIA_SERIAL_PORT
    ///////////////////////////////////////////////////////
    // open serial driver for debug outputs
    ///////////////////////////////////////////////////////
    g_oDebugSerialFd = open("/dev/ttyS0", O_RDWR | O_NOCTTY);
    printf("g_oDebugSerialFd: %d\n", g_oDebugSerialFd);
    if (g_oDebugSerialFd == -1)
    {
        printf("error in open\n");
        perror("");
    }
    g_oDTR01_flag = TIOCM_DTR;
    g_oRTS02_flag = TIOCM_RTS;
#endif
}

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////

EtherCATMaster::~EtherCATMaster(void)
{
    if (pthread_cancel(m_oCheckProcessesThread) != 0)
    {
		wmLog(eDebug, "was not able to abort thread\n");
    }

    if (pthread_cancel(m_oECATCycleTaskThread) != 0)
    {
		wmLog(eDebug, "was not able to abort thread\n");
    }

    if (pthread_cancel(m_oECATDebugDataThread) != 0)
    {
		wmLog(eDebug, "was not able to abort thread\n");
    }

#if ECAT_CYCLE_VIA_SERIAL_PORT
    close(g_oDebugSerialFd);
#endif

	pthread_mutex_destroy(&m_oCheckProcessesMutex);
	pthread_mutex_destroy(&oEPOS4OutLock);
	pthread_mutex_destroy(&oACCELNETOutLock);
    pthread_mutex_destroy(&m_EL5151OutputLock);
	pthread_mutex_destroy(&oEL5101OutLock);
	pthread_mutex_destroy(&oEL4132OutLock);
	pthread_mutex_destroy(&oEL4102OutLock);
	pthread_mutex_destroy(&oEL2008OutLock);
	pthread_mutex_destroy(&oGatewayOutLock);
	pthread_mutex_destroy(&oFRONTENDOutLock);

    // enable cstates
    if (m_cStateFd != -1)
    {
        close(m_cStateFd);
    }
}

///////////////////////////////////////////////////////////
// private members
///////////////////////////////////////////////////////////

void EtherCATMaster::disableCStates()
{
    struct stat s;
    if (stat("/dev/cpu_dma_latency", &s) == -1) {
        wmLog(eDebug, "ERROR: Could not stat /dev/cpu_dma_latency: %s\n", strerror(errno));
        wmLogTr(eError, "QnxMsg.VI.ChangeLatFailed", "Change of latency failed ! %s\n", "(001)");
        return;
    }

    m_cStateFd = open("/dev/cpu_dma_latency", O_RDWR);
    if (m_cStateFd == -1) {
        wmLog(eDebug, "ERROR: Failed to open /dev/cpu_dma_latency: %s\n", strerror(errno));
        wmLogTr(eError, "QnxMsg.VI.ChangeLatFailed", "Change of latency failed ! %s\n", "(002)");
        return;
    }

    const int32_t target = 0;
    if (write(m_cStateFd, &target, sizeof(target)) < 1) {
        wmLog(eDebug, "ERROR: Failed writing to /dev/cpu_dma_latency: %s\n", strerror(errno));
        wmLogTr(eError, "QnxMsg.VI.ChangeLatFailed", "Change of latency failed ! %s\n", "(003)");
        close(m_cStateFd);
        return;
    }
    wmLog(eDebug, "Adjusted /dev/cpu_dma_latency to have lower latency\n");
}

int EtherCATMaster::InitializeEtherCATMaster(void)
{
    char oDebugStrg[81];
	int oRetValue;

    m_pEtherCATMaster = ecrt_request_master(0);
    sprintf(oDebugStrg, "0x%08lX", (unsigned long)m_pEtherCATMaster);
    wmLog(eDebug, "Return of ecrt_request_master: %s\n", oDebugStrg);
    if (!m_pEtherCATMaster)
        return -1;

    m_oSlaveInfo4Service.clear();
    oRetValue = 0;
    int oPosition = -1;
    while (oRetValue == 0)
    {
		ec_slave_info_t oSlaveInfo;
		oPosition++;
		oRetValue = ecrt_master_get_slave(m_pEtherCATMaster, oPosition, &oSlaveInfo);
        wmLog(eDebug, "ecrt_master_get_slave oRetValue: %d\n", oRetValue);
		if (oRetValue == 0) // Slave gefunden
		{
			sprintf(oDebugStrg, "%d", oSlaveInfo.alias);
            wmLog(eDebug, "oSlaveInfo.alias:         %s\n", oDebugStrg);
			sprintf(oDebugStrg, "%d", oSlaveInfo.position);
            wmLog(eDebug, "oSlaveInfo.position:      %s\n", oDebugStrg);
			sprintf(oDebugStrg, "%08X", oSlaveInfo.vendor_id);
            wmLog(eDebug, "oSlaveInfo.vendor_id:     %s\n", oDebugStrg);
			sprintf(oDebugStrg, "%08X", oSlaveInfo.product_code);
            wmLog(eDebug, "oSlaveInfo.product_code:  %s\n", oDebugStrg);
			sprintf(oDebugStrg, "%08X", oSlaveInfo.serial_number);
            wmLog(eDebug, "oSlaveInfo.serial_number: %s\n", oDebugStrg);
			sprintf(oDebugStrg, "%s", oSlaveInfo.name);
            wmLog(eDebug, "oSlaveInfo.name:          %s\n", oDebugStrg);

			wm_slave_data_t oSlaveData;
			oSlaveData.m_oAlias = oSlaveInfo.alias;
			oSlaveData.m_oPosition = oSlaveInfo.position;
			oSlaveData.m_oVendorId = oSlaveInfo.vendor_id;
			oSlaveData.m_oProductCode = oSlaveInfo.product_code;

		    ec_slave_config_t *pSlave_Config = NULL;
		    pSlave_Config = ecrt_master_slave_config(m_pEtherCATMaster,
		    		oSlaveInfo.alias, oSlaveInfo.position, oSlaveInfo.vendor_id, oSlaveInfo.product_code);
            sprintf(oDebugStrg, "0x%08lX. (%d,%d,0x%08X,0x%08X)", (unsigned long)pSlave_Config,
                    oSlaveInfo.alias, oSlaveInfo.position, oSlaveInfo.vendor_id, oSlaveInfo.product_code);
            wmLog(eDebug, "Return of ecrt_master_slave_config: %s\n", oDebugStrg);
		    if (!pSlave_Config)
		    {
		        sprintf(oDebugStrg, "(%d,%d,0x%08X,0x%08X)",
		        		oSlaveInfo.alias, oSlaveInfo.position, oSlaveInfo.vendor_id, oSlaveInfo.product_code);
                wmLogTr(eError, "QnxMsg.VI.ECMSlaveConfFail", "Failed to get slave configuration: %s\n", oDebugStrg);
		        return -1;
		    }
		    oSlaveData.m_pSlaveConfig = pSlave_Config;

		    ec_sync_info_t *syncs = NULL;
		    if ((oSlaveInfo.vendor_id == VENDORID_BECKHOFF) && (oSlaveInfo.product_code == PRODUCTCODE_EL1018))
		    {
		    	m_oSlaveNumbers.m_oNbr_EL1018++;
				syncs = EL1018_syncs;

				ec_pdo_entry_reg_t oPdoEntry;
				oPdoEntry.alias = oSlaveInfo.alias;
				oPdoEntry.position = oSlaveInfo.position;
				oPdoEntry.vendor_id = oSlaveInfo.vendor_id;
				oPdoEntry.product_code = oSlaveInfo.product_code;
				oPdoEntry.index = 0x6000;
				oPdoEntry.subindex = 1;
				oPdoEntry.offset = &EL1018_Input_Offset[m_oSlaveNumbers.m_oNbr_EL1018 - 1];
				oPdoEntry.bit_position = NULL;
			    m_oPdoEntryRegistration.push_back(oPdoEntry);

                // create SlaveInfo for Service-IO-Interface
                EC_T_GET_SLAVE_INFO oLocalSlaveInfo;
                sprintf(oLocalSlaveInfo.abyDeviceName, "EL1018 (%d)", m_oSlaveNumbers.m_oNbr_EL1018);
                oLocalSlaveInfo.wCfgPhyAddress = 1001 + oSlaveInfo.position;
                oLocalSlaveInfo.dwVendorId    = oSlaveInfo.vendor_id;
                oLocalSlaveInfo.dwProductCode = oSlaveInfo.product_code;
                oLocalSlaveInfo.dwPdOffsIn    = EL1018_Input_Offset[m_oSlaveNumbers.m_oNbr_EL1018 - 1] * 8; // Wert ist Bit-Position
                oLocalSlaveInfo.dwPdSizeIn    = 8;
                oLocalSlaveInfo.dwPdOffsOut   = -1;
                oLocalSlaveInfo.dwPdSizeOut   = 0;
                m_oSlaveInfo4Service.push_back(oLocalSlaveInfo);
		    }
		    else if ((oSlaveInfo.vendor_id == VENDORID_BECKHOFF) && (oSlaveInfo.product_code == PRODUCTCODE_EL2008))
		    {
		    	m_oSlaveNumbers.m_oNbr_EL2008++;
		    	syncs = EL2008_syncs;

				ec_pdo_entry_reg_t oPdoEntry;
				oPdoEntry.alias = oSlaveInfo.alias;
				oPdoEntry.position = oSlaveInfo.position;
				oPdoEntry.vendor_id = oSlaveInfo.vendor_id;
				oPdoEntry.product_code = oSlaveInfo.product_code;
				oPdoEntry.index = 0x7000;
				oPdoEntry.subindex = 1;
				oPdoEntry.offset = &EL2008_Output_Offset[m_oSlaveNumbers.m_oNbr_EL2008 - 1];
				oPdoEntry.bit_position = NULL;
			    m_oPdoEntryRegistration.push_back(oPdoEntry);

                // create SlaveInfo for Service-IO-Interface
                EC_T_GET_SLAVE_INFO oLocalSlaveInfo;
                sprintf(oLocalSlaveInfo.abyDeviceName, "EL2008 (%d)", m_oSlaveNumbers.m_oNbr_EL2008);
                oLocalSlaveInfo.wCfgPhyAddress = 1001 + oSlaveInfo.position;
                oLocalSlaveInfo.dwVendorId    = oSlaveInfo.vendor_id;
                oLocalSlaveInfo.dwProductCode = oSlaveInfo.product_code;
                oLocalSlaveInfo.dwPdOffsIn    = -1;
                oLocalSlaveInfo.dwPdSizeIn    = 0;
                oLocalSlaveInfo.dwPdOffsOut   = EL2008_Output_Offset[m_oSlaveNumbers.m_oNbr_EL2008 - 1] * 8; // Wert ist Bit-Position;
                oLocalSlaveInfo.dwPdSizeOut   = 8;
                m_oSlaveInfo4Service.push_back(oLocalSlaveInfo);
		    }
		    else if ((oSlaveInfo.vendor_id == VENDORID_BECKHOFF) && (oSlaveInfo.product_code == PRODUCTCODE_EL3102))
		    {
		    	m_oSlaveNumbers.m_oNbr_EL3102++;
		    	syncs = EL3102_syncs;

				ec_pdo_entry_reg_t oPdoEntry;
				oPdoEntry.alias = oSlaveInfo.alias;
				oPdoEntry.position = oSlaveInfo.position;
				oPdoEntry.vendor_id = oSlaveInfo.vendor_id;
				oPdoEntry.product_code = oSlaveInfo.product_code;
				oPdoEntry.index = 0x3101;
				oPdoEntry.subindex = 1;
				oPdoEntry.offset = &EL3102_Input_Offset[m_oSlaveNumbers.m_oNbr_EL3102 - 1];
				oPdoEntry.bit_position = NULL;
			    m_oPdoEntryRegistration.push_back(oPdoEntry);

                // create SlaveInfo for Service-IO-Interface
                EC_T_GET_SLAVE_INFO oLocalSlaveInfo;
                sprintf(oLocalSlaveInfo.abyDeviceName, "EL3102 (%d)", m_oSlaveNumbers.m_oNbr_EL3102);
                oLocalSlaveInfo.wCfgPhyAddress = 1001 + oSlaveInfo.position;
                oLocalSlaveInfo.dwVendorId    = oSlaveInfo.vendor_id;
                oLocalSlaveInfo.dwProductCode = oSlaveInfo.product_code;
                oLocalSlaveInfo.dwPdOffsIn    = EL3102_Input_Offset[m_oSlaveNumbers.m_oNbr_EL3102 - 1] * 8; // Wert ist Bit-Position
                oLocalSlaveInfo.dwPdSizeIn    = 48;
                oLocalSlaveInfo.dwPdOffsOut   = -1;
                oLocalSlaveInfo.dwPdSizeOut   = 0;
                m_oSlaveInfo4Service.push_back(oLocalSlaveInfo);
            }
            else if ((oSlaveInfo.vendor_id == VENDORID_BECKHOFF) && (oSlaveInfo.product_code == PRODUCTCODE_EL3702))
            {
                m_oSlaveNumbers.m_oNbr_EL3702++;

                // build up PDO structures for Beckhoff terminal EL3702
                EL3702_pdo_entries_input_chan1[0].index = 0x6800;
                EL3702_pdo_entries_input_chan1[0].subindex = 1;
                EL3702_pdo_entries_input_chan1[0].bit_length = 16;
                for(int i = 1;i <= EL3702_OVERSAMP_FACTOR;i++)
                {
                    EL3702_pdo_entries_input_chan1[i].index = 0x6000 + ((i - 1) * 0x0010);
                    EL3702_pdo_entries_input_chan1[i].subindex = 1;
                    EL3702_pdo_entries_input_chan1[i].bit_length = 16;
                }

                EL3702_pdo_entries_input_chan2[0].index = 0x6800;
                EL3702_pdo_entries_input_chan2[0].subindex = 2;
                EL3702_pdo_entries_input_chan2[0].bit_length = 16;
                for(int i = 1;i <= EL3702_OVERSAMP_FACTOR;i++)
                {
                    EL3702_pdo_entries_input_chan2[i].index = 0x6000 + ((i - 1) * 0x0010);
                    EL3702_pdo_entries_input_chan2[i].subindex = 2;
                    EL3702_pdo_entries_input_chan2[i].bit_length = 16;
                }

                EL3702_pdo_entries_input_add[0].index = 0x1D09;
                EL3702_pdo_entries_input_add[0].subindex = 98;
                EL3702_pdo_entries_input_add[0].bit_length = 32;

                EL3702_pdos_chan1[0].index = 0x1B00;
                EL3702_pdos_chan1[0].n_entries = 1;
                EL3702_pdos_chan1[0].entries = &EL3702_pdo_entries_input_chan1[0];
                for(int i = 1;i <= EL3702_OVERSAMP_FACTOR;i++)
                {
                    EL3702_pdos_chan1[i].index = 0x1A00 + ((i - 1) * 0x0001);
                    EL3702_pdos_chan1[i].n_entries = 1;
                    EL3702_pdos_chan1[i].entries = &EL3702_pdo_entries_input_chan1[i];
                }

                EL3702_pdos_chan2[0].index = 0x1B01;
                EL3702_pdos_chan2[0].n_entries = 1;
                EL3702_pdos_chan2[0].entries = &EL3702_pdo_entries_input_chan2[0];
                for(int i = 1;i <= EL3702_OVERSAMP_FACTOR;i++)
                {
                    EL3702_pdos_chan2[i].index = 0x1A80 + ((i - 1) * 0x0001);
                    EL3702_pdos_chan2[i].n_entries = 1;
                    EL3702_pdos_chan2[i].entries = &EL3702_pdo_entries_input_chan2[i];
                }

                EL3702_pdos_add[0].index = 0x1B10;
                EL3702_pdos_add[0].n_entries = 1;
                EL3702_pdos_add[0].entries = &EL3702_pdo_entries_input_add[0];

                EL3702_syncs[0].index = 0;
                EL3702_syncs[0].dir = EC_DIR_INPUT;
                EL3702_syncs[0].n_pdos = EL3702_OVERSAMP_FACTOR + 1;
                EL3702_syncs[0].pdos = &EL3702_pdos_chan1[0];
                EL3702_syncs[0].watchdog_mode = EC_WD_DISABLE;
                EL3702_syncs[1].index = 1;
                EL3702_syncs[1].dir = EC_DIR_INPUT;
                EL3702_syncs[1].n_pdos = EL3702_OVERSAMP_FACTOR + 1;
                EL3702_syncs[1].pdos = &EL3702_pdos_chan2[0];
                EL3702_syncs[1].watchdog_mode = EC_WD_DISABLE;
                EL3702_syncs[2].index = 2;
                EL3702_syncs[2].dir = EC_DIR_INPUT;
                EL3702_syncs[2].n_pdos = 1;
                EL3702_syncs[2].pdos = &EL3702_pdos_add[0];
                EL3702_syncs[2].watchdog_mode = EC_WD_DISABLE;
                EL3702_syncs[3].index = 0xff;

                syncs = EL3702_syncs;

                ec_pdo_entry_reg_t oPdoEntry;
                oPdoEntry.alias = oSlaveInfo.alias;
                oPdoEntry.position = oSlaveInfo.position;
                oPdoEntry.vendor_id = oSlaveInfo.vendor_id;
                oPdoEntry.product_code = oSlaveInfo.product_code;
                oPdoEntry.index = 0x6800;
                oPdoEntry.subindex = 1;
                oPdoEntry.offset = &EL3702_CH1_Input_Offset[m_oSlaveNumbers.m_oNbr_EL3702 - 1];
                oPdoEntry.bit_position = NULL;
                m_oPdoEntryRegistration.push_back(oPdoEntry);
                oPdoEntry.index = 0x6800;
                oPdoEntry.subindex = 2;
                oPdoEntry.offset = &EL3702_CH2_Input_Offset[m_oSlaveNumbers.m_oNbr_EL3702 - 1];
                m_oPdoEntryRegistration.push_back(oPdoEntry);
                oPdoEntry.index = 0x1D09;
                oPdoEntry.subindex = 98;
                oPdoEntry.offset = &EL3702_Add_Input_Offset[m_oSlaveNumbers.m_oNbr_EL3702 - 1];
                m_oPdoEntryRegistration.push_back(oPdoEntry);

                // create SlaveInfo for Service-IO-Interface
                EC_T_GET_SLAVE_INFO oLocalSlaveInfo;
                sprintf(oLocalSlaveInfo.abyDeviceName, "EL3702 (%d)", m_oSlaveNumbers.m_oNbr_EL3702);
                oLocalSlaveInfo.wCfgPhyAddress = 1001 + oSlaveInfo.position;
                oLocalSlaveInfo.dwVendorId    = oSlaveInfo.vendor_id;
                oLocalSlaveInfo.dwProductCode = oSlaveInfo.product_code;
                oLocalSlaveInfo.dwPdOffsIn    = EL3702_CH1_Input_Offset[m_oSlaveNumbers.m_oNbr_EL3702 - 1] * 8; // Wert ist Bit-Position
                oLocalSlaveInfo.dwPdSizeIn    = (((EL3702_OVERSAMP_FACTOR + 1) * 2 * 2) + 4) * 8; // ((EL3702_OVERSAMP_FACTOR + CycleCount-Var.) * 2 byte/value * 2 channels) + NextSync(32Bit)
                oLocalSlaveInfo.dwPdOffsOut   = -1;
                oLocalSlaveInfo.dwPdSizeOut   = 0;
                m_oSlaveInfo4Service.push_back(oLocalSlaveInfo);
            }
		    else if ((oSlaveInfo.vendor_id == VENDORID_BECKHOFF) && (oSlaveInfo.product_code == PRODUCTCODE_EL4102))
		    {
		    	m_oSlaveNumbers.m_oNbr_EL4102++;
		    	syncs = EL4102_syncs;

				ec_pdo_entry_reg_t oPdoEntry;
				oPdoEntry.alias = oSlaveInfo.alias;
				oPdoEntry.position = oSlaveInfo.position;
				oPdoEntry.vendor_id = oSlaveInfo.vendor_id;
				oPdoEntry.product_code = oSlaveInfo.product_code;
				oPdoEntry.index = 0x3001;
				oPdoEntry.subindex = 1;
				oPdoEntry.offset = &EL4102_Output_Offset[m_oSlaveNumbers.m_oNbr_EL4102 - 1];
				oPdoEntry.bit_position = NULL;
			    m_oPdoEntryRegistration.push_back(oPdoEntry);

                // create SlaveInfo for Service-IO-Interface
                EC_T_GET_SLAVE_INFO oLocalSlaveInfo;
                sprintf(oLocalSlaveInfo.abyDeviceName, "EL4102 (%d)", m_oSlaveNumbers.m_oNbr_EL4102);
                oLocalSlaveInfo.wCfgPhyAddress = 1001 + oSlaveInfo.position;
                oLocalSlaveInfo.dwVendorId    = oSlaveInfo.vendor_id;
                oLocalSlaveInfo.dwProductCode = oSlaveInfo.product_code;
                oLocalSlaveInfo.dwPdOffsIn    = -1;
                oLocalSlaveInfo.dwPdSizeIn    = 0;
                oLocalSlaveInfo.dwPdOffsOut   = EL4102_Output_Offset[m_oSlaveNumbers.m_oNbr_EL4102 - 1] * 8; // Wert ist Bit-Position;
                oLocalSlaveInfo.dwPdSizeOut   = 32;
                m_oSlaveInfo4Service.push_back(oLocalSlaveInfo);
		    }
		    else if ((oSlaveInfo.vendor_id == VENDORID_BECKHOFF) && (oSlaveInfo.product_code == PRODUCTCODE_EL4132))
		    {
		    	m_oSlaveNumbers.m_oNbr_EL4132++;
		    	syncs = EL4132_syncs;

				ec_pdo_entry_reg_t oPdoEntry;
				oPdoEntry.alias = oSlaveInfo.alias;
				oPdoEntry.position = oSlaveInfo.position;
				oPdoEntry.vendor_id = oSlaveInfo.vendor_id;
				oPdoEntry.product_code = oSlaveInfo.product_code;
				oPdoEntry.index = 0x3001;
				oPdoEntry.subindex = 1;
				oPdoEntry.offset = &EL4132_Output_Offset[m_oSlaveNumbers.m_oNbr_EL4132 - 1];
				oPdoEntry.bit_position = NULL;
			    m_oPdoEntryRegistration.push_back(oPdoEntry);

                // create SlaveInfo for Service-IO-Interface
                EC_T_GET_SLAVE_INFO oLocalSlaveInfo;
                sprintf(oLocalSlaveInfo.abyDeviceName, "EL4132 (%d)", m_oSlaveNumbers.m_oNbr_EL4132);
                oLocalSlaveInfo.wCfgPhyAddress = 1001 + oSlaveInfo.position;
                oLocalSlaveInfo.dwVendorId    = oSlaveInfo.vendor_id;
                oLocalSlaveInfo.dwProductCode = oSlaveInfo.product_code;
                oLocalSlaveInfo.dwPdOffsIn    = -1;
                oLocalSlaveInfo.dwPdSizeIn    = 0;
                oLocalSlaveInfo.dwPdOffsOut   = EL4132_Output_Offset[m_oSlaveNumbers.m_oNbr_EL4132 - 1] * 8; // Wert ist Bit-Position;
                oLocalSlaveInfo.dwPdSizeOut   = 32;
                m_oSlaveInfo4Service.push_back(oLocalSlaveInfo);
		    }
		    else if ((oSlaveInfo.vendor_id == VENDORID_BECKHOFF) && (oSlaveInfo.product_code == PRODUCTCODE_EL5101))
		    {
		    	m_oSlaveNumbers.m_oNbr_EL5101++;
		    	syncs = EL5101_syncs;

				ec_pdo_entry_reg_t oPdoEntry;
				oPdoEntry.alias = oSlaveInfo.alias;
				oPdoEntry.position = oSlaveInfo.position;
				oPdoEntry.vendor_id = oSlaveInfo.vendor_id;
				oPdoEntry.product_code = oSlaveInfo.product_code;
				oPdoEntry.index = 0x6010;
				oPdoEntry.subindex = 1;
				oPdoEntry.offset = &EL5101_Input_Offset[m_oSlaveNumbers.m_oNbr_EL5101 - 1];
				oPdoEntry.bit_position = NULL;
			    m_oPdoEntryRegistration.push_back(oPdoEntry);
				oPdoEntry.index = 0x7010;
				oPdoEntry.subindex = 1;
				oPdoEntry.offset = &EL5101_Output_Offset[m_oSlaveNumbers.m_oNbr_EL5101 - 1];
			    m_oPdoEntryRegistration.push_back(oPdoEntry);

                // create SlaveInfo for Service-IO-Interface
                EC_T_GET_SLAVE_INFO oLocalSlaveInfo;
                sprintf(oLocalSlaveInfo.abyDeviceName, "EL5101 (%d)", m_oSlaveNumbers.m_oNbr_EL5101);
                oLocalSlaveInfo.wCfgPhyAddress = 1001 + oSlaveInfo.position;
                oLocalSlaveInfo.dwVendorId    = oSlaveInfo.vendor_id;
                oLocalSlaveInfo.dwProductCode = oSlaveInfo.product_code;
                oLocalSlaveInfo.dwPdOffsIn    = EL5101_Input_Offset[m_oSlaveNumbers.m_oNbr_EL5101 - 1] * 8; // Wert ist Bit-Position;
                oLocalSlaveInfo.dwPdSizeIn    = 80;
                oLocalSlaveInfo.dwPdOffsOut   = EL5101_Output_Offset[m_oSlaveNumbers.m_oNbr_EL5101 - 1] * 8; // Wert ist Bit-Position;
                oLocalSlaveInfo.dwPdSizeOut   = 48;
                m_oSlaveInfo4Service.push_back(oLocalSlaveInfo);
		    }
            else if ((oSlaveInfo.vendor_id == VENDORID_BECKHOFF) && (oSlaveInfo.product_code == PRODUCTCODE_EL5151))
            {
                m_oSlaveNumbers.m_numberOfEL5151++;
                syncs = EL5151Syncs;

                ec_pdo_entry_reg_t pdoEntry;
                pdoEntry.alias = oSlaveInfo.alias;
                pdoEntry.position = oSlaveInfo.position;
                pdoEntry.vendor_id = oSlaveInfo.vendor_id;
                pdoEntry.product_code = oSlaveInfo.product_code;
                pdoEntry.index = 0x6000;
                pdoEntry.subindex = 1;
                pdoEntry.offset = &EL5151InputOffset[m_oSlaveNumbers.m_numberOfEL5151 - 1];
                pdoEntry.bit_position = nullptr;
                m_oPdoEntryRegistration.push_back(pdoEntry);
                pdoEntry.index = 0x7000;
                pdoEntry.subindex = 1;
                pdoEntry.offset = &EL5151OutputOffset[m_oSlaveNumbers.m_numberOfEL5151 - 1];
                m_oPdoEntryRegistration.push_back(pdoEntry);

                // create SlaveInfo for Service-IO-Interface
                EC_T_GET_SLAVE_INFO localSlaveInfo;
                sprintf(localSlaveInfo.abyDeviceName, "EL5151 (%d)", m_oSlaveNumbers.m_numberOfEL5151);
                localSlaveInfo.wCfgPhyAddress = 1001 + oSlaveInfo.position;
                localSlaveInfo.dwVendorId    = oSlaveInfo.vendor_id;
                localSlaveInfo.dwProductCode = oSlaveInfo.product_code;
                localSlaveInfo.dwPdOffsIn    = EL5151InputOffset[m_oSlaveNumbers.m_numberOfEL5151 - 1] * 8; // position is a Bit value
                localSlaveInfo.dwPdSizeIn    = 80;
                localSlaveInfo.dwPdOffsOut   = EL5151OutputOffset[m_oSlaveNumbers.m_numberOfEL5151 - 1] * 8; // position is a Bit value
                localSlaveInfo.dwPdSizeOut   = 48;
                m_oSlaveInfo4Service.push_back(localSlaveInfo);
            }
		    else if ((oSlaveInfo.vendor_id == VENDORID_KUNBUS) && (oSlaveInfo.product_code == PRODUCTCODE_KUNBUS_GW))
		    {
		    	m_oSlaveNumbers.m_oNbr_GATEWAY++;
		    	syncs = KUNBUS_syncs;

				ec_pdo_entry_reg_t oPdoEntry;
				oPdoEntry.alias = oSlaveInfo.alias;
				oPdoEntry.position = oSlaveInfo.position;
				oPdoEntry.vendor_id = oSlaveInfo.vendor_id;
				oPdoEntry.product_code = oSlaveInfo.product_code;
				oPdoEntry.index = 0x2000;
				oPdoEntry.subindex = 1;
				oPdoEntry.offset = &GATEWAY_Input_Offset[m_oSlaveNumbers.m_oNbr_GATEWAY - 1];
				oPdoEntry.bit_position = NULL;
			    m_oPdoEntryRegistration.push_back(oPdoEntry);
				oPdoEntry.index = 0x2100;
				oPdoEntry.subindex = 1;
				oPdoEntry.offset = &GATEWAY_Output_Offset[m_oSlaveNumbers.m_oNbr_GATEWAY - 1];
			    m_oPdoEntryRegistration.push_back(oPdoEntry);

                if (!m_oFieldbusViaSeparateFieldbusBoard)
                {
                    // create SlaveInfo for Service-IO-Interface
                    EC_T_GET_SLAVE_INFO oLocalSlaveInfo;
                    sprintf(oLocalSlaveInfo.abyDeviceName, "GATEWAY (%d)", m_oSlaveNumbers.m_oNbr_GATEWAY);
                    oLocalSlaveInfo.wCfgPhyAddress = 1001 + oSlaveInfo.position;
                    // change VendorID and Product Code to Anybus Gateway
                    // This lets the Windows Part believes, that there is an Anybus Gateway
                    oLocalSlaveInfo.dwVendorId    = VENDORID_HMS;
                    oLocalSlaveInfo.dwProductCode = PRODUCTCODE_ANYBUS_GW;
                    oLocalSlaveInfo.dwPdOffsIn    = GATEWAY_Input_Offset[m_oSlaveNumbers.m_oNbr_GATEWAY - 1] * 8; // Wert ist Bit-Position;
                    oLocalSlaveInfo.dwPdSizeIn    = GATEWAY1_DATA_LENGHT * 8;
                    oLocalSlaveInfo.dwPdOffsOut   = GATEWAY_Output_Offset[m_oSlaveNumbers.m_oNbr_GATEWAY - 1] * 8; // Wert ist Bit-Position;
                    oLocalSlaveInfo.dwPdSizeOut   = GATEWAY1_DATA_LENGHT * 8;
                    m_oSlaveInfo4Service.push_back(oLocalSlaveInfo);
                }
		    }
            else if (((oSlaveInfo.vendor_id == VENDORID_HMS) && (oSlaveInfo.product_code == PRODUCTCODE_ANYBUS_GW)) ||
                     ((oSlaveInfo.vendor_id == VENDORID_HMS) && (oSlaveInfo.product_code == PRODUCTCODE_COMMUNICATOR_GW)))
		    {
		    	m_oSlaveNumbers.m_oNbr_GATEWAY++;
		    	syncs = ANYBUS_syncs;

				ec_pdo_entry_reg_t oPdoEntry;
				oPdoEntry.alias = oSlaveInfo.alias;
				oPdoEntry.position = oSlaveInfo.position;
				oPdoEntry.vendor_id = oSlaveInfo.vendor_id;
				oPdoEntry.product_code = oSlaveInfo.product_code;
				oPdoEntry.index = 0x2000;
				oPdoEntry.subindex = 1;
				oPdoEntry.offset = &GATEWAY_Input_Offset[m_oSlaveNumbers.m_oNbr_GATEWAY - 1];
				oPdoEntry.bit_position = NULL;
			    m_oPdoEntryRegistration.push_back(oPdoEntry);
				oPdoEntry.index = 0x2100;
				oPdoEntry.subindex = 1;
				oPdoEntry.offset = &GATEWAY_Output_Offset[m_oSlaveNumbers.m_oNbr_GATEWAY - 1];
			    m_oPdoEntryRegistration.push_back(oPdoEntry);

                if (!m_oFieldbusViaSeparateFieldbusBoard)
                {
                    // create SlaveInfo for Service-IO-Interface
                    EC_T_GET_SLAVE_INFO oLocalSlaveInfo;
                    sprintf(oLocalSlaveInfo.abyDeviceName, "GATEWAY (%d)", m_oSlaveNumbers.m_oNbr_GATEWAY);
                    oLocalSlaveInfo.wCfgPhyAddress = 1001 + oSlaveInfo.position;
                    oLocalSlaveInfo.dwVendorId    = VENDORID_HMS;
                    oLocalSlaveInfo.dwProductCode = PRODUCTCODE_ANYBUS_GW;
                    oLocalSlaveInfo.dwPdOffsIn    = GATEWAY_Input_Offset[m_oSlaveNumbers.m_oNbr_GATEWAY - 1] * 8; // Wert ist Bit-Position;
                    oLocalSlaveInfo.dwPdSizeIn    = GATEWAY1_DATA_LENGHT * 8;
                    oLocalSlaveInfo.dwPdOffsOut   = GATEWAY_Output_Offset[m_oSlaveNumbers.m_oNbr_GATEWAY - 1] * 8; // Wert ist Bit-Position;
                    oLocalSlaveInfo.dwPdSizeOut   = GATEWAY1_DATA_LENGHT * 8;
                    m_oSlaveInfo4Service.push_back(oLocalSlaveInfo);
                } 
		    }
		    else if ((oSlaveInfo.vendor_id == VENDORID_COPLEY) && (oSlaveInfo.product_code == PRODUCTCODE_ACCELNET))
		    {
		    	m_oSlaveNumbers.m_oNbr_ACCELNET++;
		    	syncs = ACCELNET_syncs;

				ec_pdo_entry_reg_t oPdoEntry;
				oPdoEntry.alias = oSlaveInfo.alias;
				oPdoEntry.position = oSlaveInfo.position;
				oPdoEntry.vendor_id = oSlaveInfo.vendor_id;
				oPdoEntry.product_code = oSlaveInfo.product_code;
				oPdoEntry.index = 0x6041;
				oPdoEntry.subindex = 0;
				oPdoEntry.offset = &ACCELNET_Input_Offset[m_oSlaveNumbers.m_oNbr_ACCELNET - 1];
				oPdoEntry.bit_position = NULL;
			    m_oPdoEntryRegistration.push_back(oPdoEntry);
				oPdoEntry.index = 0x6040;
				oPdoEntry.subindex = 0;
				oPdoEntry.offset = &ACCELNET_Output_Offset[m_oSlaveNumbers.m_oNbr_ACCELNET - 1];
			    m_oPdoEntryRegistration.push_back(oPdoEntry);

                // create SlaveInfo for Service-IO-Interface
                EC_T_GET_SLAVE_INFO oLocalSlaveInfo;
                sprintf(oLocalSlaveInfo.abyDeviceName, "ACCELNET (%d)", m_oSlaveNumbers.m_oNbr_ACCELNET);
                oLocalSlaveInfo.wCfgPhyAddress = 1001 + oSlaveInfo.position;
                oLocalSlaveInfo.dwVendorId    = oSlaveInfo.vendor_id;
                oLocalSlaveInfo.dwProductCode = oSlaveInfo.product_code;
                oLocalSlaveInfo.dwPdOffsIn    = ACCELNET_Input_Offset[m_oSlaveNumbers.m_oNbr_ACCELNET - 1] * 8; // Wert ist Bit-Position;
                oLocalSlaveInfo.dwPdSizeIn    = 144;
                oLocalSlaveInfo.dwPdOffsOut   = ACCELNET_Output_Offset[m_oSlaveNumbers.m_oNbr_ACCELNET - 1] * 8; // Wert ist Bit-Position;
                oLocalSlaveInfo.dwPdSizeOut   = 256;
                m_oSlaveInfo4Service.push_back(oLocalSlaveInfo);
		    }
            else if ((oSlaveInfo.vendor_id == VENDORID_MAXON) && (oSlaveInfo.product_code == PRODUCTCODE_EPOS4))
            {
                m_oSlaveNumbers.m_oNbr_EPOS4++;
                syncs = EPOS4_syncs;

                ec_pdo_entry_reg_t oPdoEntry;
                oPdoEntry.alias = oSlaveInfo.alias;
                oPdoEntry.position = oSlaveInfo.position;
                oPdoEntry.vendor_id = oSlaveInfo.vendor_id;
                oPdoEntry.product_code = oSlaveInfo.product_code;
                oPdoEntry.index = 0x6041;
                oPdoEntry.subindex = 0;
                oPdoEntry.offset = &EPOS4_Input_Offset[m_oSlaveNumbers.m_oNbr_EPOS4 - 1];
                oPdoEntry.bit_position = NULL;
                m_oPdoEntryRegistration.push_back(oPdoEntry);
                oPdoEntry.index = 0x6040;
                oPdoEntry.subindex = 0;
                oPdoEntry.offset = &EPOS4_Output_Offset[m_oSlaveNumbers.m_oNbr_EPOS4 - 1];
                m_oPdoEntryRegistration.push_back(oPdoEntry);

                // save slave config pointer for SDO communication
                m_pEPOS4SlaveConfig[m_oSlaveNumbers.m_oNbr_EPOS4 - 1] = pSlave_Config;
                // save slave position for SDO communication
                m_oEPOS4SlavePosition[m_oSlaveNumbers.m_oNbr_EPOS4 - 1] = oSlaveInfo.position;

                // create SlaveInfo for Service-IO-Interface
                EC_T_GET_SLAVE_INFO oLocalSlaveInfo;
                sprintf(oLocalSlaveInfo.abyDeviceName, "EPOS4 (%d)", m_oSlaveNumbers.m_oNbr_EPOS4);
                oLocalSlaveInfo.wCfgPhyAddress = 1001 + oSlaveInfo.position;
                oLocalSlaveInfo.dwVendorId    = oSlaveInfo.vendor_id;
                oLocalSlaveInfo.dwProductCode = oSlaveInfo.product_code;
                oLocalSlaveInfo.dwPdOffsIn    = EPOS4_Input_Offset[m_oSlaveNumbers.m_oNbr_EPOS4 - 1] * 8; // Wert ist Bit-Position;
                oLocalSlaveInfo.dwPdSizeIn    = 152;
                oLocalSlaveInfo.dwPdOffsOut   = EPOS4_Output_Offset[m_oSlaveNumbers.m_oNbr_EPOS4 - 1] * 8; // Wert ist Bit-Position;
                oLocalSlaveInfo.dwPdSizeOut   = 224;
                m_oSlaveInfo4Service.push_back(oLocalSlaveInfo);
            }
		    else if ((oSlaveInfo.vendor_id == VENDORID_BECKHOFF) && (oSlaveInfo.product_code == PRODUCTCODE_EK1100))
		    {
		    	m_oSlaveNumbers.m_oNbr_EK1100++;
		    	// keine PDOs fuer Koppler EK1100
		    	syncs = nullptr;

                // create SlaveInfo for Service-IO-Interface
                EC_T_GET_SLAVE_INFO oLocalSlaveInfo;
                sprintf(oLocalSlaveInfo.abyDeviceName, "EK1100 (%d)", m_oSlaveNumbers.m_oNbr_EK1100);
                oLocalSlaveInfo.wCfgPhyAddress = 1001 + oSlaveInfo.position;
                oLocalSlaveInfo.dwVendorId    = oSlaveInfo.vendor_id;
                oLocalSlaveInfo.dwProductCode = oSlaveInfo.product_code;
                oLocalSlaveInfo.dwPdOffsIn    = -1;
                oLocalSlaveInfo.dwPdSizeIn    = 0;
                oLocalSlaveInfo.dwPdOffsOut   = -1;
                oLocalSlaveInfo.dwPdSizeOut   = 0;
                m_oSlaveInfo4Service.push_back(oLocalSlaveInfo);
            }
		    else if ((oSlaveInfo.vendor_id == VENDORID_BECKHOFF) && (oSlaveInfo.product_code == PRODUCTCODE_EK1310))
		    {
		    	m_oSlaveNumbers.m_oNbr_EK1310++;
		    	syncs = EK1310_syncs;

				ec_pdo_entry_reg_t oPdoEntry;
				oPdoEntry.alias = oSlaveInfo.alias;
				oPdoEntry.position = oSlaveInfo.position;
				oPdoEntry.vendor_id = oSlaveInfo.vendor_id;
				oPdoEntry.product_code = oSlaveInfo.product_code;
				oPdoEntry.index = 0x6000;
				oPdoEntry.subindex = 1;
				oPdoEntry.offset = &EK1310_Input_Offset[m_oSlaveNumbers.m_oNbr_EK1310 - 1];
				oPdoEntry.bit_position = NULL;
			    m_oPdoEntryRegistration.push_back(oPdoEntry);

                // create SlaveInfo for Service-IO-Interface
                EC_T_GET_SLAVE_INFO oLocalSlaveInfo;
                sprintf(oLocalSlaveInfo.abyDeviceName, "EK1310 (%d)", m_oSlaveNumbers.m_oNbr_EK1310);
                oLocalSlaveInfo.wCfgPhyAddress = 1001 + oSlaveInfo.position;
                oLocalSlaveInfo.dwVendorId    = oSlaveInfo.vendor_id;
                oLocalSlaveInfo.dwProductCode = oSlaveInfo.product_code;
                oLocalSlaveInfo.dwPdOffsIn    = EK1310_Input_Offset[m_oSlaveNumbers.m_oNbr_EK1310 - 1] * 8; // Wert ist Bit-Position;
                oLocalSlaveInfo.dwPdSizeIn    = 8;
                oLocalSlaveInfo.dwPdOffsOut   = -1;
                oLocalSlaveInfo.dwPdSizeOut   = 0;
                m_oSlaveInfo4Service.push_back(oLocalSlaveInfo);
            }
            else if ((oSlaveInfo.vendor_id == VENDORID_PRECITEC) && (oSlaveInfo.product_code == PRODUCTCODE_FRONTEND))
            {
                m_oSlaveNumbers.m_oNbr_FRONTEND++;

                // only valid for FRONTEND_OVERSAMP_FACTOR 50 !
                // build up PDO structures for FRONTEND device (LWM40)
                // Outputs to FRONTEND 
                FRONTEND_pdo_entries_output[0].index = 0x7000; // Amplifier CH1
                FRONTEND_pdo_entries_output[0].subindex = 1;
                FRONTEND_pdo_entries_output[0].bit_length = 16;
                FRONTEND_pdo_entries_output[1].index = 0x7000; // Amplifier CH2
                FRONTEND_pdo_entries_output[1].subindex = 2;
                FRONTEND_pdo_entries_output[1].bit_length = 16;
                FRONTEND_pdo_entries_output[2].index = 0x7000; // Amplifier CH3
                FRONTEND_pdo_entries_output[2].subindex = 3;
                FRONTEND_pdo_entries_output[2].bit_length = 16;
                FRONTEND_pdo_entries_output[3].index = 0x7000; // Amplifier CH4
                FRONTEND_pdo_entries_output[3].subindex = 4;
                FRONTEND_pdo_entries_output[3].bit_length = 16;
                FRONTEND_pdo_entries_output[4].index = 0x7100; // Oversampling factor
                FRONTEND_pdo_entries_output[4].subindex = 0;
                FRONTEND_pdo_entries_output[4].bit_length = 16;
                // Inputs from FRONTEND
                FRONTEND_pdo_entries_input[0].index = 0x6000; // error variable
                FRONTEND_pdo_entries_input[0].subindex = 0;
                FRONTEND_pdo_entries_input[0].bit_length = 16;
                for(int i = 1;i <= FRONTEND_OVERSAMP_FACTOR;i++) // channel 1
                {
                    FRONTEND_pdo_entries_input[i].index = 0x6200;
                    FRONTEND_pdo_entries_input[i].subindex = i;
                    FRONTEND_pdo_entries_input[i].bit_length = 16;
                }
                for(int i = 1;i <= FRONTEND_OVERSAMP_FACTOR;i++) // channel 2
                {
                    FRONTEND_pdo_entries_input[i + FRONTEND_OVERSAMP_FACTOR].index = 0x6300;
                    FRONTEND_pdo_entries_input[i + FRONTEND_OVERSAMP_FACTOR].subindex = i;
                    FRONTEND_pdo_entries_input[i + FRONTEND_OVERSAMP_FACTOR].bit_length = 16;
                }
                for(int i = 1;i <= FRONTEND_OVERSAMP_FACTOR;i++) // channel 3
                {
                    FRONTEND_pdo_entries_input[i + (FRONTEND_OVERSAMP_FACTOR * 2)].index = 0x6400;
                    FRONTEND_pdo_entries_input[i + (FRONTEND_OVERSAMP_FACTOR * 2)].subindex = i;
                    FRONTEND_pdo_entries_input[i + (FRONTEND_OVERSAMP_FACTOR * 2)].bit_length = 16;
                }
                for(int i = 1;i <= FRONTEND_OVERSAMP_FACTOR;i++) // channel 4
                {
                    FRONTEND_pdo_entries_input[i + (FRONTEND_OVERSAMP_FACTOR * 3)].index = 0x6500;
                    FRONTEND_pdo_entries_input[i + (FRONTEND_OVERSAMP_FACTOR * 3)].subindex = i;
                    FRONTEND_pdo_entries_input[i + (FRONTEND_OVERSAMP_FACTOR * 3)].bit_length = 16;
                }

                // Outputs to FRONTEND
                // pdos for outputs
                FRONTEND_pdos[0].index = 0x1600;
                FRONTEND_pdos[0].n_entries = 4;
                FRONTEND_pdos[0].entries = &FRONTEND_pdo_entries_output[0];
                FRONTEND_pdos[1].index = 0x1610;
                FRONTEND_pdos[1].n_entries = 1;
                FRONTEND_pdos[1].entries = &FRONTEND_pdo_entries_output[4];
                // Inputs from FRONTEND
                // pdo error variable
                FRONTEND_pdos[2].index = 0x1A00;
                FRONTEND_pdos[2].n_entries = 1;
                FRONTEND_pdos[2].entries = &FRONTEND_pdo_entries_input[0];
                // pdos channel 1
                FRONTEND_pdos[3].index = 0x1A01;
                FRONTEND_pdos[3].n_entries = 5;
                FRONTEND_pdos[3].entries = &FRONTEND_pdo_entries_input[1];
                FRONTEND_pdos[4].index = 0x1A02;
                FRONTEND_pdos[4].n_entries = 5;
                FRONTEND_pdos[4].entries = &FRONTEND_pdo_entries_input[6];
                FRONTEND_pdos[5].index = 0x1A03;
                FRONTEND_pdos[5].n_entries = 15;
                FRONTEND_pdos[5].entries = &FRONTEND_pdo_entries_input[11];
                FRONTEND_pdos[6].index = 0x1A04;
                FRONTEND_pdos[6].n_entries = 25;
                FRONTEND_pdos[6].entries = &FRONTEND_pdo_entries_input[26];
                // pdos channel 2
                FRONTEND_pdos[7].index = 0x1A11;
                FRONTEND_pdos[7].n_entries = 5;
                FRONTEND_pdos[7].entries = &FRONTEND_pdo_entries_input[51];
                FRONTEND_pdos[8].index = 0x1A12;
                FRONTEND_pdos[8].n_entries = 5;
                FRONTEND_pdos[8].entries = &FRONTEND_pdo_entries_input[56];
                FRONTEND_pdos[9].index = 0x1A13;
                FRONTEND_pdos[9].n_entries = 15;
                FRONTEND_pdos[9].entries = &FRONTEND_pdo_entries_input[61];
                FRONTEND_pdos[10].index = 0x1A14;
                FRONTEND_pdos[10].n_entries = 25;
                FRONTEND_pdos[10].entries = &FRONTEND_pdo_entries_input[76];
                // pdos channel 3
                FRONTEND_pdos[11].index = 0x1A21;
                FRONTEND_pdos[11].n_entries = 5;
                FRONTEND_pdos[11].entries = &FRONTEND_pdo_entries_input[101];
                FRONTEND_pdos[12].index = 0x1A22;
                FRONTEND_pdos[12].n_entries = 5;
                FRONTEND_pdos[12].entries = &FRONTEND_pdo_entries_input[106];
                FRONTEND_pdos[13].index = 0x1A23;
                FRONTEND_pdos[13].n_entries = 15;
                FRONTEND_pdos[13].entries = &FRONTEND_pdo_entries_input[111];
                FRONTEND_pdos[14].index = 0x1A24;
                FRONTEND_pdos[14].n_entries = 25;
                FRONTEND_pdos[14].entries = &FRONTEND_pdo_entries_input[126];
                // pdos channel 4
                FRONTEND_pdos[15].index = 0x1A31;
                FRONTEND_pdos[15].n_entries = 5;
                FRONTEND_pdos[15].entries = &FRONTEND_pdo_entries_input[151];
                FRONTEND_pdos[16].index = 0x1A32;
                FRONTEND_pdos[16].n_entries = 5;
                FRONTEND_pdos[16].entries = &FRONTEND_pdo_entries_input[156];
                FRONTEND_pdos[17].index = 0x1A33;
                FRONTEND_pdos[17].n_entries = 15;
                FRONTEND_pdos[17].entries = &FRONTEND_pdo_entries_input[161];
                FRONTEND_pdos[18].index = 0x1A34;
                FRONTEND_pdos[18].n_entries = 25;
                FRONTEND_pdos[18].entries = &FRONTEND_pdo_entries_input[176];

                // sync managers
                FRONTEND_syncs[0].index = 2; // sync manager for outputs
                FRONTEND_syncs[0].dir = EC_DIR_OUTPUT;
                FRONTEND_syncs[0].n_pdos = 2;
                FRONTEND_syncs[0].pdos = &FRONTEND_pdos[0];
                FRONTEND_syncs[0].watchdog_mode = EC_WD_ENABLE;
                FRONTEND_syncs[1].index = 3; // sync manager for inputs
                FRONTEND_syncs[1].dir = EC_DIR_INPUT;
                FRONTEND_syncs[1].n_pdos = 17;
                FRONTEND_syncs[1].pdos = &FRONTEND_pdos[2];
                FRONTEND_syncs[1].watchdog_mode = EC_WD_ENABLE;
                FRONTEND_syncs[2].index = 0xff;

                syncs = FRONTEND_syncs;

                ec_pdo_entry_reg_t oPdoEntry;
                oPdoEntry.alias = oSlaveInfo.alias;
                oPdoEntry.position = oSlaveInfo.position;
                oPdoEntry.vendor_id = oSlaveInfo.vendor_id;
                oPdoEntry.product_code = oSlaveInfo.product_code;
                oPdoEntry.index = 0x6000;
                oPdoEntry.subindex = 0;
                oPdoEntry.offset = &FRONTEND_Input_Offset[m_oSlaveNumbers.m_oNbr_FRONTEND - 1];
                oPdoEntry.bit_position = NULL;
                m_oPdoEntryRegistration.push_back(oPdoEntry);
                oPdoEntry.index = 0x7000;
                oPdoEntry.subindex = 1;
                oPdoEntry.offset = &FRONTEND_Output_Offset[m_oSlaveNumbers.m_oNbr_FRONTEND - 1];
                m_oPdoEntryRegistration.push_back(oPdoEntry);

                // create SlaveInfo for Service-IO-Interface
                EC_T_GET_SLAVE_INFO oLocalSlaveInfo;
                sprintf(oLocalSlaveInfo.abyDeviceName, "FRONTEND (%d)", m_oSlaveNumbers.m_oNbr_FRONTEND);
                oLocalSlaveInfo.wCfgPhyAddress = 1001 + oSlaveInfo.position;
                oLocalSlaveInfo.dwVendorId    = oSlaveInfo.vendor_id;
                oLocalSlaveInfo.dwProductCode = oSlaveInfo.product_code;
                oLocalSlaveInfo.dwPdOffsIn    = FRONTEND_Input_Offset[m_oSlaveNumbers.m_oNbr_FRONTEND - 1] * 8; // Wert ist Bit-Position;
                oLocalSlaveInfo.dwPdSizeIn    = 16 + (FRONTEND_OVERSAMP_FACTOR * 4 * 2 * 8); // errorVar + (factor * 4 channels * 2 byte * 8 bit)
                oLocalSlaveInfo.dwPdOffsOut   = FRONTEND_Output_Offset[m_oSlaveNumbers.m_oNbr_FRONTEND - 1] * 8; // Wert ist Bit-Position;
                oLocalSlaveInfo.dwPdSizeOut   = (5 * 2 * 8); // (var count * 2 byte * 8 bit)
                m_oSlaveInfo4Service.push_back(oLocalSlaveInfo);
            }
		    else
		    {
		    	sprintf(oDebugStrg, "(%d,%d,0x%08X,0x%08X)",
		    			oSlaveInfo.alias, oSlaveInfo.position, oSlaveInfo.vendor_id, oSlaveInfo.product_code);
                wmLogTr(eError, "QnxMsg.VI.ECMNoFittingSlave", "No fitting combination of VENDOR_ID and PRODUCT_CODE found ! %s\n", oDebugStrg);
		    }

		    if (syncs != nullptr)
		    {
		    	int retVal = ecrt_slave_config_pdos(pSlave_Config, EC_END, syncs);
                sprintf(oDebugStrg, "%d. (%d,%d,0x%08X,0x%08X)", retVal,
                        oSlaveInfo.alias, oSlaveInfo.position, oSlaveInfo.vendor_id, oSlaveInfo.product_code);
                wmLog(eDebug, "Return of ecrt_slave_config_pdos: %s\n", oDebugStrg);
		    	if (retVal != 0)
		    	{
		    		sprintf(oDebugStrg, "(%d,%d,0x%08X,0x%08X)",
		    				oSlaveInfo.alias, oSlaveInfo.position, oSlaveInfo.vendor_id, oSlaveInfo.product_code);
                    wmLogTr(eError, "QnxMsg.VI.ECMPDOConfFailed", "Cannot set PDO configuration for slave ! %s\n", oDebugStrg);
		    		return -1;
		    	}

                if ((oSlaveInfo.vendor_id == VENDORID_BECKHOFF) && (oSlaveInfo.product_code == PRODUCTCODE_EK1100))
                {
                    if (m_oSlaveNumbers.m_oNbr_EK1100 == 1)
                    {
                        g_pFirstEK1100Slave = pSlave_Config;
                    }
                }
                if ((oSlaveInfo.vendor_id == VENDORID_BECKHOFF) && (oSlaveInfo.product_code == PRODUCTCODE_EL1018))
                {
                    if (m_oSlaveNumbers.m_oNbr_EL1018 == 1)
                    {
                        g_pFirstEL1018Slave = pSlave_Config;
                    }
                }
                if ((oSlaveInfo.vendor_id == VENDORID_BECKHOFF) && (oSlaveInfo.product_code == PRODUCTCODE_EL2008))
                {
                    if (m_oSlaveNumbers.m_oNbr_EL2008 == 1)
                    {
                        g_pFirstEL2008Slave = pSlave_Config;
                    }
                }

		    	// configure slave EL3702 for Distributed Clocks
                if ((oSlaveInfo.vendor_id == VENDORID_BECKHOFF) && (oSlaveInfo.product_code == PRODUCTCODE_EL3702))
                {
                    if (m_oSlaveNumbers.m_oNbr_EL3702 == 1)
                    {
                        g_pFirstEL3702Slave = pSlave_Config;
                    }
                    ecrt_slave_config_dc(pSlave_Config, 0x0730, (CYCLE_TIME_NS / EL3702_OVERSAMP_FACTOR), -10000, CYCLE_TIME_NS, 0); // 1 ms cycle time
                }

                // configure slave FRONTEND for Distributed Clocks
                if ((oSlaveInfo.vendor_id == VENDORID_PRECITEC) && (oSlaveInfo.product_code == PRODUCTCODE_FRONTEND))
                {
                    if (m_oSlaveNumbers.m_oNbr_FRONTEND == 1)
                    {
                        g_pFirstFRONTENDSlave = pSlave_Config;
                    }
                    uint16_t oAssignActivate = 0;
                    if ((FRONTEND_OVERSAMP_FACTOR == 100) ||
                        (FRONTEND_OVERSAMP_FACTOR == 50) ||
                        (FRONTEND_OVERSAMP_FACTOR == 25))
                    {
                        oAssignActivate = 0x330;
                    }
                    else if (FRONTEND_OVERSAMP_FACTOR == 10)
                    {
                         oAssignActivate = 0x310;
                    }
                    else if (FRONTEND_OVERSAMP_FACTOR == 5)
                    {
                         oAssignActivate = 0x320;
                    }
                    ecrt_slave_config_dc(pSlave_Config, oAssignActivate, (CYCLE_TIME_NS / FRONTEND_OVERSAMP_FACTOR), -1100000, CYCLE_TIME_NS, 0); // 1 ms cycle time
                }

		    }
		    // Slave in Slave Verzeichnis von WM eintragen
		    m_oSlaveDirectory.push_back(oSlaveData);
		}
    }
    // leeren Eintrag als Endekennung fuer die Liste erzeugen
	ec_pdo_entry_reg_t oPdoEntry = {};
    m_oPdoEntryRegistration.push_back(oPdoEntry);

    return 0;
}

template <typename T>
int EtherCATMaster::SDODownload(uint16_t p_oSlavePosition, uint16_t p_oIndex, uint8_t p_oSubIndex, T p_oValue)
{
    uint32_t oAbortCode;
    int oRetValue = ecrt_master_sdo_download(m_pEtherCATMaster, p_oSlavePosition, p_oIndex, p_oSubIndex, (uint8_t *)&p_oValue, sizeof(p_oValue), &oAbortCode);
    if (oRetValue != 0)
    {
        wmLogTr(eError, "QnxMsg.VI.ABCDEF", "Error in SDODownload: %d (%d,0x%x,0x%x)\n", oRetValue, p_oSlavePosition, p_oIndex, p_oSubIndex);
        return 0;
    }
    return 1;
}

template <typename T>
int EtherCATMaster::SDOUpload(uint16_t p_oSlavePosition, uint16_t p_oIndex, uint8_t p_oSubIndex, T& p_oValue)
{
    size_t oResultSize = 0;
    uint32_t oAbortCode = 0;
    int oRetValue = ecrt_master_sdo_upload(m_pEtherCATMaster, p_oSlavePosition, p_oIndex, p_oSubIndex, (uint8_t *)&p_oValue, sizeof(p_oValue), &oResultSize, &oAbortCode);
    if (oRetValue != 0)
    {
        wmLogTr(eError, "QnxMsg.VI.ABCDEF", "Error in SDOUpload: %d (%d,%x,%x)\n", oRetValue, p_oSlavePosition, p_oIndex, p_oSubIndex);
        return 0;
    }
    return 1;
}

int EtherCATMaster::SDOConfigurationEPOS4(int p_oEPOS4Index)
{
    bool oSaveAllNeccessary = false;

    uint8_t oTargetBuffer[81] {};
    uint8_t oValueU8 = 0;
    uint16_t oValueU16 = 0;
    uint32_t oValueU32 = 0;
    int32_t oValueS32 = 0;
    uint64_t oValueU64 = 0;

    // read informations about EPOS4 controller
    SDOUpload(m_oEPOS4SlavePosition[p_oEPOS4Index], 0x1000, 0x00, oValueU32);
    wmLog(eDebug, "EPOS4[%d]:[0x%x:0x%x]:Device type  : %x\n", p_oEPOS4Index, 0x1000, 0x00, oValueU32);
    SDOUpload(m_oEPOS4SlavePosition[p_oEPOS4Index], 0x1008, 0x00, oTargetBuffer);
    wmLog(eDebug, "EPOS4[%d]:[0x%x:0x%x]:Device name  : %s\n", p_oEPOS4Index, 0x1008, 0x00, (char *)&oTargetBuffer[0]);
    SDOUpload(m_oEPOS4SlavePosition[p_oEPOS4Index], 0x1018, 0x01, oValueU32);
    wmLog(eDebug, "EPOS4[%d]:[0x%x:0x%x]:Vendor ID    : 0x%x\n", p_oEPOS4Index, 0x1018, 0x01, oValueU32);
    SDOUpload(m_oEPOS4SlavePosition[p_oEPOS4Index], 0x1018, 0x02, oValueU32);
    wmLog(eDebug, "EPOS4[%d]:[0x%x:0x%x]:Product code : 0x%x\n", p_oEPOS4Index, 0x1018, 0x02, oValueU32);
    SDOUpload(m_oEPOS4SlavePosition[p_oEPOS4Index], 0x1018, 0x03, oValueU32);
    wmLog(eDebug, "EPOS4[%d]:[0x%x:0x%x]:Revision     : 0x%x\n", p_oEPOS4Index, 0x1018, 0x03, oValueU32);
    SDOUpload(m_oEPOS4SlavePosition[p_oEPOS4Index], 0x1018, 0x04, oValueU32);
    wmLog(eDebug, "EPOS4[%d]:[0x%x:0x%x]:Serial no    : %d\n", p_oEPOS4Index, 0x1018, 0x04, oValueU32);
    SDOUpload(m_oEPOS4SlavePosition[p_oEPOS4Index], 0x2100, 0x01, oValueU64);
    char oHelpStrg[81];
    sprintf(oHelpStrg, "%ld", oValueU64);
    wmLog(eDebug, "EPOS4[%d]:[0x%x:0x%x]:Serial no long: %s\n", p_oEPOS4Index, 0x2100, 0x01, oHelpStrg);

    // read informations about EtherCAT card
    SDOUpload(m_oEPOS4SlavePosition[p_oEPOS4Index], 0x2101, 0x01, oValueU16);
    wmLog(eDebug, "EPOS4[%d]:[0x%x:0x%x]:ECAT card software  : %x\n", p_oEPOS4Index, 0x2101, 0x01, oValueU16);
    SDOUpload(m_oEPOS4SlavePosition[p_oEPOS4Index], 0x2101, 0x02, oValueU16);
    wmLog(eDebug, "EPOS4[%d]:[0x%x:0x%x]:ECAT card hardware  : %x\n", p_oEPOS4Index, 0x2101, 0x02, oValueU16);
    SDOUpload(m_oEPOS4SlavePosition[p_oEPOS4Index], 0x2101, 0x03, oValueU16);
    wmLog(eDebug, "EPOS4[%d]:[0x%x:0x%x]:ECAT card appl no   : %x\n", p_oEPOS4Index, 0x2101, 0x03, oValueU16);
    SDOUpload(m_oEPOS4SlavePosition[p_oEPOS4Index], 0x2101, 0x04, oValueU16);
    wmLog(eDebug, "EPOS4[%d]:[0x%x:0x%x]:ECAT card appl vers : %x\n", p_oEPOS4Index, 0x2101, 0x04, oValueU16);
    SDOUpload(m_oEPOS4SlavePosition[p_oEPOS4Index], 0x2101, 0x05, oValueU64);
    sprintf(oHelpStrg, "%ld", oValueU64);
    wmLog(eDebug, "EPOS4[%d]:[0x%x:0x%x]:ECAT card Serial no : %s\n", p_oEPOS4Index, 0x2101, 0x05, oHelpStrg);

    // read diagnosis history
    SDOUpload(m_oEPOS4SlavePosition[p_oEPOS4Index], 0x10F3, 0x06, oTargetBuffer);
    wmLog(eDebug, "EPOS4[%d]:[0x%x:0x%x]:Diag history 1: %x%x\n", p_oEPOS4Index, 0x10F3, 0x06, oTargetBuffer[3], oTargetBuffer[2]);
    SDOUpload(m_oEPOS4SlavePosition[p_oEPOS4Index], 0x10F3, 0x07, oTargetBuffer);
    wmLog(eDebug, "EPOS4[%d]:[0x%x:0x%x]:Diag history 2: %x%x\n", p_oEPOS4Index, 0x10F3, 0x07, oTargetBuffer[3], oTargetBuffer[2]);
    SDOUpload(m_oEPOS4SlavePosition[p_oEPOS4Index], 0x10F3, 0x08, oTargetBuffer);
    wmLog(eDebug, "EPOS4[%d]:[0x%x:0x%x]:Diag history 3: %x%x\n", p_oEPOS4Index, 0x10F3, 0x08, oTargetBuffer[3], oTargetBuffer[2]);
    SDOUpload(m_oEPOS4SlavePosition[p_oEPOS4Index], 0x10F3, 0x09, oTargetBuffer);
    wmLog(eDebug, "EPOS4[%d]:[0x%x:0x%x]:Diag history 4: %x%x\n", p_oEPOS4Index, 0x10F3, 0x09, oTargetBuffer[3], oTargetBuffer[2]);
    SDOUpload(m_oEPOS4SlavePosition[p_oEPOS4Index], 0x10F3, 0x0A, oTargetBuffer);
    wmLog(eDebug, "EPOS4[%d]:[0x%x:0x%x]:Diag history 5: %x%x\n", p_oEPOS4Index, 0x10F3, 0x0A, oTargetBuffer[3], oTargetBuffer[2]);

    // Reset Controlword
    oValueU16 = 0;
    SDODownload(m_oEPOS4SlavePosition[p_oEPOS4Index], 0x6040, 0x00, oValueU16);
    usleep(20 * 1000);

    // Motor type
    oValueU16 = 1;
    SDODownload(m_oEPOS4SlavePosition[p_oEPOS4Index], 0x6402, 0x00, oValueU16); // Motor Type: Phase-modulated DC motor
    SDOUpload(m_oEPOS4SlavePosition[p_oEPOS4Index], 0x6402, 0x00, oValueU16);
    wmLog(eDebug, "EPOS4[%d]:[0x%x:0x%x]:Motor type:  %d\n", p_oEPOS4Index, 0x6402, 0x00, oValueU16);

    // Nominal current
    oValueU32 = 1330;
    SDODownload(m_oEPOS4SlavePosition[p_oEPOS4Index], 0x3001, 0x01, oValueU32);  // Motor data: Nominal current: 1330 mA
    SDOUpload(m_oEPOS4SlavePosition[p_oEPOS4Index], 0x3001, 0x01, oValueU32);
    wmLog(eDebug, "EPOS4[%d]:[0x%x:0x%x]:Nominal current:  %d\n", p_oEPOS4Index, 0x3001, 0x01, oValueU32);

    // Output current limit
    oValueU32 = 2660;
    SDODownload(m_oEPOS4SlavePosition[p_oEPOS4Index], 0x3001, 0x02, oValueU32);  // Motor data: Output current limit: 2660 mA
    SDOUpload(m_oEPOS4SlavePosition[p_oEPOS4Index], 0x3001, 0x02, oValueU32);
    wmLog(eDebug, "EPOS4[%d]:[0x%x:0x%x]:Current limit:  %d\n", p_oEPOS4Index, 0x3001, 0x02, oValueU32);

    // Number of pole pairs
    oValueU8 = 1;
    SDODownload(m_oEPOS4SlavePosition[p_oEPOS4Index], 0x3001, 0x03, oValueU8);      // Motor data: Number of pole pairs: 1
    SDOUpload(m_oEPOS4SlavePosition[p_oEPOS4Index], 0x3001, 0x03, oValueU8);
    wmLog(eDebug, "EPOS4[%d]:[0x%x:0x%x]:Pole pairs:  %d\n", p_oEPOS4Index, 0x3001, 0x03, oValueU8);

    // Thermal time constand winding
    oValueU16 = 178;
    SDODownload(m_oEPOS4SlavePosition[p_oEPOS4Index], 0x3001, 0x04, oValueU16);   // Motor data: Thermal time constant: 17.8s
    SDOUpload(m_oEPOS4SlavePosition[p_oEPOS4Index], 0x3001, 0x04, oValueU16);
    wmLog(eDebug, "EPOS4[%d]:[0x%x:0x%x]:Thermal time constant:  %d\n", p_oEPOS4Index, 0x3001, 0x04, oValueU16);

    // Torque constant
    oValueU32 = 35200;
    SDODownload(m_oEPOS4SlavePosition[p_oEPOS4Index], 0x3001, 0x05, oValueU32); // Motor data: Torque constant: 35.2 mNm/A
    SDOUpload(m_oEPOS4SlavePosition[p_oEPOS4Index], 0x3001, 0x05, oValueU32);
    wmLog(eDebug, "EPOS4[%d]:[0x%x:0x%x]:Torque constant:  %d\n", p_oEPOS4Index, 0x3001, 0x05, oValueU32);

    // Max motor speed
    oValueU32 = 6000;
    SDODownload(m_oEPOS4SlavePosition[p_oEPOS4Index], 0x6080, 0x00, oValueU32);
    SDOUpload(m_oEPOS4SlavePosition[p_oEPOS4Index], 0x6080, 0x00, oValueU32);
    wmLog(eDebug, "EPOS4[%d]:[0x%x:0x%x]:Max motor speed:  %d\n", p_oEPOS4Index, 0x6080, 0x00, oValueU32);

    // Gear reduction numerator
    oValueU32 = 24;
    SDODownload(m_oEPOS4SlavePosition[p_oEPOS4Index], 0x3003, 0x01, oValueU32);
    SDOUpload(m_oEPOS4SlavePosition[p_oEPOS4Index], 0x3003, 0x01, oValueU32);
    wmLog(eDebug, "EPOS4[%d]:[0x%x:0x%x]:Gear reduction numerator:  %d\n", p_oEPOS4Index, 0x3003, 0x01, oValueU32);

    // Gear reduction denomiator
    oValueU32 = 5;
    SDODownload(m_oEPOS4SlavePosition[p_oEPOS4Index], 0x3003, 0x02, oValueU32);
    SDOUpload(m_oEPOS4SlavePosition[p_oEPOS4Index], 0x3003, 0x02, oValueU32);
    wmLog(eDebug, "EPOS4[%d]:[0x%x:0x%x]:Gear reduction denomiator:  %d\n", p_oEPOS4Index, 0x3003, 0x02, oValueU32);

    // Max gear input speed
    oValueU32 = 6000;
    SDODownload(m_oEPOS4SlavePosition[p_oEPOS4Index], 0x3003, 0x03, oValueU32);
    SDOUpload(m_oEPOS4SlavePosition[p_oEPOS4Index], 0x3003, 0x03, oValueU32);
    wmLog(eDebug, "EPOS4[%d]:[0x%x:0x%x]:Max gear input speed:  %d\n", p_oEPOS4Index, 0x3003, 0x03, oValueU32);

    // Axis configuration: Sensors
    oValueU32 = 0x00000001; // only Digital incremental encoder 1
    SDODownload(m_oEPOS4SlavePosition[p_oEPOS4Index], 0x3000, 0x01, oValueU32);
    SDOUpload(m_oEPOS4SlavePosition[p_oEPOS4Index], 0x3000, 0x01, oValueU32);
    wmLog(eDebug, "EPOS4[%d]:[0x%x:0x%x]:Axis configuration: Sensors:  0x%x\n", p_oEPOS4Index, 0x3000, 0x01, oValueU32);

    // Axis configuration: Control structure
    oValueU32 = 0x00011111;
    SDODownload(m_oEPOS4SlavePosition[p_oEPOS4Index], 0x3000, 0x02, oValueU32); // 
    SDOUpload(m_oEPOS4SlavePosition[p_oEPOS4Index], 0x3000, 0x02, oValueU32);
    wmLog(eDebug, "EPOS4[%d]:[0x%x:0x%x]:Axis configuration: Control structure:  0x%x\n", p_oEPOS4Index, 0x3000, 0x02, oValueU32);

    // Axis configuration: Commutation sensors
    oValueU32 = 0x00000000;
    SDODownload(m_oEPOS4SlavePosition[p_oEPOS4Index], 0x3000, 0x03, oValueU32); // 
    SDOUpload(m_oEPOS4SlavePosition[p_oEPOS4Index], 0x3000, 0x03, oValueU32);
    wmLog(eDebug, "EPOS4[%d]:[0x%x:0x%x]:Axis configuration: Commutation sensors:  0x%x\n", p_oEPOS4Index, 0x3000, 0x03, oValueU32);

    // Digital incremental encoder 1: Number of pulses
    oValueU32 = 500; // 500 pulses pre revolution
    SDODownload(m_oEPOS4SlavePosition[p_oEPOS4Index], 0x3010, 0x01, oValueU32);   // number of pulses per revolution
    SDOUpload(m_oEPOS4SlavePosition[p_oEPOS4Index], 0x3010, 0x01, oValueU32);
    wmLog(eDebug, "EPOS4[%d]:[0x%x:0x%x]:Number of pulses:  %d\n", p_oEPOS4Index, 0x3010, 0x01, oValueU32);

    // Digital incremental encoder 1: Encoder type
    oValueU16 = 0; // encoder without index channel
    SDODownload(m_oEPOS4SlavePosition[p_oEPOS4Index], 0x3010, 0x02, oValueU16);   // encoder type
    SDOUpload(m_oEPOS4SlavePosition[p_oEPOS4Index], 0x3010, 0x02, oValueU16);
    wmLog(eDebug, "EPOS4[%d]:[0x%x:0x%x]:Encoder type:  %d\n", p_oEPOS4Index, 0x3010, 0x02, oValueU16);

    // Software position limit: Minimum limit
    oValueS32 = -400000;
    SDODownload(m_oEPOS4SlavePosition[p_oEPOS4Index], 0x607D, 0x01, oValueS32); // 
    SDOUpload(m_oEPOS4SlavePosition[p_oEPOS4Index], 0x607D, 0x01, oValueS32);
    wmLog(eDebug, "EPOS4[%d]:[0x%x:0x%x]:Software position limit: Minimum limit: %d\n", p_oEPOS4Index, 0x607D, 0x01, oValueS32);

    // Software position limit: Maximum limit
    oValueS32 = 400000;
    SDODownload(m_oEPOS4SlavePosition[p_oEPOS4Index], 0x607D, 0x02, oValueS32); // 
    SDOUpload(m_oEPOS4SlavePosition[p_oEPOS4Index], 0x607D, 0x02, oValueS32);
    wmLog(eDebug, "EPOS4[%d]:[0x%x:0x%x]:Software position limit: Maximum limit: %d\n", p_oEPOS4Index, 0x607D, 0x02, oValueS32);

    // Homing acceleration
    oValueU32 = 2500;
    SDODownload(m_oEPOS4SlavePosition[p_oEPOS4Index], 0x609A, 0x00, oValueU32); // 
    SDOUpload(m_oEPOS4SlavePosition[p_oEPOS4Index], 0x609A, 0x00, oValueU32);
    wmLog(eDebug, "EPOS4[%d]:[0x%x:0x%x]:Homing acceleration: %d\n", p_oEPOS4Index, 0x609A, 0x00, oValueU32);

    // Max acceleration
    oValueU32 = 200000;
    SDODownload(m_oEPOS4SlavePosition[p_oEPOS4Index], 0x60C5, 0x00, oValueU32); // 
    SDOUpload(m_oEPOS4SlavePosition[p_oEPOS4Index], 0x60C5, 0x00, oValueU32);
    wmLog(eDebug, "EPOS4[%d]:[0x%x:0x%x]:Max acceleration: %d\n", p_oEPOS4Index, 0x60C5, 0x00, oValueU32);

    // Quick stop deceleration
    oValueU32 = 160000;
    SDODownload(m_oEPOS4SlavePosition[p_oEPOS4Index], 0x6085, 0x00, oValueU32); // 
    SDOUpload(m_oEPOS4SlavePosition[p_oEPOS4Index], 0x6085, 0x00, oValueU32);
    wmLog(eDebug, "EPOS4[%d]:[0x%x:0x%x]:Quick stop deceleration: %d\n", p_oEPOS4Index, 0x6085, 0x00, oValueU32);

    // Current control parameter set: P gain
    oValueU32 = 7466790; // mV/A
    SDODownload(m_oEPOS4SlavePosition[p_oEPOS4Index], 0x30A0, 0x01, oValueU32); // 
    SDOUpload(m_oEPOS4SlavePosition[p_oEPOS4Index], 0x30A0, 0x01, oValueU32);
    wmLog(eDebug, "EPOS4[%d]:[0x%x:0x%x]:Current control P gain: %d\n", p_oEPOS4Index, 0x30A0, 0x01, oValueU32);
    // Current control parameter set: I gain
    oValueU32 = 44611448; // mV/(A*ms)
    SDODownload(m_oEPOS4SlavePosition[p_oEPOS4Index], 0x30A0, 0x02, oValueU32); // 
    SDOUpload(m_oEPOS4SlavePosition[p_oEPOS4Index], 0x30A0, 0x02, oValueU32);
    wmLog(eDebug, "EPOS4[%d]:[0x%x:0x%x]:Current control I gain: %d\n", p_oEPOS4Index, 0x30A0, 0x02, oValueU32);

    // Position control parameter set: P gain
    oValueU32 = 2480741; // mA/rad
    SDODownload(m_oEPOS4SlavePosition[p_oEPOS4Index], 0x30A1, 0x01, oValueU32); // 
    SDOUpload(m_oEPOS4SlavePosition[p_oEPOS4Index], 0x30A1, 0x01, oValueU32);
    wmLog(eDebug, "EPOS4[%d]:[0x%x:0x%x]:Position control P gain: %d\n", p_oEPOS4Index, 0x30A1, 0x01, oValueU32);
    // Position control parameter set: I gain
    oValueU32 = 16045397; // mA/(rad*s)
    SDODownload(m_oEPOS4SlavePosition[p_oEPOS4Index], 0x30A1, 0x02, oValueU32); // 
    SDOUpload(m_oEPOS4SlavePosition[p_oEPOS4Index], 0x30A1, 0x02, oValueU32);
    wmLog(eDebug, "EPOS4[%d]:[0x%x:0x%x]:Position control I gain: %d\n", p_oEPOS4Index, 0x30A1, 0x02, oValueU32);
    // Position control parameter set: D gain
    oValueU32 = 29097; // mA*s/rad
    SDODownload(m_oEPOS4SlavePosition[p_oEPOS4Index], 0x30A1, 0x03, oValueU32); // 
    SDOUpload(m_oEPOS4SlavePosition[p_oEPOS4Index], 0x30A1, 0x03, oValueU32);
    wmLog(eDebug, "EPOS4[%d]:[0x%x:0x%x]:Position control D gain: %d\n", p_oEPOS4Index, 0x30A1, 0x03, oValueU32);
    // Position control parameter set: FF velocity gain
    oValueU32 = 2865; // mA*S/rad
    SDODownload(m_oEPOS4SlavePosition[p_oEPOS4Index], 0x30A1, 0x04, oValueU32); // 
    SDOUpload(m_oEPOS4SlavePosition[p_oEPOS4Index], 0x30A1, 0x04, oValueU32);
    wmLog(eDebug, "EPOS4[%d]:[0x%x:0x%x]:Position control FF vel gain: %d\n", p_oEPOS4Index, 0x30A1, 0x04, oValueU32);
    // Position control parameter set: FF acceleration gain
    oValueU32 = 137; // mA*s2/rad
    SDODownload(m_oEPOS4SlavePosition[p_oEPOS4Index], 0x30A1, 0x05, oValueU32); // 
    SDOUpload(m_oEPOS4SlavePosition[p_oEPOS4Index], 0x30A1, 0x05, oValueU32);
    wmLog(eDebug, "EPOS4[%d]:[0x%x:0x%x]:Position control FF accel gain: %d\n", p_oEPOS4Index, 0x30A1, 0x05, oValueU32);

    // Velocity control parameter set: P gain
    oValueU32 = 37578; // mA*s/rad
    SDODownload(m_oEPOS4SlavePosition[p_oEPOS4Index], 0x30A2, 0x01, oValueU32); // 
    SDOUpload(m_oEPOS4SlavePosition[p_oEPOS4Index], 0x30A2, 0x01, oValueU32);
    wmLog(eDebug, "EPOS4[%d]:[0x%x:0x%x]:Velocity control P gain: %d\n", p_oEPOS4Index, 0x30A2, 0x01, oValueU32);
    // Velocity control parameter set: I gain
    oValueU32 = 3438662; // mA/rad
    SDODownload(m_oEPOS4SlavePosition[p_oEPOS4Index], 0x30A2, 0x02, oValueU32); // 
    SDOUpload(m_oEPOS4SlavePosition[p_oEPOS4Index], 0x30A2, 0x02, oValueU32);
    wmLog(eDebug, "EPOS4[%d]:[0x%x:0x%x]:Velocity control I gain: %d\n", p_oEPOS4Index, 0x30A2, 0x02, oValueU32);
    // Velocity control parameter set: FF velocity gain
    oValueU32 = 2988; // mA*s/rad
    SDODownload(m_oEPOS4SlavePosition[p_oEPOS4Index], 0x30A2, 0x03, oValueU32); // 
    SDOUpload(m_oEPOS4SlavePosition[p_oEPOS4Index], 0x30A2, 0x03, oValueU32);
    wmLog(eDebug, "EPOS4[%d]:[0x%x:0x%x]:Velocity control FF vel gain: %d\n", p_oEPOS4Index, 0x30A2, 0x03, oValueU32);
    // Velocity control parameter set: FF acceleration gain
    oValueU32 = 138; // mA*s2/rad
    SDODownload(m_oEPOS4SlavePosition[p_oEPOS4Index], 0x30A2, 0x04, oValueU32); // 
    SDOUpload(m_oEPOS4SlavePosition[p_oEPOS4Index], 0x30A2, 0x04, oValueU32);
    wmLog(eDebug, "EPOS4[%d]:[0x%x:0x%x]:Velocity control FF accel gain: %d\n", p_oEPOS4Index, 0x30A2, 0x04, oValueU32);

    // digital inputs
    oValueU16 = 0x0003; // change polarity
    //oValueU16 = 0x0000; // no polarity change
    SDODownload(m_oEPOS4SlavePosition[p_oEPOS4Index], 0x3141, 0x02, oValueU16);   // Polarity switch of neg. limit and pos. limit switches
    SDOUpload(m_oEPOS4SlavePosition[p_oEPOS4Index], 0x3141, 0x02, oValueU16);
    wmLog(eDebug, "EPOS4[%d]:[0x%x:0x%x]:DigIn polarity:  0x%x\n", p_oEPOS4Index, 0x3141, 0x02, oValueU16);
    oValueU8 = 0; // negative limit switch
    SDODownload(m_oEPOS4SlavePosition[p_oEPOS4Index], 0x3142, 0x01, oValueU8); // configuration for input 1
    SDOUpload(m_oEPOS4SlavePosition[p_oEPOS4Index], 0x3142, 0x01, oValueU8);
    wmLog(eDebug, "EPOS4[%d]:[0x%x:0x%x]:Func DigIn 1:  %d\n", p_oEPOS4Index, 0x3142, 0x01, oValueU8);
    oValueU8 = 1; // positive limit switch
    SDODownload(m_oEPOS4SlavePosition[p_oEPOS4Index], 0x3142, 0x02, oValueU8); // configuration for input 2
    SDOUpload(m_oEPOS4SlavePosition[p_oEPOS4Index], 0x3142, 0x02, oValueU8);
    wmLog(eDebug, "EPOS4[%d]:[0x%x:0x%x]:Func DigIn 2:  %d\n", p_oEPOS4Index, 0x3142, 0x02, oValueU8);
    oValueU8 = 255; // no function for input
    SDODownload(m_oEPOS4SlavePosition[p_oEPOS4Index], 0x3142, 0x03, oValueU8); // configuration for input 3
    SDOUpload(m_oEPOS4SlavePosition[p_oEPOS4Index], 0x3142, 0x03, oValueU8);
    wmLog(eDebug, "EPOS4[%d]:[0x%x:0x%x]:Func DigIn 3:  %d\n", p_oEPOS4Index, 0x3142, 0x03, oValueU8);
    oValueU8 = 255; // no function for input
    SDODownload(m_oEPOS4SlavePosition[p_oEPOS4Index], 0x3142, 0x04, oValueU8); // configuration for input 4
    SDOUpload(m_oEPOS4SlavePosition[p_oEPOS4Index], 0x3142, 0x04, oValueU8);
    wmLog(eDebug, "EPOS4[%d]:[0x%x:0x%x]:Func DigIn 4:  %d\n", p_oEPOS4Index, 0x3142, 0x04, oValueU8);
    SDOUpload(m_oEPOS4SlavePosition[p_oEPOS4Index], 0x3142, 0x05, oValueU8);
    wmLog(eDebug, "EPOS4[%d]:[0x%x:0x%x]:Func HSIn 1:  %d\n", p_oEPOS4Index, 0x3142, 0x05, oValueU8);
    SDOUpload(m_oEPOS4SlavePosition[p_oEPOS4Index], 0x3142, 0x06, oValueU8);
    wmLog(eDebug, "EPOS4[%d]:[0x%x:0x%x]:Func HSIn 2:  %d\n", p_oEPOS4Index, 0x3142, 0x06, oValueU8);
    SDOUpload(m_oEPOS4SlavePosition[p_oEPOS4Index], 0x3142, 0x07, oValueU8);
    wmLog(eDebug, "EPOS4[%d]:[0x%x:0x%x]:Func HSIn 3:  %d\n", p_oEPOS4Index, 0x3142, 0x07, oValueU8);
    SDOUpload(m_oEPOS4SlavePosition[p_oEPOS4Index], 0x3142, 0x08, oValueU8);
    wmLog(eDebug, "EPOS4[%d]:[0x%x:0x%x]:Func HSIn 4:  %d\n", p_oEPOS4Index, 0x3142, 0x08, oValueU8);
    SDOUpload(m_oEPOS4SlavePosition[p_oEPOS4Index], 0x3141, 0x01, oValueU16);
    wmLog(eDebug, "EPOS4[%d]:[0x%x:0x%x]:DigIn state:  0x%x\n", p_oEPOS4Index, 0x3141, 0x01, oValueU16);

    // digital outputs
    //oValueU16 = 0x0003; // change polarity
    oValueU16 = 0x0000; // no polarity change
    SDODownload(m_oEPOS4SlavePosition[p_oEPOS4Index], 0x3150, 0x02, oValueU16); // Polarity correction of digital outputs
    SDOUpload(m_oEPOS4SlavePosition[p_oEPOS4Index], 0x3150, 0x02, oValueU16);
    wmLog(eDebug, "EPOS4[%d]:[0x%x:0x%x]:DigOut polarity:  0x%x\n", p_oEPOS4Index, 0x3150, 0x02, oValueU16);
    // function of digital oputputs
    SDOUpload(m_oEPOS4SlavePosition[p_oEPOS4Index], 0x3151, 0x01, oValueU8);
    wmLog(eDebug, "EPOS4[%d]:[0x%x:0x%x]:Func DigOut 1:  %d\n", p_oEPOS4Index, 0x3151, 0x01, oValueU8);
    if (oValueU8 != 24)
    {
        oSaveAllNeccessary = true;
    }
    oValueU8 = 24; // holding brake
    SDODownload(m_oEPOS4SlavePosition[p_oEPOS4Index], 0x3151, 0x01, oValueU8); // configuration for digital output 1
    SDOUpload(m_oEPOS4SlavePosition[p_oEPOS4Index], 0x3151, 0x01, oValueU8);
    wmLog(eDebug, "EPOS4[%d]:[0x%x:0x%x]:Func DigOut 1:  %d\n", p_oEPOS4Index, 0x3151, 0x01, oValueU8);
    oValueU8 = 255; // none
    SDODownload(m_oEPOS4SlavePosition[p_oEPOS4Index], 0x3151, 0x02, oValueU8); // configuration for digital output 2
    SDOUpload(m_oEPOS4SlavePosition[p_oEPOS4Index], 0x3151, 0x02, oValueU8);
    wmLog(eDebug, "EPOS4[%d]:[0x%x:0x%x]:Func DigOut 2:  %d\n", p_oEPOS4Index, 0x3151, 0x02, oValueU8);
    oValueU8 = 255; // none
    SDODownload(m_oEPOS4SlavePosition[p_oEPOS4Index], 0x3151, 0x03, oValueU8); // configuration for high speed output 1
    SDOUpload(m_oEPOS4SlavePosition[p_oEPOS4Index], 0x3151, 0x03, oValueU8);
    wmLog(eDebug, "EPOS4[%d]:[0x%x:0x%x]:Func HSOut 1:  %d\n", p_oEPOS4Index, 0x3151, 0x03, oValueU8);
    // holding brake parameters
    oValueU16 = 15; // 15ms
    SDODownload(m_oEPOS4SlavePosition[p_oEPOS4Index], 0x3158, 0x01, oValueU16); // holding brake: rise time
    SDOUpload(m_oEPOS4SlavePosition[p_oEPOS4Index], 0x3158, 0x01, oValueU16);
    wmLog(eDebug, "EPOS4[%d]:[0x%x:0x%x]:Brake rise time:  %d\n", p_oEPOS4Index, 0x3158, 0x01, oValueU16);
    oValueU16 = 25; // 25ms
    SDODownload(m_oEPOS4SlavePosition[p_oEPOS4Index], 0x3158, 0x02, oValueU16); // holding brake: fall time
    SDOUpload(m_oEPOS4SlavePosition[p_oEPOS4Index], 0x3158, 0x02, oValueU16);
    wmLog(eDebug, "EPOS4[%d]:[0x%x:0x%x]:Brake fall time:  %d\n", p_oEPOS4Index, 0x3158, 0x02, oValueU16);
    SDOUpload(m_oEPOS4SlavePosition[p_oEPOS4Index], 0x3158, 0x03, oValueU8); // holding brake: brake state
    wmLog(eDebug, "EPOS4[%d]:[0x%x:0x%x]:Brake state:  0x%x\n", p_oEPOS4Index, 0x3158, 0x03, oValueU16);
    // state of digital outputs
    SDOUpload(m_oEPOS4SlavePosition[p_oEPOS4Index], 0x3150, 0x01, oValueU16); // output state after polarity correction
    wmLog(eDebug, "EPOS4[%d]:[0x%x:0x%x]:DigOut state:  0x%x\n", p_oEPOS4Index, 0x3150, 0x01, oValueU16);

    SDOUpload(m_oEPOS4SlavePosition[p_oEPOS4Index], 0x1600, 0x00, oValueU8);
    wmLog(eDebug, "EPOS4[%d]:[0x%x:0x%x]:  %d\n", p_oEPOS4Index, 0x1600, 0x00, oValueU8);
    for(int i = 1;i <= oValueU8;i++)
    {
        SDOUpload(m_oEPOS4SlavePosition[p_oEPOS4Index], 0x1600, i, oValueU32);
        wmLog(eDebug, "EPOS4[%d]:[0x%x:0x%x]: 0x%x\n", p_oEPOS4Index, 0x1600, i, oValueU32);
    }
    SDOUpload(m_oEPOS4SlavePosition[p_oEPOS4Index], 0x1601, 0x00, oValueU8);
    wmLog(eDebug, "EPOS4[%d]:[0x%x:0x%x]:  %d\n", p_oEPOS4Index, 0x1601, 0x00, oValueU8);
    for(int i = 1;i <= oValueU8;i++)
    {
        SDOUpload(m_oEPOS4SlavePosition[p_oEPOS4Index], 0x1601, i, oValueU32);
        wmLog(eDebug, "EPOS4[%d]:[0x%x:0x%x]: 0x%x\n", p_oEPOS4Index, 0x1601, i, oValueU32);
    }
    SDOUpload(m_oEPOS4SlavePosition[p_oEPOS4Index], 0x1602, 0x00, oValueU8);
    wmLog(eDebug, "EPOS4[%d]:[0x%x:0x%x]:  %d\n", p_oEPOS4Index, 0x1602, 0x00, oValueU8);
    for(int i = 1;i <= oValueU8;i++)
    {
        SDOUpload(m_oEPOS4SlavePosition[p_oEPOS4Index], 0x1602, i, oValueU32);
        wmLog(eDebug, "EPOS4[%d]:[0x%x:0x%x]: 0x%x\n", p_oEPOS4Index, 0x1602, i, oValueU32);
    }
    SDOUpload(m_oEPOS4SlavePosition[p_oEPOS4Index], 0x1603, 0x00, oValueU8);
    wmLog(eDebug, "EPOS4[%d]:[0x%x:0x%x]:  %d\n", p_oEPOS4Index, 0x1603, 0x00, oValueU8);
    for(int i = 1;i <= oValueU8;i++)
    {
        SDOUpload(m_oEPOS4SlavePosition[p_oEPOS4Index], 0x1603, i, oValueU32);
        wmLog(eDebug, "EPOS4[%d]:[0x%x:0x%x]: 0x%x\n", p_oEPOS4Index, 0x1603, i, oValueU32);
    }
    SDOUpload(m_oEPOS4SlavePosition[p_oEPOS4Index], 0x1A00, 0x00, oValueU8);
    wmLog(eDebug, "EPOS4[%d]:[0x%x:0x%x]:  %d\n", p_oEPOS4Index, 0x1A00, 0x00, oValueU8);
    for(int i = 1;i <= oValueU8;i++)
    {
        SDOUpload(m_oEPOS4SlavePosition[p_oEPOS4Index], 0x1A00, i, oValueU32);
        wmLog(eDebug, "EPOS4[%d]:[0x%x:0x%x]: 0x%x\n", p_oEPOS4Index, 0x1A00, i, oValueU32);
    }
    SDOUpload(m_oEPOS4SlavePosition[p_oEPOS4Index], 0x1A01, 0x00, oValueU8);
    wmLog(eDebug, "EPOS4[%d]:[0x%x:0x%x]:  %d\n", p_oEPOS4Index, 0x1A01, 0x00, oValueU8);
    for(int i = 1;i <= oValueU8;i++)
    {
        SDOUpload(m_oEPOS4SlavePosition[p_oEPOS4Index], 0x1A01, i, oValueU32);
        wmLog(eDebug, "EPOS4[%d]:[0x%x:0x%x]: 0x%x\n", p_oEPOS4Index, 0x1A01, i, oValueU32);
    }
    SDOUpload(m_oEPOS4SlavePosition[p_oEPOS4Index], 0x1A02, 0x00, oValueU8);
    wmLog(eDebug, "EPOS4[%d]:[0x%x:0x%x]:  %d\n", p_oEPOS4Index, 0x1A02, 0x00, oValueU8);
    for(int i = 1;i <= oValueU8;i++)
    {
        SDOUpload(m_oEPOS4SlavePosition[p_oEPOS4Index], 0x1A02, i, oValueU32);
        wmLog(eDebug, "EPOS4[%d]:[0x%x:0x%x]: 0x%x\n", p_oEPOS4Index, 0x1A02, i, oValueU32);
    }
    SDOUpload(m_oEPOS4SlavePosition[p_oEPOS4Index], 0x1A03, 0x00, oValueU8);
    wmLog(eDebug, "EPOS4[%d]:[0x%x:0x%x]::  %d\n", p_oEPOS4Index, 0x1A03, 0x00, oValueU8);
    for(int i = 1;i <= oValueU8;i++)
    {
        SDOUpload(m_oEPOS4SlavePosition[p_oEPOS4Index], 0x1A03, i, oValueU32);
        wmLog(eDebug, "EPOS4[%d]:[0x%x:0x%x]: 0x%x\n", p_oEPOS4Index, 0x1A03, i, oValueU32);
    }
    SDOUpload(m_oEPOS4SlavePosition[p_oEPOS4Index], 0x1C12, 0x00, oValueU8);
    wmLog(eDebug, "EPOS4[%d]:[0x%x:0x%x]:  %d\n", p_oEPOS4Index, 0x1C12, 0x00, oValueU8);
    SDOUpload(m_oEPOS4SlavePosition[p_oEPOS4Index], 0x1C13, 0x00, oValueU8);
    wmLog(eDebug, "EPOS4[%d]:[0x%x:0x%x]:  %d\n", p_oEPOS4Index, 0x1C13, 0x00, oValueU8);

    if (oSaveAllNeccessary)
    {
        wmLog(eDebug, "EPOS4[%d]:execute function: store parameters\n", p_oEPOS4Index);
        oValueU32 = 0x65766173;
        SDODownload(m_oEPOS4SlavePosition[p_oEPOS4Index], 0x1010, 0x01, oValueU32); // save all parameters
        usleep(100*1000);
        SDOUpload(m_oEPOS4SlavePosition[p_oEPOS4Index], 0x1010, 0x01, oValueU32);
        wmLog(eDebug, "EPOS4[%d]:[0x%x:0x%x]:  0x%x\n", p_oEPOS4Index, 0x1010, 0x01, oValueU32);
    }

    if (!(m_pEPOS4_ErrorRegister_Request[p_oEPOS4Index] = ecrt_slave_config_create_sdo_request(m_pEPOS4SlaveConfig[p_oEPOS4Index], 0x1001, 0, 1)))
    {
        wmLogTr(eError, "QnxMsg.VI.ABCDEF", "Failed to create SDO request (m_pEPOS4_ErrorRegister_Request).\n");
    }
    ecrt_sdo_request_timeout(m_pEPOS4_ErrorRegister_Request[p_oEPOS4Index], 40); // ms

    if (!(m_pEPOS4_HomeOffset_Request[p_oEPOS4Index] = ecrt_slave_config_create_sdo_request(m_pEPOS4SlaveConfig[p_oEPOS4Index], 0x30B1, 0, 4)))
    {
        wmLogTr(eError, "QnxMsg.VI.ABCDEF", "Failed to create SDO request (m_pEPOS4_HomeOffset_Request).\n");
    }
    ecrt_sdo_request_timeout(m_pEPOS4_HomeOffset_Request[p_oEPOS4Index], 40); // ms

    SDOUpload(m_oEPOS4SlavePosition[p_oEPOS4Index], 0x1001, 0x00, oValueU8);
    wmLog(eDebug, "EPOS4[%d]:[0x%x:0x%x]:ErrorRegister 0x%x\n", p_oEPOS4Index, 0x1001, 0x00, oValueU8);

    // reset Fault
    oValueU16 = 0x0080; // set Fault reset
    SDODownload(m_oEPOS4SlavePosition[p_oEPOS4Index], 0x6040, 0x00, oValueU16);   // write Controlword
    SDOUpload(m_oEPOS4SlavePosition[p_oEPOS4Index], 0x6040, 0x00, oValueU16); // read Controlword
    wmLog(eDebug, "EPOS4[%d]:[0x%x:0x%x]:Controlword  0x%x\n", p_oEPOS4Index, 0x6040, 0x00, oValueU16);
    oValueU16 &= ~0x0080; // reset Fault reset
    SDODownload(m_oEPOS4SlavePosition[p_oEPOS4Index], 0x6040, 0x00, oValueU16);   // write Controlword
    SDOUpload(m_oEPOS4SlavePosition[p_oEPOS4Index], 0x6040, 0x00, oValueU16); // read Controlword
    wmLog(eDebug, "EPOS4[%d]:[0x%x:0x%x]:Controlword  0x%x\n", p_oEPOS4Index, 0x6040, 0x00, oValueU16);

    // read error register
    SDOUpload(m_oEPOS4SlavePosition[p_oEPOS4Index], 0x1001, 0x00, oValueU8);
    wmLog(eDebug, "EPOS4[%d]:[0x%x:0x%x]:ErrorRegister 0x%x\n", p_oEPOS4Index, 0x1001, 0x00, oValueU8);
    if (oValueU8 != 0x00)
    {
        wmLogTr(eError, "QnxMsg.VI.ABCDEF", "EPOS4[%d]:Error register ist not clean after reset fault (%x)\n", p_oEPOS4Index, oValueU8);

        // read diagnosis history
        SDOUpload(m_oEPOS4SlavePosition[p_oEPOS4Index], 0x10F3, 0x06, oTargetBuffer);
        wmLog(eDebug, "EPOS4[%d]:[0x%x:0x%x]:Diag history 1: %x%x\n", p_oEPOS4Index, 0x10F3, 0x06, oTargetBuffer[3], oTargetBuffer[2]);
        SDOUpload(m_oEPOS4SlavePosition[p_oEPOS4Index], 0x10F3, 0x07, oTargetBuffer);
        wmLog(eDebug, "EPOS4[%d]:[0x%x:0x%x]:Diag history 2: %x%x\n", p_oEPOS4Index, 0x10F3, 0x07, oTargetBuffer[3], oTargetBuffer[2]);
        SDOUpload(m_oEPOS4SlavePosition[p_oEPOS4Index], 0x10F3, 0x08, oTargetBuffer);
        wmLog(eDebug, "EPOS4[%d]:[0x%x:0x%x]:Diag history 3: %x%x\n", p_oEPOS4Index, 0x10F3, 0x08, oTargetBuffer[3], oTargetBuffer[2]);
        SDOUpload(m_oEPOS4SlavePosition[p_oEPOS4Index], 0x10F3, 0x09, oTargetBuffer);
        wmLog(eDebug, "EPOS4[%d]:[0x%x:0x%x]:Diag history 4: %x%x\n", p_oEPOS4Index, 0x10F3, 0x09, oTargetBuffer[3], oTargetBuffer[2]);
        SDOUpload(m_oEPOS4SlavePosition[p_oEPOS4Index], 0x10F3, 0x0A, oTargetBuffer);
        wmLog(eDebug, "EPOS4[%d]:[0x%x:0x%x]:Diag history 5: %x%x\n", p_oEPOS4Index, 0x10F3, 0x0A, oTargetBuffer[3], oTargetBuffer[2]);
    }

    return 1;
}

int EtherCATMaster::StartupEtherCATMaster(void)
{
    char oDebugStrg[81];
	int oRetValue;

	if (!m_pEtherCATMaster)
	{
		return -1;
	}

	m_pDomain1 = ecrt_master_create_domain(m_pEtherCATMaster);
	sprintf(oDebugStrg, "0x%08lX", (unsigned long)m_pDomain1);
    wmLog(eDebug, "Return of ecrt_master_create_domain: %s\n", oDebugStrg);
	if (!m_pDomain1)
    {
        wmLogTr(eError, "QnxMsg.VI.ECMCreateDomFailed", "Cannot create domain !\n");
		return -1;
    }

	oRetValue = ecrt_domain_reg_pdo_entry_list(m_pDomain1, m_oPdoEntryRegistration.data());
    wmLog(eDebug, "Return of ecrt_domain_reg_pdo_entry_list: %d\n", oRetValue);
	if (oRetValue != 0)
	{
        wmLogTr(eError, "QnxMsg.VI.ECMPDORegFailed", "PDO entry registration failed !\n");
		return -1;
	}

	// now the Input_Offset und Output_Offset variables are filled with valid values
	unsigned int oLocalIndexEK1100 = 0;
	unsigned int oLocalIndexEL1018 = 0;
	unsigned int oLocalIndexEL2008 = 0;
	unsigned int oLocalIndexEL3102 = 0;
	unsigned int oLocalIndexEL3702 = 0;
	unsigned int oLocalIndexEL4102 = 0;
	unsigned int oLocalIndexEL4132 = 0;
	unsigned int oLocalIndexEL5101 = 0;
    unsigned int localIndexEL5151 = 0;
	unsigned int oLocalIndexGATEWAY = 0;
	unsigned int oLocalIndexACCELNET = 0;
	unsigned int oLocalIndexEPOS4 = 0;
	unsigned int oLocalIndexEK1310 = 0;
	unsigned int oLocalIndexFRONTEND = 0;
    for(unsigned int i = 0;i < m_oSlaveInfo4Service.size();i++)
    {
        if ((m_oSlaveInfo4Service[i].dwVendorId == VENDORID_BECKHOFF) && (m_oSlaveInfo4Service[i].dwProductCode == PRODUCTCODE_EL1018))
        {
            m_oSlaveInfo4Service[i].dwPdOffsIn  = EL1018_Input_Offset[oLocalIndexEL1018] * 8; // Wert ist Bit-Position
            m_oSlaveInfo4Service[i].dwPdOffsOut = -1;
            oLocalIndexEL1018++;
	    }
	    else if ((m_oSlaveInfo4Service[i].dwVendorId == VENDORID_BECKHOFF) && (m_oSlaveInfo4Service[i].dwProductCode == PRODUCTCODE_EL2008))
	    {
            m_oSlaveInfo4Service[i].dwPdOffsIn  = -1;
            m_oSlaveInfo4Service[i].dwPdOffsOut = EL2008_Output_Offset[oLocalIndexEL2008] * 8; // Wert ist Bit-Position
            oLocalIndexEL2008++;
	    }
	    else if ((m_oSlaveInfo4Service[i].dwVendorId == VENDORID_BECKHOFF) && (m_oSlaveInfo4Service[i].dwProductCode == PRODUCTCODE_EL3102))
	    {
            m_oSlaveInfo4Service[i].dwPdOffsIn  = EL3102_Input_Offset[oLocalIndexEL3102] * 8; // Wert ist Bit-Position
            m_oSlaveInfo4Service[i].dwPdOffsOut = -1;
            oLocalIndexEL3102++;
	    }
	    else if ((m_oSlaveInfo4Service[i].dwVendorId == VENDORID_BECKHOFF) && (m_oSlaveInfo4Service[i].dwProductCode == PRODUCTCODE_EL3702))
	    {
            m_oSlaveInfo4Service[i].dwPdOffsIn  = EL3702_CH1_Input_Offset[oLocalIndexEL3702] * 8; // Wert ist Bit-Position
            m_oSlaveInfo4Service[i].dwPdOffsOut = -1;
            oLocalIndexEL3702++;
	    }
	    else if ((m_oSlaveInfo4Service[i].dwVendorId == VENDORID_BECKHOFF) && (m_oSlaveInfo4Service[i].dwProductCode == PRODUCTCODE_EL4102))
	    {
            m_oSlaveInfo4Service[i].dwPdOffsIn  = -1;
            m_oSlaveInfo4Service[i].dwPdOffsOut = EL4102_Output_Offset[oLocalIndexEL4102] * 8; // Wert ist Bit-Position
            oLocalIndexEL4102++;
	    }
	    else if ((m_oSlaveInfo4Service[i].dwVendorId == VENDORID_BECKHOFF) && (m_oSlaveInfo4Service[i].dwProductCode == PRODUCTCODE_EL4132))
	    {
            m_oSlaveInfo4Service[i].dwPdOffsIn  = -1;
            m_oSlaveInfo4Service[i].dwPdOffsOut = EL4132_Output_Offset[oLocalIndexEL4132] * 8; // Wert ist Bit-Position
            oLocalIndexEL4132++;
	    }
	    else if ((m_oSlaveInfo4Service[i].dwVendorId == VENDORID_BECKHOFF) && (m_oSlaveInfo4Service[i].dwProductCode == PRODUCTCODE_EL5101))
	    {
            m_oSlaveInfo4Service[i].dwPdOffsIn  = EL5101_Input_Offset[oLocalIndexEL5101] * 8; // Wert ist Bit-Position
            m_oSlaveInfo4Service[i].dwPdOffsOut = EL5101_Output_Offset[oLocalIndexEL5101] * 8; // Wert ist Bit-Position
            oLocalIndexEL5101++;
	    }
        else if ((m_oSlaveInfo4Service[i].dwVendorId == VENDORID_BECKHOFF) && (m_oSlaveInfo4Service[i].dwProductCode == PRODUCTCODE_EL5151))
        {
            m_oSlaveInfo4Service[i].dwPdOffsIn  = EL5151InputOffset[localIndexEL5151] * 8; // position is a Bit value
            m_oSlaveInfo4Service[i].dwPdOffsOut = EL5151OutputOffset[localIndexEL5151] * 8; // position is a Bit value
            localIndexEL5151++;
        }
	    else if ((m_oSlaveInfo4Service[i].dwVendorId == VENDORID_KUNBUS) && (m_oSlaveInfo4Service[i].dwProductCode == PRODUCTCODE_KUNBUS_GW))
	    {
            m_oSlaveInfo4Service[i].dwPdOffsIn  = GATEWAY_Input_Offset[oLocalIndexGATEWAY] * 8; // Wert ist Bit-Position
            m_oSlaveInfo4Service[i].dwPdOffsOut = GATEWAY_Output_Offset[oLocalIndexGATEWAY] * 8; // Wert ist Bit-Position
            oLocalIndexGATEWAY++;
	    }
        else if (((m_oSlaveInfo4Service[i].dwVendorId == VENDORID_HMS) && (m_oSlaveInfo4Service[i].dwProductCode == PRODUCTCODE_ANYBUS_GW)) ||
                 ((m_oSlaveInfo4Service[i].dwVendorId == VENDORID_HMS) && (m_oSlaveInfo4Service[i].dwProductCode == PRODUCTCODE_COMMUNICATOR_GW)))
	    {
            m_oSlaveInfo4Service[i].dwPdOffsIn  = GATEWAY_Input_Offset[oLocalIndexGATEWAY] * 8; // Wert ist Bit-Position
            m_oSlaveInfo4Service[i].dwPdOffsOut = GATEWAY_Output_Offset[oLocalIndexGATEWAY] * 8; // Wert ist Bit-Position
            oLocalIndexGATEWAY++;
	    }
	    else if ((m_oSlaveInfo4Service[i].dwVendorId == VENDORID_COPLEY) && (m_oSlaveInfo4Service[i].dwProductCode == PRODUCTCODE_ACCELNET))
	    {
            m_oSlaveInfo4Service[i].dwPdOffsIn  = ACCELNET_Input_Offset[oLocalIndexACCELNET] * 8; // Wert ist Bit-Position
            m_oSlaveInfo4Service[i].dwPdOffsOut = ACCELNET_Output_Offset[oLocalIndexACCELNET] * 8; // Wert ist Bit-Position
            oLocalIndexACCELNET++;
	    }
        else if ((m_oSlaveInfo4Service[i].dwVendorId == VENDORID_MAXON) && (m_oSlaveInfo4Service[i].dwProductCode == PRODUCTCODE_EPOS4))
        {
            m_oSlaveInfo4Service[i].dwPdOffsIn  = EPOS4_Input_Offset[oLocalIndexEPOS4] * 8; // Wert ist Bit-Position
            m_oSlaveInfo4Service[i].dwPdOffsOut = EPOS4_Output_Offset[oLocalIndexEPOS4] * 8; // Wert ist Bit-Position
            oLocalIndexEPOS4++;
        }
	    else if ((m_oSlaveInfo4Service[i].dwVendorId == VENDORID_BECKHOFF) && (m_oSlaveInfo4Service[i].dwProductCode == PRODUCTCODE_EK1100))
	    {
            m_oSlaveInfo4Service[i].dwPdOffsIn  = -1;
            m_oSlaveInfo4Service[i].dwPdOffsOut = -1;
            oLocalIndexEK1100++;
        }
        else if ((m_oSlaveInfo4Service[i].dwVendorId == VENDORID_BECKHOFF) && (m_oSlaveInfo4Service[i].dwProductCode == PRODUCTCODE_EK1310))
        {
            m_oSlaveInfo4Service[i].dwPdOffsIn  = EK1310_Input_Offset[oLocalIndexEK1310] * 8; // Wert ist Bit-Position
            m_oSlaveInfo4Service[i].dwPdOffsOut = -1;
            oLocalIndexEK1310++;
        }
        else if ((m_oSlaveInfo4Service[i].dwVendorId == VENDORID_PRECITEC) && (m_oSlaveInfo4Service[i].dwProductCode == PRODUCTCODE_FRONTEND))
        {
            m_oSlaveInfo4Service[i].dwPdOffsIn  = FRONTEND_Input_Offset[oLocalIndexFRONTEND] * 8; // Wert ist Bit-Position
            m_oSlaveInfo4Service[i].dwPdOffsOut = FRONTEND_Output_Offset[oLocalIndexFRONTEND] * 8; // Wert ist Bit-Position
            oLocalIndexFRONTEND++;
        }
	    else
	    {
	    	sprintf(oDebugStrg, "(0x%08X,0x%08X)",
	    			m_oSlaveInfo4Service[i].dwVendorId, m_oSlaveInfo4Service[i].dwProductCode);
            wmLogTr(eError, "QnxMsg.VI.ECMNoFittingSlave", "No fitting combination of VENDOR_ID and PRODUCT_CODE found ! %s\n", oDebugStrg);
            continue;
	    }

        memcpy(&m_oSlaveInfoArray[i], &m_oSlaveInfo4Service[i], sizeof(EC_T_GET_SLAVE_INFO));
    }

    for(unsigned int i = 0;i < m_oSlaveNumbers.m_oNbr_EL1018;i++)
    {
        sprintf(oDebugStrg, "EL1018_Input_Offset[%d]:     %u", i, EL1018_Input_Offset[i]);
        wmLog(eDebug, "%s\n", oDebugStrg);
    }
    for(unsigned int i = 0;i < m_oSlaveNumbers.m_oNbr_EL2008;i++)
    {
        sprintf(oDebugStrg, "EL2008_Output_Offset[%d]:    %u", i, EL2008_Output_Offset[i]);
        wmLog(eDebug, "%s\n", oDebugStrg);
    }
    for(unsigned int i = 0;i < m_oSlaveNumbers.m_oNbr_EL3102;i++)
    {
        sprintf(oDebugStrg, "EL3102_Input_Offset[%d]:     %u", i, EL3102_Input_Offset[i]);
        wmLog(eDebug, "%s\n", oDebugStrg);
    }
    for(unsigned int i = 0;i < m_oSlaveNumbers.m_oNbr_EL3702;i++)
    {
        sprintf(oDebugStrg, "EL3702_CH1_Input_Offset[%d]: %u", i, EL3702_CH1_Input_Offset[i]);
        wmLog(eDebug, "%s\n", oDebugStrg);
        sprintf(oDebugStrg, "EL3702_CH2_Input_Offset[%d]: %u", i, EL3702_CH2_Input_Offset[i]);
        wmLog(eDebug, "%s\n", oDebugStrg);
        sprintf(oDebugStrg, "EL3702_Add_Input_Offset[%d]: %u", i, EL3702_Add_Input_Offset[i]);
        wmLog(eDebug, "%s\n", oDebugStrg);
    }
    for(unsigned int i = 0;i < m_oSlaveNumbers.m_oNbr_EL4102;i++)
    {
        sprintf(oDebugStrg, "EL4102_Output_Offset[%d]:    %u", i, EL4102_Output_Offset[i]);
        wmLog(eDebug, "%s\n", oDebugStrg);
    }
    for(unsigned int i = 0;i < m_oSlaveNumbers.m_oNbr_EL4132;i++)
    {
        sprintf(oDebugStrg, "EL4132_Output_Offset[%d]:    %u", i, EL4132_Output_Offset[i]);
        wmLog(eDebug, "%s\n", oDebugStrg);
    }
    for(unsigned int i = 0;i < m_oSlaveNumbers.m_oNbr_EL5101;i++)
    {
        sprintf(oDebugStrg, "EL5101_Input_Offset[%d]:     %u", i, EL5101_Input_Offset[i]);
        wmLog(eDebug, "%s\n", oDebugStrg);
        sprintf(oDebugStrg, "EL5101_Output_Offset[%d]:    %u", i, EL5101_Output_Offset[i]);
        wmLog(eDebug, "%s\n", oDebugStrg);
    }
    for (unsigned int i = 0; i < m_oSlaveNumbers.m_numberOfEL5151; i++)
    {
        sprintf(oDebugStrg, "EL5151InputOffset[%d]:       %u", i, EL5151InputOffset[i]);
        wmLog(eDebug, "%s\n", oDebugStrg);
        sprintf(oDebugStrg, "EL5151OutputOffset[%d]:      %u", i, EL5151OutputOffset[i]);
        wmLog(eDebug, "%s\n", oDebugStrg);
    }
    for(unsigned int i = 0;i < m_oSlaveNumbers.m_oNbr_GATEWAY;i++)
    {
        sprintf(oDebugStrg, "GATEWAY_Input_Offset[%d]:    %u", i, GATEWAY_Input_Offset[i]);
        wmLog(eDebug, "%s\n", oDebugStrg);
        sprintf(oDebugStrg, "GATEWAY_Output_Offset[%d]:   %u", i, GATEWAY_Output_Offset[i]);
        wmLog(eDebug, "%s\n", oDebugStrg);
    }
    for(unsigned int i = 0;i < m_oSlaveNumbers.m_oNbr_ACCELNET;i++)
    {
        sprintf(oDebugStrg, "ACCELNET_Input_Offset[%d]:   %u", i, ACCELNET_Input_Offset[i]);
        wmLog(eDebug, "%s\n", oDebugStrg);
        sprintf(oDebugStrg, "ACCELNET_Output_Offset[%d]:  %u", i, ACCELNET_Output_Offset[i]);
        wmLog(eDebug, "%s\n", oDebugStrg);
    }
    for(unsigned int i = 0;i < m_oSlaveNumbers.m_oNbr_EPOS4;i++)
    {
        sprintf(oDebugStrg, "EPOS4_Input_Offset[%d]:   %u", i, EPOS4_Input_Offset[i]);
        wmLog(eDebug, "%s\n", oDebugStrg);
        sprintf(oDebugStrg, "EPOS4_Output_Offset[%d]:  %u", i, EPOS4_Output_Offset[i]);
        wmLog(eDebug, "%s\n", oDebugStrg);
        sprintf(oDebugStrg, "m_oEPOS4SlavePosition[%d]:%u", i, m_oEPOS4SlavePosition[i]);
        wmLog(eDebug, "%s\n", oDebugStrg);
    }
    for(unsigned int i = 0;i < m_oSlaveNumbers.m_oNbr_EK1310;i++)
    {
        sprintf(oDebugStrg, "EK1310_Input_Offset[%d]:     %u", i, EK1310_Input_Offset[i]);
        wmLog(eDebug, "%s\n", oDebugStrg);
    }
    for(unsigned int i = 0;i < m_oSlaveNumbers.m_oNbr_FRONTEND;i++)
    {
        sprintf(oDebugStrg, "FRONTEND_Input_Offset[%d]:    %u", i, FRONTEND_Input_Offset[i]);
        wmLog(eDebug, "%s\n", oDebugStrg);
        sprintf(oDebugStrg, "FRONTEND_Output_Offset[%d]:   %u", i, FRONTEND_Output_Offset[i]);
        wmLog(eDebug, "%s\n", oDebugStrg);
    }

    for(unsigned int i = 0;i < m_oSlaveNumbers.m_oNbr_EPOS4;i++)
    {
        SDOConfigurationEPOS4(i);
    }

    wmLogTr(eInfo, "QnxMsg.VI.ECMActivMaster", "Activating EtherCAT Master ...\n");
	if (ecrt_master_activate(m_pEtherCATMaster))
	{
        wmLogTr(eError, "QnxMsg.VI.ECMActMasterFailed", "Cannot activate master !\n");
		return -1;
	}

	if (!(m_pDomain1_pd = ecrt_domain_data(m_pDomain1)))
	{
        wmLogTr(eError, "QnxMsg.VI.ECMDomDataFailed", "Cannot return process data address !\n");
		return -1;
	}

    m_oBytesCountInDomain = ecrt_domain_size(m_pDomain1);
    wmLog(eDebug, "m_oBytesCountInDomain: %d\n", m_oBytesCountInDomain);
    if (m_oBytesCountInDomain < (size_t)0)
    {
        return -1;
    }

	return 0;
}

void EtherCATMaster::StartCycleTaskThread(void)
{
    if (m_pEtherCATMaster == nullptr)
    {
        return;
    }

    if (g_pFirstFRONTENDSlave != nullptr)
    {
        ecrt_master_select_reference_clock(m_pEtherCATMaster, g_pFirstFRONTENDSlave);
    }
    else if (g_pFirstEL3702Slave != nullptr)
    {
        ecrt_master_select_reference_clock(m_pEtherCATMaster, g_pFirstEL3702Slave);
    }

	///////////////////////////////////////////////////////
	// Thread für zyklischen Ablauf starten
	///////////////////////////////////////////////////////
	pthread_attr_t oPthreadAttr;
    pthread_attr_init(&oPthreadAttr);

	m_oDataToECATCycleTaskThread.m_pEtherCATMaster = this;

    if (pthread_create(&m_oECATCycleTaskThread, &oPthreadAttr, &ECATCycleTaskThread, &m_oDataToECATCycleTaskThread) != 0)
    {
        wmFatal(eBusSystem, "QnxMsg.VI.ECMCreateThreadFail", "Cannot start thread for cyclic operation\n");
    }
}

void EtherCATMaster::stopThreads()
{
    m_firstOperationStateReceived = false; // to avoid automatic reactivation of m_ethercatInputsActive
    m_allSlavesOperational = false; // to avoid automatic reactivation of m_ethercatInputsActive
    m_ethercatInputsActive = false;
    m_oSystemReadyState = false;
    usleep(1000 * 1000);
    s_threadsStopped = true;
    pthread_join(m_oCheckProcessesThread, nullptr);
    pthread_join(m_oECATCycleTaskThread, nullptr);
    pthread_join(m_oECATDebugDataThread, nullptr);
}

void EtherCATMaster::ECATCyclicTask(void)
{
	if (!m_pEtherCATMaster)
	{
		return;
	}

    if (!m_ethercatInputsActive)
    {
        if ((m_allSlavesOperational) && (m_firstOperationStateReceived))
        {
            m_ethercatInputsActive = true;
            wmLog(eDebug, "m_ethercatInputsActive = true\n");
        }
    }

    ///////////////////////
    // receive process data
    ///////////////////////
    ecrt_master_receive(m_pEtherCATMaster);
    ecrt_domain_process(m_pDomain1);

	static unsigned int counter = 0;

    uint8_t        oEL2008OutputBuffer[MAX_EL2008_COUNT];
    uint16_t       oEL4102OutputBuffer[MAX_EL4102_COUNT][2]; // channel 1 and channel 2
    uint16_t       oEL4132OutputBuffer[MAX_EL4132_COUNT][2]; // channel 1 and channel 2
    uint32_t       oEL5101OutputBuffer[MAX_EL5101_COUNT][2]; // 16 Bit command and 32 Bit setCounterValue
    uint32_t       EL5151OutputBuffer[MAX_EL5151_COUNT][2]; // 16 Bit command and 32 Bit setCounterValue
    uint8_t        oGatewayOutputBuffer[MAX_GATEWAY_COUNT][MAX_GATEWAY_OUTPUT_LENGTH];
    EcatAxisOutput oACCELNETOutputBuffer[MAX_ACCELNET_COUNT];
    EcatAxisOutput oEPOS4OutputBuffer[MAX_EPOS4_COUNT];
    EcatFRONTENDOutput oFRONTENDOutputBuffer[MAX_FRONTEND_COUNT];

    static uint8_t oErrorReg[MAX_EPOS4_COUNT] {};
    static int32_t oHomeOffset[MAX_EPOS4_COUNT] {};

    static uint16_t oLastCycleCountCH1[MAX_NBR_PER_SLAVE] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
#if ECAT_TEST_JUMP_BETW_CYCLES
    static int16_t  oLastValueCH1[MAX_NBR_PER_SLAVE]      = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
#endif
    static uint16_t oLastCycleCountCH2[MAX_NBR_PER_SLAVE] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
#if ECAT_TEST_JUMP_BETW_CYCLES
    static int16_t  oLastValueCH2[MAX_NBR_PER_SLAVE]      = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
#endif

    struct timespec oSystemTime;
    static uint64_t oSystemTimeInNs = 0;

#if ECAT_CYCLE_TIMING_PRINTOUTS
    struct timespec oStartTime;
    struct timespec oEndTime;
    static struct timespec oLastStartTime = {};
    uint64_t oStartToStart;
    static uint64_t oMinStartToStart = 9999999999;
    static uint64_t oMaxStartToStart = 0;
    uint64_t oStartToEnd;
    static uint64_t oMinStartToEnd = 9999999999;
    static uint64_t oMaxStartToEnd = 0;
    static uint32_t oDebugLoop = 0;

    clock_gettime(CLOCK_TO_USE, &oStartTime);
    oStartToStart = DIFF_NS(oLastStartTime, oStartTime);
    if (oDebugLoop != 0)
    {
        if (oStartToStart < oMinStartToStart)
        {
            oMinStartToStart = oStartToStart;
        }
        if (oStartToStart > oMaxStartToStart)
        {
            oMaxStartToStart = oStartToStart;
        }
    }
    oLastStartTime = oStartTime;
#endif


	// local copy of OutputBuffers, avoids overwriting by incoming interface functions
    pthread_mutex_lock(&oEL2008OutLock);
    memcpy(oEL2008OutputBuffer,   m_oEL2008OutputBuffer,   sizeof(oEL2008OutputBuffer));
    pthread_mutex_unlock(&oEL2008OutLock);
    pthread_mutex_lock(&oEL4102OutLock);
    memcpy(oEL4102OutputBuffer,   m_oEL4102OutputBuffer,   sizeof(oEL4102OutputBuffer));
    pthread_mutex_unlock(&oEL4102OutLock);
    pthread_mutex_lock(&oEL4132OutLock);
    memcpy(oEL4132OutputBuffer,   m_oEL4132OutputBuffer,   sizeof(oEL4132OutputBuffer));
    pthread_mutex_unlock(&oEL4132OutLock);
    pthread_mutex_lock(&oEL5101OutLock);
    memcpy(oEL5101OutputBuffer,   m_oEL5101OutputBuffer,   sizeof(oEL5101OutputBuffer));
    pthread_mutex_unlock(&oEL5101OutLock);
    pthread_mutex_lock(&m_EL5151OutputLock);
    memcpy(EL5151OutputBuffer, m_EL5151OutputBuffer, sizeof(EL5151OutputBuffer));
    pthread_mutex_unlock(&m_EL5151OutputLock);
    pthread_mutex_lock(&oGatewayOutLock);
    memcpy(oGatewayOutputBuffer,  m_oGatewayOutputBuffer,  sizeof(oGatewayOutputBuffer));
    pthread_mutex_unlock(&oGatewayOutLock);
    pthread_mutex_lock(&oACCELNETOutLock);
    memcpy(oACCELNETOutputBuffer, m_oACCELNETOutputBuffer, sizeof(oACCELNETOutputBuffer));
    pthread_mutex_unlock(&oACCELNETOutLock);
    pthread_mutex_lock(&oEPOS4OutLock);
    memcpy(oEPOS4OutputBuffer, m_oEPOS4OutputBuffer, sizeof(oEPOS4OutputBuffer));
    pthread_mutex_unlock(&oEPOS4OutLock);
    pthread_mutex_lock(&oFRONTENDOutLock);
    memcpy(oFRONTENDOutputBuffer, m_oFRONTENDOutputBuffer, sizeof(oFRONTENDOutputBuffer));
    pthread_mutex_unlock(&oFRONTENDOutLock);


    /////////////////////
    // write process data
    /////////////////////
    for(unsigned int i = 0;i < m_oSlaveNumbers.m_oNbr_EL2008;i++)
    {
       	EC_WRITE_U8(m_pDomain1_pd + EL2008_Output_Offset[i], (uint8_t)oEL2008OutputBuffer[i]);
    }

    for(unsigned int i = 0;i < m_oSlaveNumbers.m_oNbr_EL4102;i++)
    {
       	EC_WRITE_U16(m_pDomain1_pd + EL4102_Output_Offset[i], (uint16_t)oEL4102OutputBuffer[i][0]); // Channel 1
       	EC_WRITE_U16(m_pDomain1_pd + EL4102_Output_Offset[i] + 2, (uint16_t)oEL4102OutputBuffer[i][1]); // Channel 2
    }

    for(unsigned int i = 0;i < m_oSlaveNumbers.m_oNbr_EL4132;i++)
    {
       	EC_WRITE_U16(m_pDomain1_pd + EL4132_Output_Offset[i], (uint16_t)oEL4132OutputBuffer[i][0]); // Channel 1
       	EC_WRITE_U16(m_pDomain1_pd + EL4132_Output_Offset[i] + 2, (uint16_t)oEL4132OutputBuffer[i][1]); // Channel 2
    }

    if (!m_oFieldbusViaSeparateFieldbusBoard)
    {
        for(unsigned int i = 0;i < m_oSlaveNumbers.m_oNbr_GATEWAY;i++)
        {
            for(unsigned int j = 0;j < GATEWAY1_DATA_LENGHT;j++)
            {
                EC_WRITE_U8(m_pDomain1_pd + GATEWAY_Output_Offset[i] + j, (uint8_t)oGatewayOutputBuffer[i][j]);
            }
        }
    }

    for(unsigned int i = 0;i < m_oSlaveNumbers.m_oNbr_EL5101;i++)
    {
       	EC_WRITE_U16(m_pDomain1_pd + EL5101_Output_Offset[i], (uint16_t)oEL5101OutputBuffer[i][0]); // command
       	EC_WRITE_U32(m_pDomain1_pd + EL5101_Output_Offset[i] + 2, (uint16_t)oEL5101OutputBuffer[i][1]); // setCounterValue
    }

    for (unsigned int i = 0; i < m_oSlaveNumbers.m_numberOfEL5151; i++)
    {
        EC_WRITE_U16(m_pDomain1_pd + EL5151OutputOffset[i], (uint16_t)EL5151OutputBuffer[i][0]); // command
        EC_WRITE_U32(m_pDomain1_pd + EL5151OutputOffset[i] + 2, (uint16_t)EL5151OutputBuffer[i][1]); // setCounterValue
    }

    for(unsigned int i = 0;i < m_oSlaveNumbers.m_oNbr_ACCELNET;i++)
    {
       	EC_WRITE_U16(m_pDomain1_pd + ACCELNET_Output_Offset[i] + 0,  (uint16_t)oACCELNETOutputBuffer[i].controlWord); // command
       	EC_WRITE_U8 (m_pDomain1_pd + ACCELNET_Output_Offset[i] + 2,   (uint8_t)oACCELNETOutputBuffer[i].modesOfOp); // command
       	EC_WRITE_S32(m_pDomain1_pd + ACCELNET_Output_Offset[i] + 3,   (int32_t)oACCELNETOutputBuffer[i].profileTargetPosition); // command
       	EC_WRITE_S32(m_pDomain1_pd + ACCELNET_Output_Offset[i] + 7,   (int32_t)oACCELNETOutputBuffer[i].profileVelocity); // command
       	EC_WRITE_S32(m_pDomain1_pd + ACCELNET_Output_Offset[i] + 11,  (int32_t)oACCELNETOutputBuffer[i].profileAcceleration); // command
       	EC_WRITE_S32(m_pDomain1_pd + ACCELNET_Output_Offset[i] + 15,  (int32_t)oACCELNETOutputBuffer[i].profileDeceleration); // command
       	EC_WRITE_U8 (m_pDomain1_pd + ACCELNET_Output_Offset[i] + 19,  (uint8_t)oACCELNETOutputBuffer[i].homingMethod); // command
       	EC_WRITE_S32(m_pDomain1_pd + ACCELNET_Output_Offset[i] + 20,  (int32_t)oACCELNETOutputBuffer[i].homingOffset); // command
       	EC_WRITE_S32(m_pDomain1_pd + ACCELNET_Output_Offset[i] + 24,  (int32_t)oACCELNETOutputBuffer[i].homingVelocityFast); // command
       	EC_WRITE_S32(m_pDomain1_pd + ACCELNET_Output_Offset[i] + 28,  (int32_t)oACCELNETOutputBuffer[i].homingVelocitySlow); // command
    }

    for(unsigned int i = 0;i < m_oSlaveNumbers.m_oNbr_EPOS4;i++)
    {
        EC_WRITE_U16(m_pDomain1_pd + EPOS4_Output_Offset[i] + 0,  (uint16_t)oEPOS4OutputBuffer[i].controlWord); // command
        EC_WRITE_U8 (m_pDomain1_pd + EPOS4_Output_Offset[i] + 2,   (uint8_t)oEPOS4OutputBuffer[i].modesOfOp); // command
        EC_WRITE_S32(m_pDomain1_pd + EPOS4_Output_Offset[i] + 3,   (int32_t)oEPOS4OutputBuffer[i].profileTargetPosition); // command
        EC_WRITE_S32(m_pDomain1_pd + EPOS4_Output_Offset[i] + 7,   (int32_t)oEPOS4OutputBuffer[i].profileVelocity); // command
        EC_WRITE_S32(m_pDomain1_pd + EPOS4_Output_Offset[i] + 11,  (int32_t)oEPOS4OutputBuffer[i].profileAcceleration); // command
        EC_WRITE_S32(m_pDomain1_pd + EPOS4_Output_Offset[i] + 15,  (int32_t)oEPOS4OutputBuffer[i].profileDeceleration); // command
        EC_WRITE_U8 (m_pDomain1_pd + EPOS4_Output_Offset[i] + 19,  (uint8_t)oEPOS4OutputBuffer[i].homingMethod); // command
        oHomeOffset[i] = oEPOS4OutputBuffer[i].homingOffset; // homingOffset is written via SDO access
        EC_WRITE_S32(m_pDomain1_pd + EPOS4_Output_Offset[i] + 20,  (int32_t)oEPOS4OutputBuffer[i].homingVelocityFast); // command
        EC_WRITE_S32(m_pDomain1_pd + EPOS4_Output_Offset[i] + 24,  (int32_t)oEPOS4OutputBuffer[i].homingVelocitySlow); // command
    }

    for(unsigned int i = 0;i < m_oSlaveNumbers.m_oNbr_FRONTEND;i++)
    {
        EC_WRITE_U16(m_pDomain1_pd + FRONTEND_Output_Offset[i] + 0, (uint16_t)m_oFRONTENDOutputBuffer[i].m_oAmpCH1); // Amp CH1
        EC_WRITE_U16(m_pDomain1_pd + FRONTEND_Output_Offset[i] + 2, (uint16_t)m_oFRONTENDOutputBuffer[i].m_oAmpCH2); // Amp CH2
        EC_WRITE_U16(m_pDomain1_pd + FRONTEND_Output_Offset[i] + 4, (uint16_t)m_oFRONTENDOutputBuffer[i].m_oAmpCH3); // Amp CH3
        EC_WRITE_U16(m_pDomain1_pd + FRONTEND_Output_Offset[i] + 6, (uint16_t)m_oFRONTENDOutputBuffer[i].m_oAmpCH4); // Amp CH4, not used
        EC_WRITE_U16(m_pDomain1_pd + FRONTEND_Output_Offset[i] + 8, (uint16_t)FRONTEND_OVERSAMP_FACTOR); // Oversampling
    }

    pthread_mutex_lock(&m_oCheckProcessesMutex);
    // insert System ready Information
    if (m_oSystemReadyOffset != -1)
    {
        unsigned char oHelpUChar = *((m_pDomain1_pd) + m_oSystemReadyOffset);
        if (!m_oSystemReadyState)
        {
            oHelpUChar &= ~m_oSystemReadyMask;
        }
        *((m_pDomain1_pd) + m_oSystemReadyOffset) = oHelpUChar;
    }
    if (m_oSystemReadyOffsetFull != -1)
    {
        unsigned char oHelpUChar = *((m_pDomain1_pd) + m_oSystemReadyOffsetFull);
        if (!m_oSystemReadyState)
        {
            oHelpUChar &= ~m_oSystemReadyMaskFull;
        }
        *((m_pDomain1_pd) + m_oSystemReadyOffsetFull) = oHelpUChar;
    }
    // insert System Error Information
    if (m_oSystemErrorFieldOffset != -1)
    {
        unsigned char oHelpUChar1 = *((m_pDomain1_pd) + m_oSystemErrorFieldOffset);
        unsigned char oHelpUChar2 = *((m_pDomain1_pd) + m_oSystemErrorFieldOffset + 1);
        if (m_oSystemErrorFieldValue != 0)
        {
            oHelpUChar1 |= m_oSystemErrorFieldMask1;
            oHelpUChar2 |= m_oSystemErrorFieldMask2;
        }
        *((m_pDomain1_pd) + m_oSystemErrorFieldOffset) = oHelpUChar1;
        *((m_pDomain1_pd) + m_oSystemErrorFieldOffset + 1) = oHelpUChar2;
    }
    if (m_oSystemErrorFieldOffsetFull != -1)
    {
        unsigned char oHelpUChar1 = *((m_pDomain1_pd) + m_oSystemErrorFieldOffsetFull);
        unsigned char oHelpUChar2 = *((m_pDomain1_pd) + m_oSystemErrorFieldOffsetFull + 1);
        if (m_oSystemErrorFieldValue != 0)
        {
            oHelpUChar1 |= m_oSystemErrorFieldMask1Full;
            oHelpUChar2 |= m_oSystemErrorFieldMask2Full;
        }
        *((m_pDomain1_pd) + m_oSystemErrorFieldOffsetFull) = oHelpUChar1;
        *((m_pDomain1_pd) + m_oSystemErrorFieldOffsetFull + 1) = oHelpUChar2;
    }
    pthread_mutex_unlock(&m_oCheckProcessesMutex);

    ecrt_domain_queue(m_pDomain1);
    // do the work for Distributed Clocks
    clock_gettime(CLOCK_TO_USE, &oSystemTime);
    oSystemTimeInNs = TIMESPEC2NS(oSystemTime);
    ecrt_master_application_time(m_pEtherCATMaster, oSystemTimeInNs);
#if WITH_SYNC_REF_COUNTER
    if (g_oSyncRefCounter)
    {
        g_oSyncRefCounter--;
    }
    else
    {
        g_oSyncRefCounter = 1;
        ecrt_master_sync_reference_clock(m_pEtherCATMaster);
    }
#else
    ecrt_master_sync_reference_clock(m_pEtherCATMaster);
#endif
    ecrt_master_sync_slave_clocks(m_pEtherCATMaster);

    ////////////////////
    // send process data
    ////////////////////
    ecrt_master_send(m_pEtherCATMaster);

    ////////////////////
    // read process data
    ////////////////////
    EtherCAT::EcatInData dataToProcesses;
    // EL1018
    dataToProcesses.digitalIn.reserve(m_oSlaveNumbers.m_oNbr_EL1018);
    for(unsigned int i = 0;i < m_oSlaveNumbers.m_oNbr_EL1018;i++)
    {
        dataToProcesses.digitalIn.emplace_back(eProductIndex_EL1018, static_cast<EcatInstance>(i + 1), uint8_t(EC_READ_U8(m_pDomain1_pd + EL1018_Input_Offset[i])));
    }

    // EL3102
    dataToProcesses.analogIn.reserve(m_oSlaveNumbers.m_oNbr_EL3102);
    for(unsigned int i = 0;i < m_oSlaveNumbers.m_oNbr_EL3102;i++)
    {
		uint8_t  statusCH1 = EC_READ_U8 (m_pDomain1_pd + EL3102_Input_Offset[i] + 0);
		uint16_t valueCH1  = EC_READ_U16(m_pDomain1_pd + EL3102_Input_Offset[i] + 1);
		uint8_t  statusCH2 = EC_READ_U8 (m_pDomain1_pd + EL3102_Input_Offset[i] + 3);
		uint16_t valueCH2  = EC_READ_U16(m_pDomain1_pd + EL3102_Input_Offset[i] + 4);
        dataToProcesses.analogIn.emplace_back(eProductIndex_EL3102, static_cast<EcatInstance>(i + 1), statusCH1, valueCH1, statusCH2, valueCH2);
    }

    // EL3702
    dataToProcesses.oversampling.reserve(m_oSlaveNumbers.m_oNbr_EL3702);
    for(unsigned int i = 0;i < m_oSlaveNumbers.m_oNbr_EL3702;i++)
    {
        auto &oversampling = dataToProcesses.oversampling.emplace_back(eProductIndex_EL3702, static_cast<EcatInstance>(i + 1));
        oversampling.channel1.reserve(EL3702_OVERSAMP_FACTOR);
        oversampling.channel2.reserve(EL3702_OVERSAMP_FACTOR);
        uint16_t oCycleCountCH1 = EC_READ_U16(m_pDomain1_pd + EL3702_CH1_Input_Offset[i] + 0);
        uint16_t oCycleCountCH2 = EC_READ_U16(m_pDomain1_pd + EL3702_CH2_Input_Offset[i] + 0);
#if ECAT_WITH_DEBUG_DATA_THREAD
        uint32_t oNextSync = EC_READ_U32(m_pDomain1_pd + EL3702_Add_Input_Offset[i] + 0);
#endif

        // check if the CycleCount value of channel 1 is consecutive
        if (oCycleCountCH1 == oLastCycleCountCH1[i])
        {
            // cycle count has not changed, discard the values !
        }
        else
        {
            if ((oCycleCountCH1 != (oLastCycleCountCH1[i] + 1)) && (oLastCycleCountCH1[i] != 0xFFFF))
            {
                // there is a problem in the sequence of oCycleCountCH1
                char oTimeStampStrg[81];
                char oLogString[121];
                sprintf(oLogString, "%s: %u/1 (%d,%d)", InsertTimeStamp(oTimeStampStrg), i, oCycleCountCH1, oLastCycleCountCH1[i]);
                wmLog(eDebug, "%s\n", oLogString);
            }
            oLastCycleCountCH1[i] = oCycleCountCH1;

            // read data of channel 1
            for(unsigned int j = 1;j < (EL3702_OVERSAMP_FACTOR + 1);j++) // all values without cycleCount
            {
                oversampling.channel1.emplace_back(int16_t(EC_READ_S16(m_pDomain1_pd + EL3702_CH1_Input_Offset[i] + (j * 2))));
            }

#if ECAT_TEST_JUMP_BETW_CYCLES
            // check if 2 consecutive cycles have a big jump in voltage
            int16_t oWarningLimit = 330;
            int16_t oDummy = (100 / EL3702_OVERSAMP_FACTOR);
            oWarningLimit *= oDummy;
            if (abs(oversampling.channel1[0] - oLastValueCH1[i]) > oWarningLimit)
            {
                char oTimeStampStrg[81];
                printf("%s: %u/1 x (%d ,%d)\n", InsertTimeStamp(oTimeStampStrg), i, oversampling.channel1[0], oLastValueCH1[i]);
            }
            oLastValueCH1[i] = oversampling.channel1[EL3702_OVERSAMP_FACTOR - 1];
#endif

#if ECAT_WITH_DEBUG_DATA_THREAD
            // log a certain amount of input values for debugging
            if (i == 0) // Debug logging nur beim ersten Slave
            {
                static unsigned long oCounter = 0;
                oCounter++;
                if (oCounter > 15000) // wait some time before start logging
                {
                    if (m_oWriteIndex == 0)
                    {
                        printf("Start\n");
                    }
                    if (m_oWriteIndex < ANZ_BUFFER)
                    {
                        m_oOversampDebugBufferCycCnt[m_oWriteIndex] = oCycleCountCH1;
#if ECAT_CYCLE_TIMING_PRINTOUTS
                        m_oOversampDebugBufferTime1[m_oWriteIndex] = oStartToStart;
#else
                        m_oOversampDebugBufferTime1[m_oWriteIndex] = oNextSync;
#endif
                        m_oOversampDebugBufferTime2[m_oWriteIndex] = oSystemTimeInNs;
                        for(int j = 0;j < EL3702_OVERSAMP_FACTOR;j++)
                        {
                            m_oOversampDebugBuffer[m_oWriteIndex][j] = oversampling.channel1[j];
                        }
                        m_oWriteIndex++;
                        if (m_oWriteIndex >= ANZ_BUFFER)
                        {
                            printf("Full\n");
                            m_oDebugFileFull = true;
                        }
                    }
                } // wait some time before start logging
            } // Debug logging nur beim ersten Slave
#endif
        } // oCycleCountCH1 != oLastCycleCountCH1[i]

        // check if the CycleCount value of channel 2 is consecutive
        if (oCycleCountCH2 == oLastCycleCountCH2[i])
        {
            // cycle count has not changed, discard the values !
        }
        else
        {
            if ((oCycleCountCH2 != (oLastCycleCountCH2[i] + 1)) && (oLastCycleCountCH2[i] != 0xFFFF))
            {
                // there is a problem in the sequence of oCycleCountCH2
                char oTimeStampStrg[81];
                char oLogString[121];
                sprintf(oLogString, "%s: %u/2 (%d,%d)", InsertTimeStamp(oTimeStampStrg), i, oCycleCountCH2, oLastCycleCountCH2[i]);
                wmLog(eDebug, "%s\n", oLogString);
            }
            oLastCycleCountCH2[i] = oCycleCountCH2;

            // read data of channel 2
            for(unsigned int j = 1;j < (EL3702_OVERSAMP_FACTOR + 1);j++) // all values without cycleCount
            {
                oversampling.channel2.emplace_back(int16_t(EC_READ_S16(m_pDomain1_pd + EL3702_CH2_Input_Offset[i] + (j * 2))));
            }

#if ECAT_TEST_JUMP_BETW_CYCLES
            // check if 2 consecutive cycles have a big jump in voltage
            int16_t oWarningLimit = 330;
            int16_t oDummy = (100 / EL3702_OVERSAMP_FACTOR);
            oWarningLimit *= oDummy;
            if (abs(oversampling.channel2[0] - oLastValueCH2[i]) > oWarningLimit)
            {
                char oTimeStampStrg[81];
                printf("%s: %u/2 x (%d ,%d)\n", InsertTimeStamp(oTimeStampStrg), i, oversampling.channel2[0], oLastValueCH2[i]);
            }
            oLastValueCH2[i] = oversampling.channel2[EL3702_OVERSAMP_FACTOR - 1];
#endif
        } // oCycleCountCH2 != oLastCycleCountCH2[i]

#if ECAT_DEBUG_OUTPUTS
        if (i == 0) // Debugausgabe nur beim ersten Slave
        {
            int16_t valueCH1_0, valueCH1_1, valueCH1_2, valueCH1_3, valueCH1_4;
            int16_t valueCH2_0, valueCH2_1, valueCH2_2, valueCH2_3, valueCH2_4;

            valueCH1_0    = EC_READ_S16(m_pDomain1_pd + EL3702_CH1_Input_Offset[i] + 2);
            valueCH1_1    = EC_READ_S16(m_pDomain1_pd + EL3702_CH1_Input_Offset[i] + 4);
            valueCH1_2    = EC_READ_S16(m_pDomain1_pd + EL3702_CH1_Input_Offset[i] + 6);
            valueCH1_3    = EC_READ_S16(m_pDomain1_pd + EL3702_CH1_Input_Offset[i] + 198);
            valueCH1_4    = EC_READ_S16(m_pDomain1_pd + EL3702_CH1_Input_Offset[i] + 200);

            valueCH2_0    = EC_READ_S16(m_pDomain1_pd + EL3702_CH2_Input_Offset[i] + 2);
            valueCH2_1    = EC_READ_S16(m_pDomain1_pd + EL3702_CH2_Input_Offset[i] + 4);
            valueCH2_2    = EC_READ_S16(m_pDomain1_pd + EL3702_CH2_Input_Offset[i] + 6);
            valueCH2_3    = EC_READ_S16(m_pDomain1_pd + EL3702_CH2_Input_Offset[i] + 198);
            valueCH2_4    = EC_READ_S16(m_pDomain1_pd + EL3702_CH2_Input_Offset[i] + 200);

            static int oLoop = 0;
            oLoop++;
            if (oLoop >= 200)
            {
                printf("EL3702: %6u, %6d, %6d, %6d, %6d, %6d | %6u, %6d, %6d, %6d, %6d, %6d, %6u\n",
                       oCycleCountCH1, valueCH1_0, valueCH1_1, valueCH1_2, valueCH1_3, valueCH1_4,
                       oCycleCountCH2, valueCH2_0, valueCH2_1, valueCH2_2, valueCH2_3, valueCH2_4,
                       nextSync1);
                oLoop = 0;
            }            
        }
#endif
    }

    // Gateway (unabhaengig ob Anybus oder Kunbus)
    if (!m_oFieldbusViaSeparateFieldbusBoard)
    {
        dataToProcesses.gateway.reserve(m_oSlaveNumbers.m_oNbr_GATEWAY);
        for(unsigned int i = 0;i < m_oSlaveNumbers.m_oNbr_GATEWAY;i++)
        {
            auto &gateway = dataToProcesses.gateway.emplace_back(eProductIndex_Anybus_GW, static_cast<EcatInstance>(i + 1));
            auto &oTempVec = gateway.data;
            oTempVec.reserve(GATEWAY1_DATA_LENGHT);
// TODO     Gateway Laenge fuer jedes Gateway einzeln beruecksichtigen
            for(unsigned int j = 0;j < GATEWAY1_DATA_LENGHT;j++)
            {
                oTempVec.push_back(uint8_t(EC_READ_U8(m_pDomain1_pd + GATEWAY_Input_Offset[i] + j)));
            }
        }
    }

    // EL5101
    dataToProcesses.encoder.reserve(m_oSlaveNumbers.m_oNbr_EL5101);
    for(unsigned int i = 0;i < m_oSlaveNumbers.m_oNbr_EL5101;i++)
    {
		uint16_t status        = EC_READ_U16(m_pDomain1_pd + EL5101_Input_Offset[i] + 0);
		uint32_t counterValue  = EC_READ_U32(m_pDomain1_pd + EL5101_Input_Offset[i] + 2);
		uint32_t latchValue    = EC_READ_U32(m_pDomain1_pd + EL5101_Input_Offset[i] + 6);
        dataToProcesses.encoder.emplace_back(eProductIndex_EL5101, static_cast<EcatInstance>(i + 1), status, counterValue, latchValue);
    }

    // EL5151
    dataToProcesses.encoder.reserve(m_oSlaveNumbers.m_numberOfEL5151);
    for (unsigned int i = 0; i < m_oSlaveNumbers.m_numberOfEL5151; i++)
    {
        uint16_t status        = EC_READ_U16(m_pDomain1_pd + EL5151InputOffset[i] + 0);
        uint32_t counterValue  = EC_READ_U32(m_pDomain1_pd + EL5151InputOffset[i] + 2);
        uint32_t latchValue    = EC_READ_U32(m_pDomain1_pd + EL5151InputOffset[i] + 6);
        dataToProcesses.encoder.emplace_back(eProductIndex_EL5151, static_cast<EcatInstance>(i + 1), status, counterValue, latchValue);
    }


    // ACCELNET
    dataToProcesses.axis.reserve(m_oSlaveNumbers.m_oNbr_ACCELNET + m_oSlaveNumbers.m_oNbr_EPOS4);
    for(unsigned int i = 0;i < m_oSlaveNumbers.m_oNbr_ACCELNET;i++)
    {
        auto &axis = dataToProcesses.axis.emplace_back(eProductIndex_ACCELNET, static_cast<EcatInstance>(i + 1));
        EcatAxisInput &axisInput = axis.axis;
        axisInput.statusWord        = EC_READ_U16(m_pDomain1_pd + ACCELNET_Input_Offset[i] + 0);
        axisInput.modesOfOpDisp     = EC_READ_U8 (m_pDomain1_pd + ACCELNET_Input_Offset[i] + 2);
        axisInput.errorReg          = EC_READ_U8 (m_pDomain1_pd + ACCELNET_Input_Offset[i] + 3);
        axisInput.manufacStatus     = EC_READ_U32(m_pDomain1_pd + ACCELNET_Input_Offset[i] + 4);
        axisInput.actualPosition    = EC_READ_U32(m_pDomain1_pd + ACCELNET_Input_Offset[i] + 8);
        axisInput.actualVelocity    = EC_READ_U32(m_pDomain1_pd + ACCELNET_Input_Offset[i] + 12);
        axisInput.actualTorque      = EC_READ_U16(m_pDomain1_pd + ACCELNET_Input_Offset[i] + 16);
        axisInput.m_oDigitalInputs  = 0;
        axisInput.m_oDigitalOutputs = 0;
        axisInput.m_oFollowingError = 0;
    }

    // EPOS4
    for(unsigned int i = 0;i < m_oSlaveNumbers.m_oNbr_EPOS4;i++)
    {
        auto &axis = dataToProcesses.axis.emplace_back(eProductIndex_EPOS4, static_cast<EcatInstance>(i + 1));
        auto &axisInput = axis.axis;
        axisInput.statusWord        = EC_READ_U16(m_pDomain1_pd + EPOS4_Input_Offset[i] + 0);
        axisInput.modesOfOpDisp     = EC_READ_U8 (m_pDomain1_pd + EPOS4_Input_Offset[i] + 2);
        axisInput.errorReg          = oErrorReg[i]; // read via SDO read access
        axisInput.manufacStatus     = (uint32_t)EC_READ_U16(m_pDomain1_pd + EPOS4_Input_Offset[i] + 3);
        axisInput.actualPosition    = EC_READ_U32(m_pDomain1_pd + EPOS4_Input_Offset[i] + 5);
        axisInput.actualVelocity    = EC_READ_U32(m_pDomain1_pd + EPOS4_Input_Offset[i] + 9);
        axisInput.actualTorque      = EC_READ_U16(m_pDomain1_pd + EPOS4_Input_Offset[i] + 13);
        axisInput.m_oDigitalInputs  = EC_READ_U16(m_pDomain1_pd + EPOS4_Input_Offset[i] + 15);
        axisInput.m_oDigitalOutputs = EC_READ_U16(m_pDomain1_pd + EPOS4_Input_Offset[i] + 17);
        axisInput.m_oFollowingError = EC_READ_S32(m_pDomain1_pd + EPOS4_Input_Offset[i] + 19);
    }

    // EK1310
    for(unsigned int i = 0;i < m_oSlaveNumbers.m_oNbr_EK1310;i++)
    {
        // currently variable not used
        //uint8_t oVoltageError = EC_READ_U8(m_pDomain1_pd + EK1310_Input_Offset[i]);
    }

    // FRONTEND LWM40
    dataToProcesses.lwm.reserve(m_oSlaveNumbers.m_oNbr_FRONTEND);
    for(unsigned int i = 0;i < m_oSlaveNumbers.m_oNbr_FRONTEND;i++)
    {
        auto &lwm = dataToProcesses.lwm.emplace_back(eProductIndex_Frontend, static_cast<EcatInstance>(i + 1));
        lwm.plasma.reserve(FRONTEND_OVERSAMP_FACTOR);
        lwm.temperature.reserve(FRONTEND_OVERSAMP_FACTOR);
        lwm.backReference.reserve(FRONTEND_OVERSAMP_FACTOR);
        lwm.analog.reserve(FRONTEND_OVERSAMP_FACTOR);
        // currently variable not used
        //uint16_t oFRONTENDError = uint16_t(EC_READ_U16(m_pDomain1_pd + FRONTEND_Input_Offset[i] + 0));

        // read data of channel 1
        unsigned int oChannelNo = 1;
        unsigned int oStartOffset = FRONTEND_OVERSAMP_FACTOR * (oChannelNo - 1) * 2;
        for(unsigned int j = 1;j < (FRONTEND_OVERSAMP_FACTOR + 1);j++) // all values without error variable
        {
            lwm.plasma.emplace_back(uint16_t(EC_READ_U16(m_pDomain1_pd + FRONTEND_Input_Offset[i] + oStartOffset + (j * 2))));
        }

        // read data of channel 2
        oChannelNo = 2;
        oStartOffset = FRONTEND_OVERSAMP_FACTOR * (oChannelNo - 1) * 2;
        for(unsigned int j = 1;j < (FRONTEND_OVERSAMP_FACTOR + 1);j++) // all values without error variable
        {
            lwm.temperature.emplace_back(uint16_t(EC_READ_U16(m_pDomain1_pd + FRONTEND_Input_Offset[i] + oStartOffset + (j * 2))));
        }

        // read data of channel 3
        oChannelNo = 3;
        oStartOffset = FRONTEND_OVERSAMP_FACTOR * (oChannelNo - 1) * 2;
        for(unsigned int j = 1;j < (FRONTEND_OVERSAMP_FACTOR + 1);j++) // all values without error variable
        {
            lwm.backReference.emplace_back(uint16_t(EC_READ_U16(m_pDomain1_pd + FRONTEND_Input_Offset[i] + oStartOffset + (j * 2))));
        }

        // read data of channel 4
        oChannelNo = 4;
        oStartOffset = FRONTEND_OVERSAMP_FACTOR * (oChannelNo - 1) * 2;
        for(unsigned int j = 1;j < (FRONTEND_OVERSAMP_FACTOR + 1);j++) // all values without error variable
        {
            lwm.analog.emplace_back(uint16_t(EC_READ_U16(m_pDomain1_pd + FRONTEND_Input_Offset[i] + oStartOffset + (j * 2))));
        }

#if ECAT_WITH_DEBUG_DATA_THREAD
        // log a certain amount of input values for debugging
        if (i == 0) // Debug logging nur beim ersten Slave
        {
            static unsigned long oCounter = 0;
            oCounter++;
            if (oCounter > 15000) // wait some time before start logging
            {
                if (m_oWriteIndex == 0)
                {
                    printf("Start\n");
                }
                if (m_oWriteIndex < ANZ_BUFFER)
                {
                    m_oOversampDebugBufferCycCnt[m_oWriteIndex] = lwm.plasma[5];
                    m_oOversampDebugBufferTime1[m_oWriteIndex] = oStartToStart;
                    m_oOversampDebugBufferTime2[m_oWriteIndex] = oFRONTENDError;
                    for(int j = 0;j < FRONTEND_OVERSAMP_FACTOR;j++)
                    {
                        m_oOversampDebugBuffer[m_oWriteIndex][j] = lwm.analog[j];
                    }
                    m_oWriteIndex++;
                    if (m_oWriteIndex >= ANZ_BUFFER)
                    {
                        printf("Full\n");
                        m_oDebugFileFull = true;
                    }
                }
            } // wait some time before start logging
        } // Debug logging nur beim ersten Slave
#endif
    }

    // send data to processes
    if (m_ethercatInputsActive)
    {
        m_rEthercatInputsProxy.ecatData(dataToProcesses);
    }

    //////////////////////////////////////
    // check process data state (optional)
    //////////////////////////////////////
    check_domain1_state();

    if (counter)
    {
        counter--;
    }
    else
    { // do this at 1 Hz
        counter = 1000; // 1000 * 1ms

        // check for master state (optional)
        check_master_state();

        // check for islave configuration state(s) (optional)
        check_slave_config_states();
    }

    // EPOS4
    for(unsigned int i = 0;i < m_oSlaveNumbers.m_oNbr_EPOS4;i++)
    {
        static int oSDOReadCounter = 50;
        if (oSDOReadCounter)
        {
            oSDOReadCounter--;
        }
        else
        {
            oSDOReadCounter = 50; // 50 * 1ms
            EPOS4_ReadSDO_U8(m_pEPOS4_ErrorRegister_Request[i], oErrorReg[i]);
        }

        static int oSDOWriteCounter = 50;
        if (oSDOWriteCounter)
        {
            oSDOWriteCounter--;
        }
        else
        {
            oSDOWriteCounter = 50; // 50 * 1ms
            EPOS4_WriteSDO_S32(m_pEPOS4_HomeOffset_Request[i], oHomeOffset[i]);
        }
    }

    ///////////////////////////////////////////////
    // send all input data for Service/IO on wmMain
    ///////////////////////////////////////////////
    if (m_sendAllData)
    {
    	stdVecUINT8 oTempVec;
    	for(unsigned int i = 0;i < m_oBytesCountInDomain;i++)
    	{
    		oTempVec.push_back(uint8_t(EC_READ_U8(m_pDomain1_pd + i)));
    	}
        if (m_ethercatInputsActive)
        {
            m_rEthercatInputsToServiceProxy.ecatAllDataIn((uint16_t)oTempVec.size(), oTempVec);
        }
    }

#if ECAT_CYCLE_TIMING_PRINTOUTS
    clock_gettime(CLOCK_TO_USE, &oEndTime);
    oStartToEnd = DIFF_NS(oStartTime, oEndTime);
    if (oStartToEnd < oMinStartToEnd)
    {
        oMinStartToEnd = oStartToEnd;
    }
    if (oStartToEnd > oMaxStartToEnd)
    {
        oMaxStartToEnd = oStartToEnd;
    }

    oDebugLoop++;
    if (oDebugLoop >= 1000)
    {
        printf("%10ld(%6ld), %10ld(%6ld), %10ld, %10ld, %d\n", oMinStartToStart, (CYCLE_TIME_NS - oMinStartToStart), oMaxStartToStart, (oMaxStartToStart - CYCLE_TIME_NS),
               oMinStartToEnd, oMaxStartToEnd, oversampling.channel1[0]);
        oMinStartToStart = 9999999999;
        oMaxStartToStart = 0;
        oMinStartToEnd = 9999999999;
        oMaxStartToEnd = 0;
        oDebugLoop = 0;
    }
#endif
}

void EtherCATMaster::EPOS4_ReadSDO_U8(ec_sdo_request_t *m_pEPOS4_Request, uint8_t &p_oRegister)
{
    switch (ecrt_sdo_request_state(m_pEPOS4_Request))
    {
        case EC_REQUEST_UNUSED: // request was not used yet
printf("EPOS4_ReadSDO_U8: EC_REQUEST_UNUSED\n");
            ecrt_sdo_request_read(m_pEPOS4_Request); // trigger first read
            break;
        case EC_REQUEST_BUSY:
            break;
        case EC_REQUEST_SUCCESS:
            p_oRegister = EC_READ_U8(ecrt_sdo_request_data(m_pEPOS4_Request));
            ecrt_sdo_request_read(m_pEPOS4_Request); // trigger next read
            break;
        case EC_REQUEST_ERROR:
printf("EPOS4_ReadSDO_U8: EC_REQUEST_ERROR\n");
            ecrt_sdo_request_read(m_pEPOS4_Request); // retry reading
            break;
    }
}

void EtherCATMaster::EPOS4_WriteSDO_S32(ec_sdo_request_t *m_pEPOS4_Request, int32_t p_oRegister)
{
    switch (ecrt_sdo_request_state(m_pEPOS4_Request))
    {
        case EC_REQUEST_UNUSED: // request was not used yet
printf("EPOS4_WriteSDO_S32: EC_REQUEST_UNUSED\n");
            ecrt_sdo_request_write(m_pEPOS4_Request); // trigger first read
            break;
        case EC_REQUEST_BUSY:
            break;
        case EC_REQUEST_SUCCESS:
            EC_WRITE_S32(ecrt_sdo_request_data(m_pEPOS4_Request), p_oRegister);
            ecrt_sdo_request_write(m_pEPOS4_Request); // trigger next read
            break;
        case EC_REQUEST_ERROR:
printf("EPOS4_WriteSDO_S32: EC_REQUEST_ERROR\n");
            ecrt_sdo_request_write(m_pEPOS4_Request); // retry reading
            break;
    }
}

void EtherCATMaster::check_master_state(void)
{
    ec_master_state_t oMasterState;

    ecrt_master_state(m_pEtherCATMaster, &oMasterState);

    if (oMasterState.slaves_responding != m_oMasterState.slaves_responding)
    {
        wmLog(eDebug, "MasterState: %d slave(s)\n", oMasterState.slaves_responding);
    }
    if (oMasterState.al_states != m_oMasterState.al_states)
    {
        char oDebugStrg[81];
        sprintf(oDebugStrg, "0x%02X", oMasterState.al_states);
        wmLog(eDebug, "MasterState: AL states: %s\n", oDebugStrg);
    }
    if (oMasterState.link_up != m_oMasterState.link_up)
    {
        wmLog(eDebug, "MasterState: Link is %s\n", oMasterState.link_up ? "up" : "down");
    }

    m_oMasterState = oMasterState;
}

void EtherCATMaster::check_slave_config_states(void)
{
    char oDebugStrg[81];
    ec_slave_config_state_t oLocalConfigState;

    for(auto i = m_oSlaveDirectory.begin();i != m_oSlaveDirectory.end();i++)
    {
        ecrt_slave_config_state(i->m_pSlaveConfig, &oLocalConfigState);
        if (oLocalConfigState.al_state != i->m_oSlaveState.al_state)
        {
            sprintf(oDebugStrg, "0x%08x:0x%08x: State 0x%02X", i->m_oVendorId, i->m_oProductCode, oLocalConfigState.al_state);
            wmLog(eDebug, "%s\n", oDebugStrg);
        }
        if (oLocalConfigState.online != i->m_oSlaveState.online)
        {
            sprintf(oDebugStrg, "0x%08x:0x%08x: %s", i->m_oVendorId, i->m_oProductCode, oLocalConfigState.online ? "online" : "offline");
            wmLog(eDebug, "%s\n", oDebugStrg);
        }
        if (oLocalConfigState.operational != i->m_oSlaveState.operational)
        {
            sprintf(oDebugStrg, "0x%08x:0x%08x: %soperational", i->m_oVendorId, i->m_oProductCode, oLocalConfigState.operational ? "" : "Not ");
            wmLog(eDebug, "%s\n", oDebugStrg);
        }
        i->m_oSlaveState = oLocalConfigState;
    }

    if (!m_allSlavesOperational)
    {
        char oString[41] {};
        m_allSlavesOperational = true;
        for(auto i = m_oSlaveDirectory.begin();i != m_oSlaveDirectory.end();i++)
        {
            if (i->m_oSlaveState.operational)
            {
                strcat(oString, "1,");
            }
            else
            {
                strcat(oString, "0,");
                m_allSlavesOperational = false;
            }
        }
        wmLog(eDebug, "operational: %d , %s\n", m_allSlavesOperational, oString);
    }
}

void EtherCATMaster::check_domain1_state(void)
{
    ec_domain_state_t oDomainState;

    ecrt_domain_state(m_pDomain1, &oDomainState);

    if (oDomainState.working_counter != m_oDomain1State.working_counter)
    {
        wmLog(eDebug, "Domain1: WC %d\n", oDomainState.working_counter);
    }
    if (oDomainState.wc_state != m_oDomain1State.wc_state)
    {
        wmLog(eDebug, "Domain1: State %d\n", oDomainState.wc_state);
    }

    m_oDomain1State = oDomainState;
}

// Thread Funktion muss ausserhalb der Klasse sein
void *ECATCycleTaskThread(void *p_pArg)
{
    prctl(PR_SET_NAME, "ECATCycleTaskThread");
	struct timespec oWakeupTime;
	int retValue;

	struct DataToECATCycleTaskThread* pDataToECATCycleTaskThread;
	EtherCATMaster* pEtherCATMaster;

	pDataToECATCycleTaskThread = static_cast<struct DataToECATCycleTaskThread *>(p_pArg);
	pEtherCATMaster = pDataToECATCycleTaskThread->m_pEtherCATMaster;

	wmLog(eDebug, "ECATCycleTaskThread is started\n");

    pthread_t oMyPthread_ID = pthread_self();

    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(3, &cpuset);
    if (pthread_setaffinity_np(oMyPthread_ID, sizeof(cpuset), &cpuset) != 0)
    {
        wmLog(eError, "ECATCycleTaskThread: cannot set cpu affinity\n");
    }

    system::makeThreadRealTime(system::Priority::FieldBusCyclicTask);

    usleep(10000 * 1000); // 10 seconds delay before starting to feed interfaces

	wmLog(eDebug, "ECATCycleTaskThread is active\n");

	clock_gettime(CLOCK_TO_USE, &oWakeupTime);
	oWakeupTime.tv_sec += 1; // start in future
	oWakeupTime.tv_nsec = 0;

    bool oFirstLoop = true;
	while(!s_threadsStopped)
	{
		retValue = clock_nanosleep(CLOCK_TO_USE, TIMER_ABSTIME, &oWakeupTime, NULL);
		if (retValue)
		{
            wmLogTr(eError, "QnxMsg.VI.ECMCyclicSleepFail", "Sleeping time for cycle loop failed (%s) !\n", strerror(retValue));
			break;
		}

#if ECAT_CYCLE_VIA_SERIAL_PORT
        static bool oToggleCyclicTask = true;
        if (oToggleCyclicTask)
        {
            int oRetVal = ioctl(g_oDebugSerialFd, TIOCMBIS, &g_oDTR01_flag);
            if (oRetVal != 0)
            {
                printf("Error in ioctl\n");
                perror("");
            }
            oToggleCyclicTask = false;
        }
        else
        {
            int oRetVal = ioctl(g_oDebugSerialFd, TIOCMBIC, &g_oDTR01_flag);
            if (oRetVal != 0)
            {
                printf("Error in ioctl\n");
                perror("");
            }
            oToggleCyclicTask = true;
        }
#endif

        if (oFirstLoop)
        {
            oFirstLoop = false;
        }
#if ECAT_CYCLE_VIA_SERIAL_PORT
        int oRetVal = ioctl(g_oDebugSerialFd, TIOCMBIS, &g_oRTS02_flag);
        if (oRetVal != 0)
        {
            printf("Error in ioctl\n");
            perror("");
        }
#endif
		pEtherCATMaster->ECATCyclicTask();
#if ECAT_CYCLE_VIA_SERIAL_PORT
        oRetVal = ioctl(g_oDebugSerialFd, TIOCMBIC, &g_oRTS02_flag);
        if (oRetVal != 0)
        {
            printf("Error in ioctl\n");
            perror("");
        }
#endif

		oWakeupTime.tv_nsec += CYCLE_TIME_NS;
		while(oWakeupTime.tv_nsec >= NSEC_PER_SEC)
		{
			oWakeupTime.tv_nsec -= NSEC_PER_SEC;
			oWakeupTime.tv_sec++;
		}
	}

    return NULL;
}

void EtherCATMaster::StartDebugDataThread(void)
{
#if ECAT_WITH_DEBUG_DATA_THREAD
	///////////////////////////////////////////////////////
	// Thread für Daten Debugging starten
	///////////////////////////////////////////////////////
	pthread_attr_t oPthreadAttr;
    pthread_attr_init(&oPthreadAttr);

	m_oDataToECATDebugDataThread.m_pEtherCATMaster = this;

    if (pthread_create(&m_oECATDebugDataThread, &oPthreadAttr, &ECATDebugDataThread, &m_oDataToECATDebugDataThread) != 0)
    {
        wmFatal(eBusSystem, "QnxMsg.VI.ECMCreateThreadFail", "Cannot start thread for cyclic operation\n");
    }
#endif
}

void EtherCATMaster::ECATDebugData(void)
{
    FILE *pDebugFile;

    if (m_oDebugFileFull)
    {
        char *oHomeDir;
        oHomeDir = getenv("HOME");
        
        char oFileName[121];
        strcpy(oFileName, oHomeDir);
        strcat(oFileName, "/OversampDebug.txt");
        printf("Filename: %s\n", oFileName);
        pDebugFile = fopen(oFileName, "w");
        if (pDebugFile == NULL)
        {
            perror("Error opening file (OversampDebug)");
        }
        else
        {
            printf("File schreiben (OversampDebug)\n");

            for(int i = 0;i < ANZ_BUFFER;i++)
            {
                for(int j = 0;j < EL3702_OVERSAMP_FACTOR;j++)
                {
                    if (j == 0)
                    {
                        fprintf(pDebugFile, "%7d  %7u  %9ld  %9ld\n", m_oOversampDebugBuffer[i][j], m_oOversampDebugBufferCycCnt[i], m_oOversampDebugBufferTime1[i], m_oOversampDebugBufferTime2[i]);
                    }
                    else
                    {
                        fprintf(pDebugFile, "%7d\n", m_oOversampDebugBuffer[i][j]);
                    }
                }
            }
            fclose(pDebugFile);

            printf("File geschrieben (OversampDebug)\n");
        }

        strcpy(oFileName, oHomeDir);
        strcat(oFileName, "/cycleCountDebug.txt");
        printf("Filename: %s\n", oFileName);
        pDebugFile = fopen(oFileName, "w");
        if (pDebugFile == NULL)
        {
            perror("Error opening file (cycleCountDebug)");
        }
        else
        {
            printf("File schreiben (cycleCountDebug)\n");
            for(int i = 0;i < ANZ_BUFFER;i++)
            {
                fprintf(pDebugFile, "%7u\n", m_oOversampDebugBufferCycCnt[i]);
            }
            fclose(pDebugFile);

            printf("File geschrieben (cycleCountDebug)\n");
        }
        m_oDebugFileFull = false;
    }
}

// Thread Funktion muss ausserhalb der Klasse sein
void *ECATDebugDataThread(void *p_pArg)
{
    struct DataToECATDebugDataThread* pDataToECATDebugDataThread;
    EtherCATMaster* pEtherCATMaster;

    pDataToECATDebugDataThread = static_cast<struct DataToECATDebugDataThread *>(p_pArg);
    pEtherCATMaster = pDataToECATDebugDataThread->m_pEtherCATMaster;

    wmLog(eDebug, "DataToECATDebugDataThread is started\n");

    sleep(10); // 10 seconds delay before starting to feed interfaces

    wmLog(eDebug, "DataToECATDebugDataThread is active\n");

    while(!s_threadsStopped)
    {
        pEtherCATMaster->ECATDebugData();
        sleep(2);
    }

    return NULL;
}

void EtherCATMaster::StartCheckProcessesThread(void)
{
    ///////////////////////////////////////////////////////
    // start thread for process monitoring
    ///////////////////////////////////////////////////////
    pthread_attr_t oPthreadAttr;
    pthread_attr_init(&oPthreadAttr);

    m_oDataToCheckProcessesThread.m_pEtherCATMaster = this;

    if (pthread_create(&m_oCheckProcessesThread, &oPthreadAttr, &CheckProcessesThread, &m_oDataToCheckProcessesThread) != 0)
    {
        wmFatal(eBusSystem, "QnxMsg.VI.CreateProcMonThFail", "Cannot start thread for process monitoring\n");
    }
}

void EtherCATMaster::InitCheckProcesses(void)
{
    m_oConfigParser.getSystemReadyInfo(m_oInfoSystemReady);
    m_oConfigParser.getSystemErrorFieldInfo(m_oInfoSystemErrorField);
    m_oConfigParser.getSystemReadyInfoFull(m_oInfoSystemReadyFull);
    m_oConfigParser.getSystemErrorFieldInfoFull(m_oInfoSystemErrorFieldFull);

#if PROCESS_MONITOR_INIT_DEBUG
    printf("SystemReady m_oPresent:              %d\n", m_oInfoSystemReady.m_oPresent);
    printf("SystemReady m_oVendorID:             %d\n", m_oInfoSystemReady.m_oVendorID);
    printf("SystemReady m_oProductCode:          %d\n", m_oInfoSystemReady.m_oProductCode);
    printf("SystemReady m_oStartBit:             %d\n", m_oInfoSystemReady.m_oStartBit);
    printf("SystemReady m_oLength:               %d\n", m_oInfoSystemReady.m_oLength);
    printf("SystemReady m_oInstance:             %d\n", m_oInfoSystemReady.m_oInstance);
    printf("SystemErrorField m_oPresent:         %d\n", m_oInfoSystemErrorField.m_oPresent);
    printf("SystemErrorField m_oVendorID:        %d\n", m_oInfoSystemErrorField.m_oVendorID);
    printf("SystemErrorField m_oProductCode:     %d\n", m_oInfoSystemErrorField.m_oProductCode);
    printf("SystemErrorField m_oStartBit:        %d\n", m_oInfoSystemErrorField.m_oStartBit);
    printf("SystemErrorField m_oLength:          %d\n", m_oInfoSystemErrorField.m_oLength);
    printf("SystemErrorField m_oInstance:        %d\n", m_oInfoSystemErrorField.m_oInstance);
    printf("SystemReadyFull m_oPresent:          %d\n", m_oInfoSystemReadyFull.m_oPresent);
    printf("SystemReadyFull m_oVendorID:         %d\n", m_oInfoSystemReadyFull.m_oVendorID);
    printf("SystemReadyFull m_oProductCode:      %d\n", m_oInfoSystemReadyFull.m_oProductCode);
    printf("SystemReadyFull m_oStartBit:         %d\n", m_oInfoSystemReadyFull.m_oStartBit);
    printf("SystemReadyFull m_oLength:           %d\n", m_oInfoSystemReadyFull.m_oLength);
    printf("SystemReadyFull m_oInstance:         %d\n", m_oInfoSystemReadyFull.m_oInstance);
    printf("SystemErrorFieldFull m_oPresent:     %d\n", m_oInfoSystemErrorFieldFull.m_oPresent);
    printf("SystemErrorFieldFull m_oVendorID:    %d\n", m_oInfoSystemErrorFieldFull.m_oVendorID);
    printf("SystemErrorFieldFull m_oProductCode: %d\n", m_oInfoSystemErrorFieldFull.m_oProductCode);
    printf("SystemErrorFieldFull m_oStartBit:    %d\n", m_oInfoSystemErrorFieldFull.m_oStartBit);
    printf("SystemErrorFieldFull m_oLength:      %d\n", m_oInfoSystemErrorFieldFull.m_oLength);
    printf("SystemErrorFieldFull m_oInstance:    %d\n", m_oInfoSystemErrorFieldFull.m_oInstance);
#endif

    pthread_mutex_lock(&m_oCheckProcessesMutex);
    int oGatewayInstance = 0;
    for(unsigned int i = 0;i < m_oSlaveInfo4Service.size();i++)
    {
#if PROCESS_MONITOR_INIT_DEBUG
        printf("----------\n");
        printf("dwProductCode: %d\n", m_oSlaveInfo4Service[i].dwProductCode);
        printf("dwVendorId:    %d\n", m_oSlaveInfo4Service[i].dwVendorId);
        printf("wPortState:    %d\n", m_oSlaveInfo4Service[i].wPortState);
        printf("dwPdOffsOut:   %d\n", m_oSlaveInfo4Service[i].dwPdOffsOut);
        printf("dwPdSizeOut:   %d\n", m_oSlaveInfo4Service[i].dwPdSizeOut);
#endif

        if ((m_oSlaveInfo4Service[i].dwProductCode == m_oInfoSystemReady.m_oProductCode) &&
            (m_oSlaveInfo4Service[i].dwVendorId == m_oInfoSystemReady.m_oVendorID))
        {
            oGatewayInstance++;
        }

        if ((m_oSlaveInfo4Service[i].dwProductCode == m_oInfoSystemReady.m_oProductCode) &&
            (m_oSlaveInfo4Service[i].dwVendorId == m_oInfoSystemReady.m_oVendorID) &&
            (oGatewayInstance == m_oInfoSystemReady.m_oInstance))
        {
            m_oSystemReadyOffset = (m_oSlaveInfo4Service[i].dwPdOffsOut / 8) + (m_oInfoSystemReady.m_oStartBit / 8);
        }
        if ((m_oSlaveInfo4Service[i].dwProductCode == m_oInfoSystemReadyFull.m_oProductCode) &&
            (m_oSlaveInfo4Service[i].dwVendorId == m_oInfoSystemReadyFull.m_oVendorID) &&
            (oGatewayInstance == m_oInfoSystemReadyFull.m_oInstance))
        {
            m_oSystemReadyOffsetFull = (m_oSlaveInfo4Service[i].dwPdOffsOut / 8) + (m_oInfoSystemReadyFull.m_oStartBit / 8);
        }
        if ((m_oSlaveInfo4Service[i].dwProductCode == m_oInfoSystemErrorField.m_oProductCode) &&
            (m_oSlaveInfo4Service[i].dwVendorId == m_oInfoSystemErrorField.m_oVendorID) &&
            (oGatewayInstance == m_oInfoSystemErrorField.m_oInstance))
        {
            m_oSystemErrorFieldOffset = (m_oSlaveInfo4Service[i].dwPdOffsOut / 8) + (m_oInfoSystemErrorField.m_oStartBit / 8);
        }
        if ((m_oSlaveInfo4Service[i].dwProductCode == m_oInfoSystemErrorFieldFull.m_oProductCode) &&
            (m_oSlaveInfo4Service[i].dwVendorId == m_oInfoSystemErrorFieldFull.m_oVendorID) &&
            (oGatewayInstance == m_oInfoSystemErrorFieldFull.m_oInstance))
        {
            m_oSystemErrorFieldOffsetFull = (m_oSlaveInfo4Service[i].dwPdOffsOut / 8) + (m_oInfoSystemErrorFieldFull.m_oStartBit / 8);
        }
    }
    m_oSystemReadyMask = 0x01 << (m_oInfoSystemReady.m_oStartBit % 8);
    m_oSystemReadyMaskFull = 0x01 << (m_oInfoSystemReadyFull.m_oStartBit % 8);
    if (eInternalError < 256)
    {
        m_oSystemErrorFieldMask1 = (unsigned char)(eInternalError);
        m_oSystemErrorFieldMask2 = 0x00;
        m_oSystemErrorFieldMask1Full = (unsigned char)(eInternalError);
        m_oSystemErrorFieldMask2Full = 0x00;
    }
    else
    {
        m_oSystemErrorFieldMask1 = 0x00;
        m_oSystemErrorFieldMask2 = (unsigned char)(eInternalError - 256);
        m_oSystemErrorFieldMask1Full = 0x00;
        m_oSystemErrorFieldMask2Full = (unsigned char)(eInternalError - 256);
    }

#if PROCESS_MONITOR_INIT_DEBUG
    printf("m_oSystemReadyOffset:         %d\n",   m_oSystemReadyOffset);
    printf("m_oSystemReadyMask:           %02X\n", m_oSystemReadyMask);
    printf("m_oSystemReadyOffsetFull:     %d\n",   m_oSystemReadyOffsetFull);
    printf("m_oSystemReadyMaskFull:       %02X\n", m_oSystemReadyMaskFull);
    printf("m_oSystemErrorFieldOffset     %d\n",   m_oSystemErrorFieldOffset);
    printf("m_oSystemErrorFieldMask1:     %02X\n", m_oSystemErrorFieldMask1);
    printf("m_oSystemErrorFieldMask2:     %02X\n", m_oSystemErrorFieldMask2);
    printf("m_oSystemErrorFieldOffsetFull %d\n",   m_oSystemErrorFieldOffsetFull);
    printf("m_oSystemErrorFieldMask1Full: %02X\n", m_oSystemErrorFieldMask1Full);
    printf("m_oSystemErrorFieldMask2Full: %02X\n", m_oSystemErrorFieldMask2Full);
#endif

    pthread_mutex_unlock(&m_oCheckProcessesMutex);
}

void EtherCATMaster::CheckProcesses(void)
{
#if PROCESS_MONITOR_DEBUG
    printf("************************************\n");
#endif
    for(int i = 0;i <= LAST_KEY_INDEX;i++)
    {
        size_t dotPos;
        dotPos = precitec::interface::pidKeys[i].rfind('.');
        if(dotPos == std::string::npos)
        {
            continue;
        }

        std::string appName = precitec::interface::pidKeys[i].substr( 0, dotPos );
        std::string appNameWatch = appName;
        std::string appNameIsAlive = appName;
        std::string appNamePid = appName;

        appNameWatch.append(".Watch");
        appNameIsAlive.append(".IsAlive");
        appNamePid.append(".Pid");

        bool appNameWatchState = precitec::interface::ConnectionConfiguration::instance().getBool(appNameWatch, true);
        bool appNameIsAliveState = precitec::interface::ConnectionConfiguration::instance().getBool(appNameIsAlive, false);
        int appNamePidNo = precitec::interface::ConnectionConfiguration::instance().getInt(appNamePid, 0);

#if PROCESS_MONITOR_DEBUG
        printf("------%s------\n", appName.c_str());
        printf("%s: %d\n", appNameWatch.c_str(), appNameWatchState);
        printf("%s: %d\n", appNameIsAlive.c_str(), appNameIsAliveState);
        printf("%s: %d\n", appNamePid.c_str(), appNamePidNo);
#endif

        bool oProcessIsDead = false;
        if (appNameWatchState) // Prozess soll ueberwacht werden
        {
            if (!appNameIsAliveState) // ConnectServer meldet: Prozess lebt nicht mehr
            {
#if PROCESS_MONITOR_DEBUG
                printf("%s: %d\n", appNameIsAlive.c_str(), appNameIsAliveState);
#endif
                oProcessIsDead = true;
            }
            if (appNamePidNo == 0) // Der Prozess wurde nie korrekt gestartet
            {
#if PROCESS_MONITOR_DEBUG
                printf("%s: %d\n", appNamePid.c_str(), appNamePidNo);
#endif
                oProcessIsDead = true;
            }
            else // Prozess wurde gestartet
            {
                int appNameGpid = getpgid(appNamePidNo); // read group ID, if process doesn't exists, an error is returned
                if (appNameGpid == -1) // Auf den Prozess kann nicht zugegriffen werden
                {
#if PROCESS_MONITOR_DEBUG
                    printf("pgid of %s: %d\n", appName.c_str(), appNameGpid);
#endif
                    oProcessIsDead = true;
                }
            }
        } // if (appNameWatchState)
        if (oProcessIsDead)
        {
            printf("--------> %s is no longer running <--------\n", appName.c_str());
            wmLogTr(eError, "QnxMsg.VI.ProcMonMissing", "process is not longer alive: %s\n", appName.c_str());
            wmFatal(eInternalError, "QnxMsg.VI.ProcMonMissing", "process is not longer alive: %s\n", appName.c_str());
            pthread_mutex_lock(&m_oCheckProcessesMutex);
            m_oSystemReadyState = false;
            m_oSystemErrorFieldValue = eInternalError;
            pthread_mutex_unlock(&m_oCheckProcessesMutex);
        } // if (oProcessIsDead)
        sched_yield();
    } // <= LAST_KEY_INDEX
}

// Thread Funktion muss ausserhalb der Klasse sein
void *CheckProcessesThread(void *p_pArg)
{
    prctl(PR_SET_NAME, "CheckProcesses");
    struct DataToCheckProcessesThread* pDataToCheckProcessesThread;
    EtherCATMaster* pEtherCATMaster;

    pDataToCheckProcessesThread = static_cast<struct DataToCheckProcessesThread *>(p_pArg);
    pEtherCATMaster = pDataToCheckProcessesThread->m_pEtherCATMaster;

    wmLog(eDebug, "CheckProcessesThread is started\n");

    sleep(60); // 60 seconds delay before monitoring processes
    pEtherCATMaster->InitCheckProcesses();

    wmLog(eDebug, "CheckProcessesThread is active\n");

    while(!s_threadsStopped)
    {
        pEtherCATMaster->CheckProcesses();
        sleep(10); // wait 10 seconds to next check
    }

    return NULL;
}

///////////////////////////////////////////////////////////
// interface functions (public member)
///////////////////////////////////////////////////////////

void EtherCATMaster::ecatDigitalOut(EcatProductIndex productIndex, EcatInstance instance, uint8_t value, uint8_t mask) // Interface EthercatOutputs
{
    if (productIndex == eProductIndex_EL2008)
    {
        pthread_mutex_lock(&oEL2008OutLock);
        if ((int)instance > m_oSlaveNumbers.m_oNbr_EL2008)
        {
            // falsche instance !
            wmLogTr(eError, "QnxMsg.VI.ECMWrongInst", "Wrong instance (%d) for output terminal %s\n", static_cast<int>(instance), "EL2008");
            instance = eInstance1; // damit nicht falscher Speicher ueberschrieben wird
        }
        int oInstanceIndex = (int)instance - 1;

        m_oEL2008OutputBuffer[oInstanceIndex] &= ~mask;
        m_oEL2008OutputBuffer[oInstanceIndex] |= value;

        pthread_mutex_unlock(&oEL2008OutLock);
    }
    else
    {
        // falscher productIndex !
        wmLogTr(eError, "QnxMsg.VI.ECMWrongProd", "Wrong product index (%d) for %s\n", static_cast<int>(productIndex), "digital output");
        return;
    }
#if ECAT_DEBUG_OUTPUTS
	static uint8_t oBuffer = 0;

	if (value != oBuffer)
	{
		printf("ecatDigitalOut: productIndex: %d, instance: %d, value: %02X, mask: %02X\n", productIndex, instance, value, mask);
	}
	oBuffer = value;
#endif
}

void EtherCATMaster::ecatAnalogOut(EcatProductIndex productIndex, EcatInstance instance, EcatChannel channel, uint16_t value) // Interface EthercatOutputs
{
    if (productIndex == eProductIndex_EL4102)
    {
        pthread_mutex_lock(&oEL4102OutLock);
        if ((int)instance > m_oSlaveNumbers.m_oNbr_EL4102)
        {
            // falsche instance !
            wmLogTr(eError, "QnxMsg.VI.ECMWrongInst", "Wrong instance (%d) for output terminal %s\n", static_cast<int>(instance), "EL4102");
            instance = eInstance1; // damit nicht falscher Speicher ueberschrieben wird
        }
        int oInstanceIndex = (int)instance - 1;

        if (channel == eChannel1)
        {
            m_oEL4102OutputBuffer[oInstanceIndex][0] = value;
        }
        else if (channel == eChannel2)
        {
            m_oEL4102OutputBuffer[oInstanceIndex][1] = value;
        }
        else
        {
            // falscher channel !
        }
        pthread_mutex_unlock(&oEL4102OutLock);
    }
    else if (productIndex == eProductIndex_EL4132)
    {
        pthread_mutex_lock(&oEL4132OutLock);
        if ((int)instance > m_oSlaveNumbers.m_oNbr_EL4132)
        {
            // falsche instance !
            wmLogTr(eError, "QnxMsg.VI.ECMWrongInst", "Wrong instance (%d) for output terminal %s\n", static_cast<int>(instance), "EL4132");
            instance = eInstance1; // damit nicht falscher Speicher ueberschrieben wird
        }
        int oInstanceIndex = (int)instance - 1;

        if (channel == eChannel1)
        {
            m_oEL4132OutputBuffer[oInstanceIndex][0] = value;
        }
        else if (channel == eChannel2)
        {
            m_oEL4132OutputBuffer[oInstanceIndex][1] = value;
        }
        else
        {
            // falscher channel !
        }
        pthread_mutex_unlock(&oEL4132OutLock);
    }
    else
    {
        // falscher productIndex !
        wmLogTr(eError, "QnxMsg.VI.ECMWrongProd", "Wrong product index (%d) for %s\n", static_cast<int>(productIndex), "analog output");
        return;
    }
#if ECAT_DEBUG_OUTPUTS
	static uint8_t oBuffer = 0;

	if (value != oBuffer)
	{
		printf("ecatAnalogOut: productIndex: %u, instance: %u, channel: %u, value: %04X\n", productIndex, instance, channel, value);
	}
	oBuffer = value;
#endif
}

void EtherCATMaster::ecatGatewayOut(EcatProductIndex productIndex, EcatInstance instance, uint8_t size, stdVecUINT8 data, stdVecUINT8 mask) // Interface EthercatOutputs
{
    if (m_oFieldbusViaSeparateFieldbusBoard) // communication via separate fieldbus board
    {
        // do nothing
        return;
    }

    if (m_oSlaveNumbers.m_oNbr_GATEWAY == 0) // there was no Gateway found on EtherCAT
    {
        // do nothing
        return;
    }

    if ((productIndex == eProductIndex_Anybus_GW) ||
        (productIndex == eProductIndex_Kunbus_GW))
    {
        pthread_mutex_lock(&oGatewayOutLock);
        if ((int)instance > m_oSlaveNumbers.m_oNbr_GATEWAY)
        {
            // falsche instance !
            wmLogTr(eError, "QnxMsg.VI.ECMWrongInst", "Wrong instance (%d) for output terminal %s\n", static_cast<int>(instance), "GATEWAY");
            instance = eInstance1; // damit nicht falscher Speicher ueberschrieben wird
        }
        int oInstanceIndex = (int)instance - 1;

        for(unsigned int i = 0;i < size;i++)
        {
            m_oGatewayOutputBuffer[oInstanceIndex][i] &= ~mask[i];
            m_oGatewayOutputBuffer[oInstanceIndex][i] |= data[i];
        }

        pthread_mutex_unlock(&oGatewayOutLock);
    }
    else
    {
        // falscher productIndex !
        wmLogTr(eError, "QnxMsg.VI.ECMWrongProd", "Wrong product index (%d) for %s\n", static_cast<int>(productIndex), "fieldbus gateway");
        return;
    }

#if ECAT_DEBUG_OUTPUTS
    static uint8_t oBuffer[MAX_GATEWAY_OUTPUT_LENGTH]{};
    bool oChangeFlag = false;

    for(unsigned int i = 0;i < size;i++)
    {
        if (oBuffer[i] != mask[i]) oChangeFlag = true;
        oBuffer[i] = mask[i];
    }

    if (oChangeFlag)
    {
        printf("ecatGatewayOut: instance: %d size: %u\n", (int)instance, size);
        for (unsigned int i = 0;i < size;i++)
        {
            printf("%02X ", data[i]);
        }
        printf("\n");
        for (unsigned int i = 0;i < size;i++)
        {
            printf("%02X ", mask[i]);
        }
        printf("\n");
    }
#endif
}

void EtherCATMaster::ecatEncoderOut(EcatProductIndex productIndex, EcatInstance instance, uint16_t command, uint32_t setCounterValue) // Interface EthercatOutputs
{
    switch (productIndex)
    {
        case eProductIndex_EL5101:
            {
                pthread_mutex_lock(&oEL5101OutLock);
                if ((int)instance > m_oSlaveNumbers.m_oNbr_EL5101)
                {
                    // falsche instance !
                    wmLogTr(eError, "QnxMsg.VI.ECMWrongInst", "Wrong instance (%d) for output terminal %s\n", static_cast<int>(instance), "EL5101");
                    instance = eInstance1; // damit nicht falscher Speicher ueberschrieben wird
                }
                int oInstanceIndex = (int)instance - 1;

                m_oEL5101OutputBuffer[oInstanceIndex][0] = command;
                m_oEL5101OutputBuffer[oInstanceIndex][1] = setCounterValue;

                pthread_mutex_unlock(&oEL5101OutLock);
            }
            break;
        case eProductIndex_EL5151:
            {
                pthread_mutex_lock(&m_EL5151OutputLock);
                if ((int)instance > m_oSlaveNumbers.m_numberOfEL5151)
                {
                    // wrong instance !
                    wmLogTr(eError, "QnxMsg.VI.ECMWrongInst", "Wrong instance (%d) for output terminal %s\n", static_cast<int>(instance), "EL5151");
                    instance = eInstance1; // to avoid writing into wrong memory
                }
                int instanceIndex = (int)instance - 1;

                m_EL5151OutputBuffer[instanceIndex][0] = command;
                m_EL5151OutputBuffer[instanceIndex][1] = setCounterValue;

                pthread_mutex_unlock(&m_EL5151OutputLock);
            }
            break;
        default:
            {
                // falscher productIndex !
                wmLogTr(eError, "QnxMsg.VI.ECMWrongProd", "Wrong product index (%d) for %s\n", static_cast<int>(productIndex), "encoder terminal");
                return;
            }
            break;
    }

#if ECAT_DEBUG_OUTPUTS
	static uint8_t oBuffer = 0;

	if (command != oBuffer)
	{
		printf("ecatEncoderOut: productIndex: %u, instance: %u, command: %04X\n", productIndex, instance, command);
	}
	oBuffer = command;
#endif
}

void EtherCATMaster::ecatAxisOut(EcatProductIndex productIndex, EcatInstance instance, EcatAxisOutput axisOutput) // Interface EthercatOutputs
{
    if (productIndex == eProductIndex_ACCELNET)
    {
        pthread_mutex_lock(&oACCELNETOutLock);
        if ((int)instance > m_oSlaveNumbers.m_oNbr_ACCELNET)
        {
            // falsche instance !
            wmLogTr(eError, "QnxMsg.VI.ECMWrongInst", "Wrong instance (%d) for output terminal %s\n", static_cast<int>(instance), "ACCELNET");
            instance = eInstance1; // damit nicht falscher Speicher ueberschrieben wird
        }
        int oInstanceIndex = (int)instance - 1;

        memcpy(&m_oACCELNETOutputBuffer[oInstanceIndex], &axisOutput, sizeof(EcatAxisOutput));

        pthread_mutex_unlock(&oACCELNETOutLock);
    }
    else if (productIndex == eProductIndex_EPOS4)
    {
        pthread_mutex_lock(&oEPOS4OutLock);
        if ((int)instance > m_oSlaveNumbers.m_oNbr_EPOS4)
        {
            // falsche instance !
            wmLogTr(eError, "QnxMsg.VI.ECMWrongInst", "Wrong instance (%d) for output terminal %s\n", static_cast<int>(instance), "EPOS4");
            instance = eInstance1; // damit nicht falscher Speicher ueberschrieben wird
        }
        int oInstanceIndex = (int)instance - 1;

        memcpy(&m_oEPOS4OutputBuffer[oInstanceIndex], &axisOutput, sizeof(EcatAxisOutput));

        pthread_mutex_unlock(&oEPOS4OutLock);
    }
    else
    {
        // falscher productIndex !
        wmLogTr(eError, "QnxMsg.VI.ECMWrongProd", "Wrong product index (%d) for %s\n", static_cast<int>(productIndex), "axis controller");
        return;
    }

#if ECAT_DEBUG_OUTPUTS
	static uint16_t oBuffer = 0;

	if (axisOutput.controlWord != oBuffer)
	{
		printf("ecatAxisOut: productIndex: %u, instance: %u, controlWord: %04X\n", productIndex, instance, axisOutput.controlWord);
	}
	oBuffer = axisOutput.controlWord;
#endif
}

void EtherCATMaster::ecatFRONTENDOut(EcatProductIndex productIndex, EcatInstance instance, EcatFRONTENDOutput frontendOutput) // Interface EthercatOutputs
{
    if (productIndex == eProductIndex_Frontend)
    {
        pthread_mutex_lock(&oFRONTENDOutLock);
        if ((int)instance > m_oSlaveNumbers.m_oNbr_FRONTEND)
        {
            // falsche instance !
            wmLogTr(eError, "QnxMsg.VI.ECMWrongInst", "Wrong instance (%d) for output terminal %s\n", static_cast<int>(instance), "FRONTEND");
            instance = eInstance1; // damit nicht falscher Speicher ueberschrieben wird
        }
        int oInstanceIndex = (int)instance - 1;

        memcpy(&m_oFRONTENDOutputBuffer[oInstanceIndex], &frontendOutput, sizeof(EcatFRONTENDOutput));

        pthread_mutex_unlock(&oFRONTENDOutLock);
    }
    else
    {
        // falscher productIndex !
        wmLogTr(eError, "QnxMsg.VI.ECMWrongProd", "Wrong product index (%d) for %s\n", static_cast<int>(productIndex), "FRONTEND");
        return;
    }
}

void EtherCATMaster::ecatRequestSlaveInfo(void) // Interface EthercatOutputs
{
    sendSlaveInfo();
}

void EtherCATMaster::sendSlaveInfo()
{
    SlaveInfo oAllSlaveInfos(m_oSlaveInfo4Service.size());
    oAllSlaveInfos.FillBuffer(m_oSlaveInfoArray);
    m_rEthercatInputsToServiceProxy.ecatAllSlaveInfo(oAllSlaveInfos);
}

void EtherCATMaster::resetDebugFileVars(void)
{
    for(int i = 0;i < ANZ_BUFFER;i++)
    {
        for(int j = 0;j < 100;j++)
        {
            m_oOversampDebugBuffer[i][j] = 0;
        }
        m_oOversampDebugBufferCycCnt[i] = 0;
        m_oOversampDebugBufferTime1[i] = 0;
        m_oOversampDebugBufferTime2[i] = 0;
    }
    m_oWriteIndex = 0;
    m_oReadIndex = 0;
    m_oDebugFileFull = false;
}

char * EtherCATMaster::InsertTimeStamp(char * p_pTimeStampStrg)
{
    struct timespec oTimeStamp;
    clock_gettime(CLOCK_REALTIME, &oTimeStamp);
    struct tm *pTmVar;
    pTmVar = localtime(&oTimeStamp.tv_sec);
    sprintf(p_pTimeStampStrg, "%02d:%02d:%02d:%03ld", pTmVar->tm_hour, pTmVar->tm_min, pTmVar->tm_sec, (oTimeStamp.tv_nsec / 1000000));
    return p_pTimeStampStrg;
}

void EtherCATMaster::operationState(OperationState state) // Interface systemStatus
{
    if (!m_firstOperationStateReceived)
    {
        m_firstOperationStateReceived = true;
    }
}

} // namespace ethercat

} // namespace precitec

