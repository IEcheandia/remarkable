/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		Alexander Egger (EA)
 * 	@date		2014
 * 	@brief		Communicates via TCP/IP communication with LaserControl device
 */

#ifndef DATATOLASERCONTROL_H_
#define DATATOLASERCONTROL_H_

#include <iostream>

#include "Poco/Mutex.h"

namespace precitec
{

namespace hardware
{

struct LCDataDef
{
    signed long variable1;
    signed long variable2;
    signed long variable3;
    signed long variable4;
    signed long variable5;
    signed long variable6;
    signed long variable7;
    signed long variable8;
    signed long variable9;
    signed long variable10;
    signed long variable11;
    signed long variable12;
    signed long variable13;
    signed long variable14;
    signed long variable15;
    signed long variable16;
    signed long variable17;
    signed long variable18;
    signed long variable19;
    signed long variable20;
    signed long variable21;
    signed long variable22;
    signed long variable23;
    signed long variable24;
    signed long variable25;
    signed long variable26;
    signed long variable27;
    signed long variable28;
};

struct acknDef
{
    signed long acknCode;
    signed long blockType;
    signed long reserve1;
    signed long reserve2;
    signed long reserve3;
    signed long reserve4;
};

enum LCDataVarIdx { eLCVarIdx1, eLCVarIdx2, eLCVarIdx3, eLCVarIdx4, eLCVarIdx5, eLCVarIdx6, eLCVarIdx7, eLCVarIdx8, eLCVarIdx9,
					eLCVarIdx10, eLCVarIdx11, eLCVarIdx12, eLCVarIdx13, eLCVarIdx14, eLCVarIdx15,
					eLCVarIdx16, eLCVarIdx17, eLCVarIdx18, eLCVarIdx19, eLCVarIdx20, eLCVarIdx21,
					eLCVarIdx22, eLCVarIdx23, eLCVarIdx24, eLCVarIdx25, eLCVarIdx26, eLCVarIdx27,
					eLCVarIdx28 };

enum SocketMode { eSetNonBlocking, eSetBlocking };

class DataToLaserControl
{
    public:

		/**
	 	 * @brief CTor.
	 	 * @param
	 	 */
		DataToLaserControl();

		/**
		* @brief DTor.
		*/
		virtual ~DataToLaserControl();

		void setDataVariable(LCDataVarIdx p_oVarIdx, signed long p_oVarValue);
		signed long getDataVariable(LCDataVarIdx p_oVarIdx);
		void setInspectCycleIsOn(bool p_oState);

		int openTcpConnectionToLC (void);
		int sendDataBlockToLC (void);
		int closeTcpConnectionToLC (void);

		int sendDataToLC(void);

    private:
		// consts for connection
		std::string SERVER_NAME_LASERCONTROL;
		static const int TCPSERVER_THREAD_PORT                = 1024;

		// consts for LaserControl TCP/IP Block
		static const int DBLOCK_LASERCONTROL                  = 1;
		static const int DBLOCK_LASERCONTROL_VERSION          = 0x0200;

		// consts for Acknowledge TCP/IP Block
		static const int DBLOCK_ACKN                          = 99;
		static const int DBLOCK_ACKN_VERSION                  = 0x0100;
		static const int DBLOCK_ACKN_OK                       = 111;
		static const int DBLOCK_ACKN_ERROR                    = 200;

		LCDataDef m_oLCDataBlock;
		acknDef m_oAcknBlock;

		int m_oSockDesc;
		bool m_oTcpConnIsOpen;
		bool m_oTcpOpeningIsActive;
		bool m_oErrorState;
		bool m_oInspectCycleIsOn;

		void packLCData(LCDataDef & p_rLCDataBlock, char * p_pSendBuffer, int & p_rBufLength);
		void unpackAcknBlock(char * p_pRecvBuffer, acknDef & p_rAcknBlock);

		int setSocketMode (int p_oSockDesc, SocketMode p_oMode);

		Poco::FastMutex m_oTCP_to_LC_Mutex;
}; // class DataToLaserControl

} // namespace hardware

} // namespace precitec

#endif /* DATATOLASERCONTROL_H_ */

