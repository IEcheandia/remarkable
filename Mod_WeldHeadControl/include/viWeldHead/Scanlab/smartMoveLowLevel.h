/**
 *  @file
 *  @copyright  Precitec Vision GmbH & Co. KG
 *  @author     Alexander Egger (EA)
 *  @date       2023
 *  @brief      Communicates via TCP/IP wuth a SmartMove scanner
 */

#pragma once

#include <string>
#include <optional>
#include "common/definesSmartMove.h"

namespace precitec
{

using precitec::smartMove::TCPSendStatus;

namespace hardware
{

#define XMODEM_SOH 0x01 /* start of header (128 byte block) */
#define XMODEM_STX 0x02 /* start of header for extended block (1024 byteblock) */
#define XMODEM_EOT 0x04 /* end of transmission */
#define XMODEM_ACK 0x06 /* acknowledge */
#define XMODEM_NAK 0x15 /* not acknowledge */
#define XMODEM_CAN 0x18 /* cancel (force receiver to send try chars again */
#define XMODEM_TRYCHAR 0x43 /* ASCII "C" */
#define XMODEM_EOFCHAR 0x1A /* fill last package with ^Z after last byte of file */

typedef enum
{
    XMODEM_STATUS_NOINIT, /* not initialized */
    XMODEM_STATUS_INIT , /* initialized, but no sync so far */
    XMODEM_STATUS_TRANSMIT, /* in sync */
    XMODEM_STATUS_CANCEL, /* transmission cancelled by transmitter or receiver */
    XMODEM_STATUS_EOT, /* end of transmission  sent */
    XMODEM_STATUS_ERROR /* error */
}
XmodemStatus;

typedef enum
{
    XMODEM_BUFSIZE_NORMAL = 128,
    XMODEM_BUFSIZE_EXTENDED = 1024
}
XmodemBufferSize;

typedef struct
{
    int MaxRetrans; /* maximal number of retransmissions of a broken packet */
    XmodemBufferSize PacketSize; /* normal blocksize or 1k xmodem */
}
XmodemParameter;

class SmartMoveLowLevel
{

public:

    SmartMoveLowLevel(void);
    virtual ~SmartMoveLowLevel();

    int openTcpConnection(void);
    void closeTcpConnection(void);

    bool isTCPConnectionOpen(void) const
    {
        return m_sockDesc >= 0;
    }

    TCPSendStatus processJob(const std::string& processJobCommand);
    TCPSendStatus selectJob(const std::string& selectJobCommand);

    int askForVersion(void);
    int askForDownloadInfo(void);
    void askForMessageQueue(void);
    void askForSymbolList(void);
    int doSoftwareReset(void);
    int setJumpSpeed(const int pen, const float speed);

    bool sendFocalLength(const std::string& focalLengthOrder);
    bool sendScanfieldSize(const std::string& scanfieldSizeOrder);
    bool sendForceEnable(const std::string& foceEnable);
    std::string sendCalibrationFilename(const std::string& getCalibrationFilename);

    bool sendSystemOperationMode(const std::string& systemOperationModeOrder);
    bool sendInputMode(const std::string& inputModeOrder);
    bool sendAlignX(const std::string& alignXOrder);
    bool sendAlignY(const std::string& alignYOrder);
    bool sendLaserOnForAlignment(const std::string& laserOnOrder);
    bool sendLaserOnMaxForAlignment(const std::string& laserOnMaxOrder);

    bool sendJumpSpeed(const std::string& jumpSpeedOrder);
    bool sendMarkSpeed(const std::string& markSpeedOrder);

    std::pair<bool, double> sendGetPositionFeedbackX(const std::string& getPositionFeedbackOrder);
    std::pair<bool, double> sendGetPositionFeedbackY(const std::string& getPositionFeedbackOrder);
    std::optional<double> sendGetSysTs(const std::string& command);

    int openFile(const std::string& filePath);
    int closeFile(int fileDescriptor);

    int transmitMarkingFileXModem(int fileDescriptor, std::string& answer);
    int transmitPenFileXModem(int fileDescriptor, std::string& answer);

    int transmitMarkingFileBTX(int fileDescriptor, std::string& answer);
    int transmitPenFileBTX(int fileDescriptor, std::string& answer);

    /**
     * Checks property INT_PRINT_READY. When the signal is 1 @c true is returned, otherwise @c false.
     *
     * From documentation:
     * This signal is inactive during the firmware initialization phase and when
     * the system is marking, otherwise it is active.
     **/
    bool printReady();

    /**
     * Checks property INT_SC_READY. When the signal is 1 @c true is returned, otherwise @c false.
     *
     * From documentation:
     * This signal is active if the scanning system is up and running. If this signal
     * is low, indicating either that the scanning system is still in initialization phase or
     * a scanner fault condition, the laser output will be forced to the inactive state
     * (laser off).
     **/
    bool scannerReady();

    bool sendCommand(const std::string& command, const std::string& errorLogMessage);

    bool uploadFilePermanentlyToFlash(const std::string& filename);

private:
    int convertAnswerToNumber(const std::string& answer, const std::string& errorLogMessage);

    int readMEParameter(const std::string& command, std::string& answer, int timeout);
    int setMEParameter(const std::string& command, std::string& answer, int timeout);
    int readToPrompt(char* answer, size_t maxlen, size_t *nread, int timeout);

    int transmitFileXModem(const std::string& command, int fileDescriptor, std::string& answer);
    int transmitFileBTX(const std::string& command, int fileDescriptor, std::string& answer);

    //NOTE Uploading permanently saved files like calibration/correction file, parameter file and firmware (NOTE But firmware must be checked because it takes a lof of time and the timeout is short)
    void performHardwareReset();
    bool waitForSynchronizationCharacterThenUploadFile(int fileDescriptor);
    bool resetFilePointer(int fileDescriptor);
    void uploadFileWithXModemProtocol(int fileDescriptor);
    bool permanentlyStoredFileFeedback();
    void synchronizeME();

    int xmodemOutBlock(int* data, int bytesToSend);
    int xmodemInpByte(int* inputByte, unsigned int timeout);
    int xmodemWriteChar(int *writePtr);
    int xmodemSync(void);
    XmodemStatus xmodemReset(void);
    void xmodemFlushInput(void);
    XmodemStatus xmodemEOT(void);
    XmodemStatus xmodemSendPacket(int *data, XmodemBufferSize bufsize);
    int xmodemCalcCRC(int* ptr, int count);

    std::string m_scanner2DControllerIpAddress{"192.168.170.105"};
    uint32_t m_scanner2DControllerIpAddressNetwork{0x69AAA8C0};
    const uint16_t m_scanner2DControllerServerPort{10001};

    int m_sockDesc{-1};
    XmodemStatus m_xmodemStatus{XMODEM_STATUS_NOINIT};
    int m_xmodemPacketBuf[XMODEM_BUFSIZE_EXTENDED]{};
    int m_xmodemPacketNo{};
    XmodemParameter m_xmodemParameter{};
};

}
}

