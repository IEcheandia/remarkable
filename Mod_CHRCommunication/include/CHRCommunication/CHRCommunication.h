/**
 *     @file
 *     @copyright    Precitec Vision GmbH & Co. KG
 *     @author       EA
 *     @date         2017/2022
 *     @brief        serves the interface to CHRocodile device
 */

#pragma once

///////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////

#include <vector>
#include <atomic>
#include <mutex>

#include "semaphore.h"

#include "common/triggerContext.h"
#include "common/triggerInterval.h"

#include "event/sensor.proxy.h"

#include "CHRocodileLib.h"

#include "CHRCommunication/OCTDeviceConfiguration.h"

#include "viWeldHead/RingBuffer.h"

///////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////

namespace precitec
{
using namespace interface;

namespace grabber
{

///////////////////////////////////////////////////////////
// typedefs
///////////////////////////////////////////////////////////

class CHRCommunication; // forward declaration

struct DataToCHRReadThread
{
    CHRCommunication* m_pCHRCommunication;
};

struct DataToCHRCyclicTaskThread
{
    CHRCommunication* m_pCHRCommunication;
    bool isFirstStart;
};

struct DataToCHRSpectrumTransferThread
{
    CHRCommunication* m_pCHRCommunication;
    sem_t* m_pSpectrumTransferSemaphore;
};

struct DataToCHRWriteThread
{
    CHRCommunication* m_pCHRCommunication;
    sem_t* m_commandsToCHRSemaphore;
};

enum DWD_Window_Type
{
    eDWD_Window_Left,
    eDWD_Window_Right
};

const uint32_t SIGNAL_ID_PULSE_COUNTER0 = 65; // 0x0041
const uint32_t SIGNAL_ID_PULSE_COUNTER1 = 66; // 0x0042
const uint32_t SIGNAL_ID_PULSE_COUNTER2 = 67; // 0x0043
const uint32_t SIGNAL_ID_PULSE_COUNTER3 = 68; // 0x0044
const uint32_t SIGNAL_ID_PULSE_COUNTER4 = 69; // 0x0045
const uint32_t SIGNAL_ID_SAMPLE_COUNTER = 83; // 0x0053
const uint32_t SIGNAL_ID_DISTANCE1 = 256;     // 0x0100
const uint32_t SIGNAL_ID_QUALITY1 = 257;      // 0x0101
const uint32_t SIGNAL_ID_MEDIAN1 = 260;       // 0x0104
const uint32_t SIGNAL_ID_DISTANCE2 = 264;     // 0x0108
const uint32_t SIGNAL_ID_QUALITY2 = 265;      // 0x0109
const uint32_t SIGNAL_ID_MEDIAN2 = 268;       // 0x010C

const uint32_t SIGNAL_ID_DISTANCE1_INT = 16640; // 0x4100
const uint32_t SIGNAL_ID_QUALITY1_INT = 16641;  // 0x4101
const uint32_t SIGNAL_ID_MEDIAN1_INT = 16644;   // 0x4104
const uint32_t SIGNAL_ID_DISTANCE2_INT = 16648; // 0x4108
const uint32_t SIGNAL_ID_QUALITY2_INT = 16649;  // 0x4109
const uint32_t SIGNAL_ID_MEDIAN2_INT = 16652;   // 0x410C

///////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////

// TODO - Copyable/Movable? Power of two size?
struct CHRCircularLineBuffer
{
    explicit CHRCircularLineBuffer(size_t maxLines, size_t sampleSizeInBytes)
        : m_maxLines(maxLines)
        , m_sampleSizeInBytes(sampleSizeInBytes)
        , m_buffer(maxLines * sampleSizeInBytes)
    {
    }

    void AddLines(size_t numLines, const double* linesData)
    {
        {
            std::lock_guard<std::mutex> guard(m_mutex);

            if (numLines > m_maxLines)
            {
                // TODO we could dynamically increase the buffer here.
                throw std::runtime_error("CHRCircularLineBuffer data size exceeds buffer size");
            }

            if (m_writer + numLines - m_reader >= m_maxLines)
            {
                // TODO - Follow the convention in Weldmaster.
                throw std::runtime_error("CHRCircularLineBuffer writer faster than reader");
            }
        }

        size_t startLine = m_writer % m_maxLines;

        if (startLine + numLines <= m_maxLines) // Kein Wraparound
        {

            size_t writeIndex = startLine * m_sampleSizeInBytes;
            std::memcpy(m_buffer.data() + writeIndex, linesData, numLines * m_sampleSizeInBytes);
        }
        else
        {
            auto linesTillEnd = m_maxLines - startLine;
            auto dataSizeTillEnd = linesTillEnd * m_sampleSizeInBytes;
            size_t writeIndex = startLine * m_sampleSizeInBytes;
            std::memcpy(m_buffer.data() + writeIndex, linesData, dataSizeTillEnd);

            auto remainingLines = numLines - linesTillEnd;
            auto dataSize = remainingLines * m_sampleSizeInBytes;
            std::memcpy(m_buffer.data(), linesData + (dataSizeTillEnd / sizeof(double)), dataSize);
        }

        std::lock_guard<std::mutex> guard(m_mutex);
        m_writer += numLines;
    }

    // numLines enthaelt die Anzahl der erwuenschten Linien. Falls weniger verfuegbar sind, wird numLines
    // entsprechend geaendert.
    void GetLinesData(size_t* numLines, const double** data) const noexcept
    {
        std::lock_guard<std::mutex> guard(m_mutex);

        size_t availableLines = m_writer - m_reader;
        if (availableLines < *numLines)
            *numLines = availableLines;

        *data = reinterpret_cast<const double*>(m_buffer.data() + (m_reader % m_maxLines) * m_sampleSizeInBytes);
    }

    // Pointers zu Linien zurueckgegeben.
    std::vector<const double*> GetLines(size_t numLines) const
    {
        std::lock_guard<std::mutex> guard(m_mutex);

        size_t availableLines = m_writer - m_reader;
        if (availableLines < numLines)
            numLines = availableLines;

        std::vector<const double*> lines;

        if (numLines == 0)
            return lines;

        auto start = m_reader;
        lines.resize(numLines);
        for (size_t i = 0; i < numLines; ++i)
        {
            lines[i] = reinterpret_cast<const double*>(m_buffer.data() + (start++ % m_maxLines) * m_sampleSizeInBytes);
        }

        return lines;
    }

    // Aufgerufen nach der Verarbeitung der letzen Lines
    void AdvanceReader(size_t numLines)
    {
        std::lock_guard<std::mutex> guard(m_mutex);

        m_reader += numLines;
    }

    void Reset()
    {
        std::lock_guard<std::mutex> guard(m_mutex);

        m_reader = m_writer;
    }

private:
    uint64_t m_reader = 0;
    uint64_t m_writer = 0;
    size_t m_maxLines = 0;
    size_t m_sampleSizeInBytes = 0;
    // Raw Puffer fuer die Daten.
    std::vector<uint8_t> m_buffer;
    mutable std::mutex m_mutex;
};

class CHRCommunication
{

public:
    CHRCommunication(TSensor<AbstractInterface>& p_rSensorProxy);
    virtual ~CHRCommunication(void);

    void InitCHRCommunication(void);

    void StartCHRReadThread(void);
    void CHRReadFunction(void);

    void StartCHRCyclicTaskThread(bool isFirstStart);
    void StopCHRCyclicTaskThread(void);
    void IncomingIDMWeldingDepth(void);
    void incomingIDMQualityPeak1(void);
    void IncomingCLSLines(void);

    void StartCHRSpectrumTransferThread(void);
    void IncomingCHRSpectrumLine(void);

    void StartCHRWriteThread(void);
    void CHRWriteFunction(void);

    // functions for DeviceServer

    interface::SmpKeyValue getKeyValue(std::string key) const;
    interface::Configuration makeConfiguration() const;
    void SetNumberOfChannels(int p_oValue);
    void SetSampleFrequency(int p_oValue);
    void SetLampIntensity(int p_oValue);
    void SetResultsOnOff(bool p_oOnOff);
    bool GetResultsOnOff(void) { return m_oResultsOnOff; }
    void AskVersionString(void);
    void setLoggingVerbosity(int value);
    void SetDetectionWindow(DWD_Window_Type p_oType, int p_oValue);
    void SetQualityThreshold(int p_oValue);
    void SetDataAveraging(int p_oValue);
    void SetSpectralAveraging(int p_oValue);
    void SetNumberOfPeaks(int p_oValue);

    void SetDarkReference(void);
    void SetFastDarkReference(void);
    void SetEqualizeNoise(void);
    void SetRefreshOfDarkRef(void);
    void SetSaveSetup(void);
    void PerformDarkReference(void);

    void SetSLDDimmerOnOff(bool p_oOnOff);
    bool GetSLDDimmerOnOff(void);
    void SetAdaptiveExposureOnOff(bool p_oOnOff);
    bool GetAdaptiveExposureOnOff(void);
    void SetAdaptiveExposureBasicValue(int p_oValue);
    int GetAdaptiveExposureBasicValue(void);
    void SendAdaptiveExposureData(void);

    void queryScale();
    void setScale(int value);
    int getScale(void) { return m_scale; }

    void SetSignalSubscription(void);
    void DownloadRawSpectrum(void);
    void DownloadChromaticSpectrum(void);
    void DownloadFFTSpectrum(void);

    void SetDirectCHRCommand(const PvString& p_oCommand);

    void SetDebugInfoCHRResponseOnOff(bool p_oValue) { m_oDebugInfoCHRResponse = p_oValue; }
    bool GetDebugInfoCHRResponseOnOff(void) { return m_oDebugInfoCHRResponse; }

    void TestFunction2(void);
    void TestFunction3(void);
    void TestFunction4(void);
    void TestFunction5(void);
    void TestFunction6(void);
    int GetRescaleFactorCHRResults() const { return m_oRescaleFactorCHRResults; }
    void SetRescaleFactorCHRResults(int p_oValue);
    void SetWeldingDepthSystemOffset(int p_oValue);
    void saveConfigurationToFile();
    void SetMaxNumProfilesToSend(int p_oValue);
    void SetMaxBufferLines(int p_oValue);

    bool isIDMDevice1Enabled(void) { return m_oIDMDevice1Enable; }
    bool isCLS2Device1Enabled(void) { return m_oCLS2Device1Enable; }

    // interface triggerCmd
    void burst(const std::vector<int>& ids, TriggerContext const& context, TriggerInterval const& interval);
    void cancel(int id);

    // interface inspection
    void startAutomaticmode(uint32_t producttype, uint32_t productnumber); // Interface inspection
    void stopAutomaticmode(void);                                          // Interface inspection
    void start(int seamnumber);                                            // Interface inspection
    void end(int seamnumber);                                              // Interface inspection

    void CommandResponseOK(void);
    void CheckCommandResponse(void);

    std::atomic_uint m_oTriggerDistance_ns;

    std::mutex m_IDMDistancePeak1RingBufferMutex;
    ethercat::RingBuffer<uint32_t> m_IDMDistancePeak1RingBuffer;
    std::mutex m_IDMQualityPeak1RingBufferMutex;
    ethercat::RingBuffer<uint32_t> m_IDMQualityPeak1RingBuffer;

    const static int m_oNumberOfLines = 40;
    const static int m_oMaxNumberOfValues = 1200;

    std::mutex m_oSpectrumTransferMutex;
    sem_t m_oSpectrumTransferSemaphore;
    unsigned int m_oSpectrumArray[m_oNumberOfLines][m_oMaxNumberOfValues];
    unsigned int m_oSpectrumLength[m_oNumberOfLines];
    bool m_oSpectrumIsFree[m_oNumberOfLines];
    uint32_t m_oSpectrumIdxToSend;
    uint32_t m_oSpectrumIdxToWrite;

    bool m_oSpectrumActive;

    std::mutex m_oCommandsToCHRMutex;
    std::queue<std::string> m_oCommandsToCHR{};
    sem_t m_commandsToCHRSemaphore;

    FILE* m_pResultDebugFile{nullptr};

    bool m_oDebugInfoCHRResponse{false};
    bool m_oDebugInfoSpectrumFile{false};
    bool m_oDebugInfoSampleSignals{false};
    bool m_oDebugInfoSampleData{false};

    std::unique_ptr<CHRCircularLineBuffer> m_pLineBuffer;
    TSampleSignalGeneralInfo m_sampleSigGenInfo = {0, 0, 0, 0};
    std::vector<TSampleSignalInfo> m_signalInfo;

    std::atomic<int> m_dataAveragingBuffer{1};
    std::atomic<bool> m_checkSampleCounter{false};
    std::atomic<bool> m_firstSampleCounter{true};

    std::atomic<bool> m_firstCallBackArrived{false};

private:
    bool m_oIDMDevice1Enable;
    std::string m_oIDMDevice1IpAddress{"192.168.170.2"};
    bool m_oCLS2Device1Enable;
    std::string m_oCLS2Device1IpAddress{"192.168.170.3"};

    Conn_h m_oCHRHandle{std::numeric_limits<Conn_h>::max()};

    void cleanBuffer();
    void initBufferWeldDepth();
    void initBufferQualityPeak1();
    void initBufferTrackingLine();
    void initBufferSpectrum();

    void ErrorInCHRFunction(const Res_t p_oRes, const std::string& p_oFunctionName);

    void initConfiguration();

    void insertInCommandsToCHR(const std::string& commandString);

    pthread_t m_oCHRReadThread_ID;
    struct DataToCHRReadThread m_oDataToCHRReadThread;

    pthread_t m_oCHRCyclicTaskThread_ID;
    struct DataToCHRCyclicTaskThread m_oDataToCHRCyclicTaskThread;

    pthread_t m_oCHRSpectrumTransferThread_ID;
    struct DataToCHRSpectrumTransferThread m_oDataToCHRSpectrumTransferThread;

    pthread_t m_oCHRWriteThread_ID;
    struct DataToCHRWriteThread m_oDataToCHRWriteThread;

    OCTDeviceConfiguration m_oConfiguration;
    bool m_oResultsOnOff;
    int m_scale = 1;

    TSensor<AbstractInterface>& m_rSensorProxy;

    std::map<Sensor, bool> m_oSensorIdsEnabled;

    std::mutex m_oBurstDataMutex;
    TriggerContext m_oTriggerContext;
    TriggerInterval m_oTriggerInterval;

    int m_oImageNr_IDMWeldDepth;
    system::TSmartArrayPtr<int>::ShArrayPtr* m_pValues_IDMWeldDepth;

    int m_imageNoIDMQualityPeak1;
    system::TSmartArrayPtr<int>::ShArrayPtr* m_valuesIDMQualityPeak1;

    int m_oSendSamples_CHRSpectrumLine;
    int m_oImageNr_CHRSpectrumLine;
    system::TSmartArrayPtr<int>::ShArrayPtr* m_pValues_CHRSpectrumLine;
    int m_oSendSamples_CLSLines;
    int m_oImageNr_CLSLines;

    bool m_oCycleIsOn;
    bool m_oSeamIsOn;
    bool m_oLiveModeIsOn;

    bool m_oSLDDimmerOnOff;
    bool m_oAdaptiveExposureOnOff;
    int m_oAdaptiveExposureBasicValue;

    int m_oRescaleFactorCHRResults; // divide by m_oRescaleFactorCHRResults: bring values into the range of image pixels
    image::BImage m_o2DScanImage;
    bool m_oConfigurationModified; //flag to decide if write to file is needed
    std::string m_oConfigFile;

    std::atomic<bool> m_oWaitingForCommandResponse;
    std::atomic<uint32_t> m_oWaitingForCommandResponseCounter;
    std::atomic<uint32_t> m_oWaitingForCommandResponseLimit;
};

} // namespace grabber

} // namespace precitec
