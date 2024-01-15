/**
 *  @file
 *  @copyright  Precitec Vision GmbH & Co. KG
 *  @author     EA
 *  @date       2023
 *  @brief      Handles the TCP/IP communication with LWM
 */

#pragma once

///////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////

#include <string>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <functional>

///////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////

namespace precitec
{

namespace ethercat
{

const int32_t LWM_SIGN_OFF_CLIENT{0x00000007};
const int32_t LWM_WATCHDOG{0x00170000};

const int32_t LWM_RESULT_RANGES{0x00120100};
const int32_t LWM_RESULT_VALUES{0x00120300};
const int32_t LWM_RESULT_VALUES_RANGES{0x00120500};

const int32_t LWM_SIMPLE_IO_SELECTION{0x02201130};
const int32_t LWM_SIMPLE_IO_STOPP{0x02201140};
const int32_t LWM_SIMPLE_IO_ERROR{0x02201150};
const int32_t LWM_SIMPLE_IO_TRIGGER{0x02201160};

///////////////////////////////////////////////////////////
// typedefs
///////////////////////////////////////////////////////////

class TCPClientLWM;

struct DataToTCPClientSendThread
{
    TCPClientLWM* m_TCPClientLWM;
};

struct DataToTCPClientReceiveThread
{
    TCPClientLWM* m_TCPClientLWM;
};

typedef struct
{
    bool m_requestAcknowledge;
    bool m_systemActivated;
    int32_t m_programNumber;
    std::string m_comment;
}
SimpleIOSelectionDataType;

typedef std::array<char, 1500> TCPDataBlockType;

typedef struct
{
    int32_t m_programNumber;
    int32_t m_configId;
    int32_t m_configVersion;
    int32_t m_overallResult;
    int32_t m_extendedResult;
    float m_errorProbability;
    int32_t m_errorSignals;
    int32_t m_measurementId;
    std::string m_comment;
    int16_t m_year;
    int16_t m_month;
    int16_t m_day;
    int16_t m_hour;
    int16_t m_minute;
    int16_t m_second;
    int32_t m_numberOfRanges;
}
ResultsRangesDataType;

typedef std::function<void(void)> TCPCallbackFunctionType1;

///////////////////////////////////////////////////////////
// class TCPClientLWM
///////////////////////////////////////////////////////////

class TCPClientLWM
{

public:
    TCPClientLWM(void);
    virtual ~TCPClientLWM(void);

    int run(void);

    int receiveFunction(void);

    void initSendRequest(std::condition_variable* LWMSendRequestCondVar, std::mutex* LWMSendRequestMutex, bool* LWMSendRequestVariable)
    {
        m_LWMSendRequestCondVar = LWMSendRequestCondVar;
        m_LWMSendRequestMutex = LWMSendRequestMutex;
        m_LWMSendRequestVariable = LWMSendRequestVariable;
    }

    void setSendRequestSimpleIOSelection(const SimpleIOSelectionDataType& simpleIOSelectionData);
    void setSendRequestSimpleIOStopp(const bool requestAcknowledge = true);
    void setSendRequestWatchdog(void);
    void setSendRequestSignOffClient(void);

    int32_t getErrorStatus(void);
    void getResultsRanges(ResultsRangesDataType& resultsRanges);

    void connectSimpleIOSelectionCallback(TCPCallbackFunctionType1 simpleIOSelectionCallback);
    void connectSimpleIOStoppCallback(TCPCallbackFunctionType1 simpleIOStoppCallback);
    void connectSimpleIOTriggerCallback(TCPCallbackFunctionType1 simpleIOTriggerCallback);
    void connectSimpleIOErrorCallback(TCPCallbackFunctionType1 simpleIOErrorCallback);
    void connectResultsRangesCallback(TCPCallbackFunctionType1 resultsRangesCallback);
    void connectConnectionOKCallback(TCPCallbackFunctionType1 connectConnectionOKCallback);
    void connectConnectionNOKCallback(TCPCallbackFunctionType1 connectConnectionNOKCallback);

    pthread_t m_TCPClientReceiveThreadId{0};

private:
    void startTCPClientSendThread(void);
    void startTCPClientReceiveThread(void);
    void stopTCPClientReceiveThread(void);

    //****************************************************************************

    int openCommunicationToLWM(void);

    void sendSimpleIOSelection(void);
    void sendSimpleIOStopp(void);
    void sendWatchdog(void);
    void sendSignOffClient(void);

    void checkSendRequests(void);
    void checkLWMWatchog(void);

    void unpackAcknSimpleIOSelection(TCPDataBlockType& recvBuffer);
    void unpackAcknSimpleIOStopp(TCPDataBlockType& recvBuffer);
    void unpackSimpleIOTrigger(TCPDataBlockType& recvBuffer);
    void unpackSimpleIOError(TCPDataBlockType& recvBuffer);
    void unpackWatchdog(TCPDataBlockType& recvBuffer);

    void unpackResultsRanges(TCPDataBlockType& recvBuffer);
    void unpackResultsValuesRanges(TCPDataBlockType& recvBuffer);

    void evaluateStatus(const int32_t status);
    void evaluateExtendedResult(const int32_t status);

    TCPCallbackFunctionType1 m_simpleIOSelectionCallback;
    TCPCallbackFunctionType1 m_simpleIOStoppCallback;
    TCPCallbackFunctionType1 m_simpleIOTriggerCallback;
    TCPCallbackFunctionType1 m_simpleIOErrorCallback;
    TCPCallbackFunctionType1 m_resultsRangesCallback;
    TCPCallbackFunctionType1 m_connectionOKCallback;
    TCPCallbackFunctionType1 m_connectionNOKCallback;

    //****************************************************************************

    pthread_t m_TCPClientSendThreadId;
    struct DataToTCPClientSendThread m_dataToTCPClientSendThread;

    struct DataToTCPClientReceiveThread m_dataToTCPClientReceiveThread;

    std::string m_LWMDeviceIpAddress;
    uint32_t m_LWMDeviceIpAddressInt; // in network byte order
    uint16_t m_LWMDeviceTCPPort;

    int m_socket{-1};
    bool m_connectionIsOn{false};

    //****************************************************************************

    int32_t m_errorStatus{0x00};
    ResultsRangesDataType m_resultsRanges{};

    std::condition_variable* m_LWMSendRequestCondVar{nullptr};
    std::mutex* m_LWMSendRequestMutex{nullptr};
    bool* m_LWMSendRequestVariable{nullptr};

    std::atomic<bool> m_sendRequestSimpleIOSelection{false};
    std::mutex m_simpleIOSelectionMutex;
    SimpleIOSelectionDataType m_simpleIOSelectionData{};
    std::atomic<bool> m_sendRequestSimpleIOStopp{false};
    std::mutex m_simpleIOStoppMutex;
    bool m_simpleIOStoppRequestAcknowledge{false};
    std::atomic<bool> m_sendRequestWatchdog{false};
    std::atomic<bool> m_watchdogTelegramSent{false};
    std::atomic<bool> m_watchdogTelegramReceived{false};
    std::atomic<bool> m_sendRequestSignOffClient{false};
    std::atomic<bool> m_backToReconnect{false};

    //****************************************************************************

    static const int runFunctionTimeout; // milliseconds
};

}
}

