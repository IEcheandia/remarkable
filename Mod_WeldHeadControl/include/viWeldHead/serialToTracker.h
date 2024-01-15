/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		Alexander Egger (EA)
 * 	@date		2013
 * 	@brief		Communicates via serial communication with the ScanTracker
 */

#ifndef SERIALTOTRACKER_H_
#define SERIALTOTRACKER_H_

#include <pthread.h>
#include <termios.h>

namespace precitec
{

namespace hardware
{

#define SERIAL_QUEUE_LEN  2000L

struct SerialQueueType
{
    unsigned char m_oBuffer[SERIAL_QUEUE_LEN];
    unsigned short m_oWriteIndex;
    unsigned short m_oReadIndex;
    int m_oBufferLimit;
    unsigned long m_oAnzSerial;
};
typedef SerialQueueType *pSerialQueueType;

struct DataToSerialBasicsThread
{
	SerialQueueType *m_pSerialInQueue;
	int *m_pPreciRS422DriverFd;
};

struct PciType
{
    unsigned int m_oBusnum;
    unsigned int m_oDevfuncnum;
    unsigned short m_oVendorID;
    unsigned short m_oDeviceID;
    unsigned short m_oCommandReg;
    unsigned short m_oStatusReg;
    unsigned char m_oRevID;
    unsigned long m_oClassCode;
    unsigned long m_oBaseAddress0;
    unsigned long m_oBaseAddress1;
    unsigned long m_oBaseAddress2;
    unsigned long m_oBaseAddress3;
    unsigned long m_oBaseAddress4;
    unsigned long m_oBaseAddress5;
    unsigned long m_oCardbusPtr;
    unsigned short m_oSubVendorID;
    unsigned short m_oSubDeviceID;
    unsigned long m_oExpansionRom;
    unsigned char m_oIRQLine;
    unsigned char m_oIRQPin;
    unsigned char m_oMinGrant;
    unsigned char m_oMaxLatency;
};

struct PCISerialBoard_Type
{
    bool m_oBoardFound;
    unsigned long m_oUART0BaseAddress;
    unsigned long m_oUART1BaseAddress;
    unsigned long m_oLocalConfBaseAddress;
    unsigned char m_oIRQLine;
};

class SerialBasicsInterface;
class SerialToTracker;

struct DataToSerialToTrackerThread
{
	SerialBasicsInterface* m_pSerialBasicsInterface;
	SerialToTracker* m_pSerialToTracker;
};

enum ComPortAddr {eADR_COM1=0x3F8, eADR_COM2=0x2F8, eADR_COM3=0x3E8, eADR_COM4=0x2E8};
enum ComPortIRQ {eIRQ_COM1=4, eIRQ_COM2=3};

// identischer enum existiert im wmMain, bei Aenderungen hier muss auch im wmMain geaendert weden !
enum TrackerFrequencyStep {eFreq30, eFreq40, eFreq50, eFreq100, eFreq150, eFreq200, eFreq250, eFreq300,
						   eFreq350, eFreq400, eFreq450, eFreq500, eFreq550, eFreq600, eFreq650, eFreq700, eFreq750};
enum TrackerMode {eOff, eOn};

class SerialBasicsInterface
{
    public:
        SerialBasicsInterface() {}
        virtual ~SerialBasicsInterface() {}

        virtual int readSerialProtocolData (void) = 0;
        virtual void collectSerialChar (unsigned char p_oSendChar) = 0;
        virtual void xmtSerialChar (void) = 0;
        virtual void test (void) = 0;
};

class SerialBasics : public SerialBasicsInterface
{
    public:

		/**
	 	 * @brief CTor.
	 	 * @param
	 	 */
		SerialBasics(int p_oSerialCommType = 1, int p_oSerialCommPort = 1);

		/**
		* @brief DTor.
		*/
		virtual ~SerialBasics();

		void setComPort (ComPortAddr p_oPortAddr, ComPortIRQ p_oPortIRQ);
		void initComParameter (float p_oBaud, int p_oDBits, int p_oSBits, char p_oPar);
		void initBaudrate (float p_oBaud);
		unsigned char getComStatus (void);
		unsigned char getComChar (void);
		void sendCharCom (unsigned char p_oSendASCII);
		void startCOMInterrupt (void);
		void stopCOMInterrupt (void);

		virtual int readSerialProtocolData (void);
		virtual void collectSerialChar (unsigned char p_oSendChar);
		virtual void xmtSerialChar (void);
		virtual void test (void);

	private:

		int m_oPreciRS422DriverFd;

		int m_oSerialCommType;
		int m_oSerialCommPort;

		unsigned short m_oSerialBase;
		unsigned short m_oSerialIRQ;

		SerialQueueType m_oSerialInQueue;
	    SerialQueueType m_oSerialOutQueue;

		pthread_t m_oThread;
		struct DataToSerialBasicsThread m_oDataToSerialBasicsThread;

}; // class SerialBasics

class SerialBasicsDirectOS : public SerialBasicsInterface
{
    public:

        /**
        * @brief CTor.
        * @param
        */
        SerialBasicsDirectOS();

        /**
        * @brief DTor.
        */
        virtual ~SerialBasicsDirectOS();

        void initComConfiguration (void);

        virtual int readSerialProtocolData (void);
        virtual void collectSerialChar (unsigned char p_oSendChar);
        virtual void xmtSerialChar (void);
        virtual void test (void);

    private:

        int m_oPreciRS422DriverFd;

        SerialQueueType m_oSerialInQueue;
        SerialQueueType m_oSerialOutQueue;

        pthread_t m_oThread;
        struct DataToSerialBasicsThread m_oDataToSerialBasicsThread;

        struct termios m_oOldAttributes;
        struct termios m_oNewAttributes;
}; // class SerialBasicsDirectOS

//*****************************************************************************
//*****************************************************************************
//*****************************************************************************

class SerialToTracker
{
	public:

		/**
		* @brief CTor.
		* @param
		*/
		SerialToTracker(int p_oSerialCommType = 1, int p_oSerialCommPort = 1);

		/**
		* @brief DTor.
		*/
		virtual ~SerialToTracker();

		void run (void);
		void stop (void);

		void setTrackerFrequencyStep(TrackerFrequencyStep p_oFreq);
		void setTrackerFrequencyCont(unsigned short p_oFreq);
		void enableTracker(TrackerMode p_oMode);
		void trackerCmdShow(void);
		void trackerCmdRevisions(void);
		void trackerCmdShowSerial(void);
		void trackerCmdLimitTable(void);
		void trackerCmdHelp(void);

		void test (void);

		bool getIsWaitingForAnswer(void) {return m_oIsWaitingForAnswer; };
		void setIsWaitingForAnswer(bool p_oIsWaitingForAnswer) {m_oIsWaitingForAnswer = p_oIsWaitingForAnswer; };
		uint32_t getAnswerTimeoutCounter(void) {return m_oAnswerTimeoutCounter; };
		void setAnswerTimeoutCounter(uint32_t p_oAnswerTimeoutCounter) {m_oAnswerTimeoutCounter = p_oAnswerTimeoutCounter; };
		void incAnswerTimeoutCounter(void) {m_oAnswerTimeoutCounter++; };

	private:

        bool m_oScanTrackerSerialViaOnboardPort;
        std::string m_oScanTrackerSerialOnboardPort;

		int m_oSerialCommType;
		int m_oSerialCommPort;

		SerialBasicsInterface* m_pSerialBasicsInterface;
		SerialBasics* m_pSerialBasics;
        SerialBasicsDirectOS* m_pSerialBasicsDirectOS;

		pthread_t m_oThread;
	    struct DataToSerialToTrackerThread m_oDataToSerialToTrackerThread;

	    bool m_oIsWaitingForAnswer;
	    uint32_t m_oAnswerTimeoutCounter;

}; // class SerialToTracker

} // namespace hardware

} // namespace precitec

#endif /* SERIALTOTRACKER_H_ */

