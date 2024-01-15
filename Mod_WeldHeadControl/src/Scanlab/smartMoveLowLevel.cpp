/**
 *  @file
 *  @copyright  Precitec Vision GmbH & Co. KG
 *  @author     Alexander Egger (EA)
 *  @date       2023
 *  @brief      Communicates via TCP/IP wuth a SmartMove scanner
 */

#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <chrono>
#include <vector>
#include <fcntl.h>

#include "common/systemConfiguration.h"
#include "module/moduleLogger.h"

#include "viWeldHead/Scanlab/smartMoveLowLevel.h"

namespace precitec
{
namespace hardware
{

using namespace interface;

const int ANSWER_BUFFER_LEN{1000};
const int MCI_TIMEOUT_SYMBOL{100'000}; // 100ms
const int MCI_TIMEOUT_SYMBOLLIST{2'000'000}; // 2s
const int XMODEM_RXTIMEOUT{10'000'000}; // 10s

SmartMoveLowLevel::SmartMoveLowLevel(void)
{
    m_scanner2DControllerIpAddress = SystemConfiguration::instance().get(SystemConfiguration::StringKey::Scanner2DController_IP_Address);
    wmLog(eDebug, "m_scanner2DControllerIpAddress: <%s>\n", m_scanner2DControllerIpAddress.c_str());

    int returnValue = inet_pton(AF_INET, m_scanner2DControllerIpAddress.c_str(), &m_scanner2DControllerIpAddressNetwork);
    if (returnValue != 1)
    {
        wmLogTr(eError, "QnxMsg.VI.TCPWrongIPFormat", "wrong format of IP address string\n");
        m_scanner2DControllerIpAddressNetwork = 0x69AAA8C0; // 192.168.170.105 in network byte order
    }
    char helpStrg[21];
    sprintf(helpStrg, "%08X", m_scanner2DControllerIpAddressNetwork);
    wmLog(eDebug, "m_scanner2DControllerIpAddressNetwork: %s, retVal: %d\n", helpStrg, returnValue); // in network byte order
}

SmartMoveLowLevel::~SmartMoveLowLevel()
{
}

int SmartMoveLowLevel::openTcpConnection(void)
{
    struct sockaddr_in serverAddr;
    if (m_sockDesc != -1)
    {
        // already open
        return 0;
    }

    m_sockDesc = socket(AF_INET, SOCK_STREAM, 0);
    if (m_sockDesc == -1)
    {
        wmLog(eDebug, "SmartMoveLowLevel: Error while creating socket: %s\n", strerror(errno));
        wmFatal(eExtEquipment, "QnxMsg.VI.InitTCPCommFault", "Problem while initializing TCP/IP communication %s\n", "(101)");
        return -1;
    }

    struct timeval tcpTimeout;
    // tcpTimeout must be significant lower than 1 sec
    tcpTimeout.tv_sec = 0;
    tcpTimeout.tv_usec = 500000;
    setsockopt(m_sockDesc, SOL_SOCKET, SO_SNDTIMEO, (const char*)&tcpTimeout, sizeof tcpTimeout);
    setsockopt(m_sockDesc, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tcpTimeout, sizeof tcpTimeout);

    // TODO ist the following part necessary ?
    int flag = 1;
    int returnValue = setsockopt(m_sockDesc, IPPROTO_TCP, TCP_NODELAY, (char *) &flag, sizeof(flag));
    if (returnValue == -1)
    {
        wmLog(eDebug, "SmartMoveLowLevel: Error while setting socket options: %s\n", strerror(errno));
        wmFatal(eExtEquipment, "QnxMsg.VI.InitTCPCommFault", "Problem while initializing TCP/IP communication %s\n", "(102)");
        return -1;
    }

    memset((char *)&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    memcpy(&serverAddr.sin_addr, &m_scanner2DControllerIpAddressNetwork, 4); // in network byte order
    serverAddr.sin_port = htons(m_scanner2DControllerServerPort);

    // connect to Server
    if (connect(m_sockDesc, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1)
    {
        if (errno == ECONNREFUSED)
        {
            wmLogTr(eWarning, "QnxMsg.VI.TCPCommNoConn", "connection to server not possible %s\n", "(001)");
            close (m_sockDesc);
            sleep(2);
            return -1;
        }
        else if (errno == ETIMEDOUT)
        {
            wmLogTr(eWarning, "QnxMsg.VI.TCPCommNoConn", "connection to server not possible %s\n", "(002)");
            close (m_sockDesc);
            return -1;
        }
        else
        {
            wmLog(eDebug, "SmartMoveLowLevel: Error while connecting server: %s\n", strerror(errno));
            close (m_sockDesc);
            wmFatal(eExtEquipment, "QnxMsg.VI.InitTCPCommFault", "Problem while initializing TCP/IP communication %s\n", "(103)");
            return -1;
        }
    }
    wmLogTr(eInfo, "QnxMsg.VI.TCPCommStarted", "TCP/IP Connection started %s\n", "(MarkingEngine)");

    /* flush input */
    int loop{0};
    char buffer[ANSWER_BUFFER_LEN + 1];
    while (recv(m_sockDesc, buffer, ANSWER_BUFFER_LEN, 0) > 0)
    {
        loop++;
    }

    std::string answer{};
    /* send an empty line for syncronization */
    setMEParameter("", answer, MCI_TIMEOUT_SYMBOL);
    usleep(10 * 1000);
    setMEParameter("", answer, MCI_TIMEOUT_SYMBOL);
    usleep(10 * 1000);

    return 0;
}

void SmartMoveLowLevel::closeTcpConnection(void)
{
    close(m_sockDesc);
    m_sockDesc = -1;
}

TCPSendStatus SmartMoveLowLevel::processJob(const std::string& processJobCommand)
{
    std::string answer{};

    if (processJobCommand.empty())
    {
        wmLog(eError, "Job process is empty!\n");
        return TCPSendStatus::Failed;
    }

    answer.reserve(processJobCommand.size());

    setMEParameter(processJobCommand, answer, MCI_TIMEOUT_SYMBOL);

    if (answer != processJobCommand)
    {
        return TCPSendStatus::Failed;
    }
    return TCPSendStatus::Successful;
}

TCPSendStatus SmartMoveLowLevel::selectJob(const std::string& selectJobCommand)
{
    std::string answer{};

    if (selectJobCommand.empty())
    {
        wmLog(eError, "Job select is empty!\n");
        return TCPSendStatus::Failed;
    }

    answer.reserve(selectJobCommand.size());

    setMEParameter(selectJobCommand, answer, MCI_TIMEOUT_SYMBOL);

    if (answer != selectJobCommand)
    {
        return TCPSendStatus::Failed;
    }
    return TCPSendStatus::Successful;
}

int SmartMoveLowLevel::readMEParameter(const std::string& command, std::string& answer, int timeout)
{
    char buffer[ANSWER_BUFFER_LEN + 1];
    size_t bytesRead;

    std::string sendCommand{command + "\r\n"};
    if (send(m_sockDesc, sendCommand.c_str(), sendCommand.size(), 0) == -1)
    {
        wmLog(eDebug, "SmartMoveLowLevel: Error while sending data: %s\n", strerror(errno));
        wmFatal(eExtEquipment, "QnxMsg.VI.TCPCommFault", "Problem while establishing TCP/IP communication %s\n", "(101)");
        return -1;
    }

    if (readToPrompt(buffer, ANSWER_BUFFER_LEN, &bytesRead, timeout) < 0)
    {
        wmLog(eDebug, "SmartMoveLowLevel: Error while receiving data\n");
        wmFatal(eExtEquipment, "QnxMsg.VI.TCPCommFault", "Problem while establishing TCP/IP communication %s\n", "(102)");
        return -2;
    }
    buffer[bytesRead] = '\0';
    std::string inputString{buffer};
    auto first = inputString.find("\r\n");
    answer = inputString.substr(0, first);

    return 0;
}

int SmartMoveLowLevel::setMEParameter(const std::string& command, std::string& answer, int timeout)
{
    char buffer[ANSWER_BUFFER_LEN + 1];
    size_t bytesRead;

    std::string sendCommand{command + "\r\n"};
    if (send(m_sockDesc, sendCommand.c_str(), sendCommand.size(), 0) == -1)
    {
        wmLog(eDebug, "SmartMoveLowLevel: Error while sending data: %s\n", strerror(errno));
        wmFatal(eExtEquipment, "QnxMsg.VI.TCPCommFault", "Problem while establishing TCP/IP communication %s\n", "(103)");
        return -1;
    }

    if (readToPrompt(buffer, ANSWER_BUFFER_LEN, &bytesRead, timeout) < 0)
    {
        wmLog(eDebug, "SmartMoveLowLevel: Error while receiving data\n");
        wmFatal(eExtEquipment, "QnxMsg.VI.TCPCommFault", "Problem while establishing TCP/IP communication %s\n", "(104)");
        return -2;
    }
    buffer[bytesRead] = '\0';
    std::string inputString{buffer};
    auto first = inputString.find("\r\n");
    answer = inputString.substr(0, first);

    return 0;
}

int SmartMoveLowLevel::readToPrompt(char* answer, size_t maxlen, size_t *nread, int timeout)
{
    const auto startTime{std::chrono::steady_clock::now()};

    *nread = 0;
    char buffer[200];
    ssize_t bytesRead;
    while (true)
    {
        bytesRead = recv(m_sockDesc, buffer, sizeof(buffer), 0);
        if (bytesRead > 0)
        {
            for (ssize_t i = 0; i < bytesRead; i++)
            {
                if (buffer[i] == '>')
                {
                    return 0;
                }
                answer[(*nread)++] = buffer[i];
                if (*nread >= maxlen)
                {
                    return -1;
                }
            }
        }
        const auto waited = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - startTime);
        if (waited.count() > timeout)
        {
            return -1;
        }
        usleep(2 * 1000);
    }
}

int SmartMoveLowLevel::askForVersion(void)
{
    std::string answer{};

    readMEParameter("G INFO_SERIAL", answer, MCI_TIMEOUT_SYMBOL);
    wmLog(eDebug, "G INFO_SERIAL: %s\n", answer.c_str());
    usleep(10 * 1000);
    readMEParameter("G INFO_CONFIG", answer, MCI_TIMEOUT_SYMBOL);
    wmLog(eDebug, "G INFO_CONFIG: %s\n", answer.c_str());
    usleep(10 * 1000);
    readMEParameter("G INFO_VERSION", answer, MCI_TIMEOUT_SYMBOL);
    wmLog(eDebug, "G INFO_VERSION: %s\n", answer.c_str());
    usleep(10 * 1000);
    readMEParameter("G INFO_PLD_CONFIG", answer, MCI_TIMEOUT_SYMBOL);
    wmLog(eDebug, "G INFO_PLD_CONFIG: %s\n", answer.c_str());
    usleep(10 * 1000);
    readMEParameter("G INFO_PLD_VERSION", answer, MCI_TIMEOUT_SYMBOL);
    wmLog(eDebug, "G INFO_PLD_VERSION: %s\n", answer.c_str());
    usleep(10 * 1000);
    readMEParameter("G INFO_BOOT_CONFIG", answer, MCI_TIMEOUT_SYMBOL);
    wmLog(eDebug, "G INFO_BOOT_CONFIG: %s\n", answer.c_str());
    usleep(10 * 1000);
    readMEParameter("G INFO_BOOT_VERSION", answer, MCI_TIMEOUT_SYMBOL);
    wmLog(eDebug, "G INFO_BOOT_VERSION: %s\n", answer.c_str());
    usleep(10 * 1000);
    readMEParameter("G RSI_INFO", answer, MCI_TIMEOUT_SYMBOL);
    wmLog(eDebug, "G RSI_INFO: %s\n", answer.c_str());
    usleep(10 * 1000);

    return 1;
}

int SmartMoveLowLevel::askForDownloadInfo(void)
{
    std::string answer{};

    readMEParameter("G RSI_USTAT", answer, MCI_TIMEOUT_SYMBOL);
    wmLog(eDebug, "G RSI_USTAT: %s\n", answer.c_str());
    usleep(10 * 1000);
    readMEParameter("G HP_VECCNT", answer, MCI_TIMEOUT_SYMBOL);
    wmLog(eDebug, "G HP_VECCNT: %s\n", answer.c_str());
    usleep(10 * 1000);

    return 1;
}

void SmartMoveLowLevel::askForMessageQueue(void)
{
    std::string answer{};

    setMEParameter("D", answer, MCI_TIMEOUT_SYMBOL);
    wmLog(eDebug, "ME MessageQueue: %s\n", answer.c_str());
}

void SmartMoveLowLevel::askForSymbolList(void)
{
    std::string answer{};

    readMEParameter("T", answer, MCI_TIMEOUT_SYMBOLLIST);
}

int SmartMoveLowLevel::doSoftwareReset(void)
{
    wmLog(eDebug, "SmartMoveLowLevel::doSoftwareReset start\n");

    // send reset command
    std::string downloadCommand{"R\r\n"};
    if (send(m_sockDesc, downloadCommand.c_str(), downloadCommand.size(), 0) == -1)
    {
        wmLog(eDebug, "SmartMoveLowLevel: Error while sending data: %s\n", strerror(errno));
        wmFatal(eExtEquipment, "QnxMsg.VI.TCPCommFault", "Problem while establishing TCP/IP communication %s\n", "(105)");
        return -1;
    }

    sleep(2);

    // waiting for answer
    int inputBuffer{};
    int returnValue{};
    std::string answerDownload{};
    returnValue = recv(m_sockDesc, &inputBuffer, 1, 0);
    if (returnValue == -1)
    {
        wmLog(eDebug, "SmartMoveLowLevel: after first receive error (%s)\n", strerror(errno));
    }
    else
    {
        answerDownload.push_back(inputBuffer);
    }
    while (returnValue != -1)
    {
        returnValue = recv(m_sockDesc, &inputBuffer, 1, 0);
        if (returnValue == -1)
        {
            wmLog(eDebug, "SmartMoveLowLevel: after next receive error  (%s)\n", strerror(errno));
        }
        else
        {
            answerDownload.push_back(inputBuffer);
            if (static_cast<char>(inputBuffer & 0xFF) == '>')
            {
                break;
            }
        }
    }
    auto first = answerDownload.find("\r\n");
    std::string answer = answerDownload.substr(0, first);
    if (answer == std::string{"R"})
    {
        wmLog(eDebug, "SmartMoveLowLevel::doSoftwareReset successful\n");
        return 0;
    }
    else
    {
        wmLog(eDebug, "SmartMoveLowLevel::doSoftwareReset not successful\n");
        return -1;
    }
}

int SmartMoveLowLevel::setJumpSpeed(const int pen, const float speed)
{
    std::string answer{};

    std::string command{"S MV_JUMPSPEED." + std::to_string(pen) + " " + std::to_string(speed)};
    setMEParameter(command, answer, MCI_TIMEOUT_SYMBOL);
    if (answer != command)
    {
        wmLog(eDebug, "SmartMoveLowLevel: answer is NOT ident to command: [%s]\n", answer.c_str());
        return 0;
    }
    return 1;
}

bool SmartMoveLowLevel::sendFocalLength(const std::string& focalLengthOrder)
{
    return sendCommand(focalLengthOrder, "Focal length order is empty!");
}

bool SmartMoveLowLevel::sendScanfieldSize(const std::string& scanfieldSizeOrder)
{
    return sendCommand(scanfieldSizeOrder, "Scan field size order is empty!");
}

bool SmartMoveLowLevel::sendForceEnable(const std::string& forceEnable)
{
    return sendCommand(forceEnable, "Force enable is empty!");
}

bool SmartMoveLowLevel::sendCommand(const std::string& command, const std::string& errorLogMessage)
{
    std::string answer{};

    if (command.empty())
    {
        wmLog(eWarning, "%s\n", errorLogMessage);
        return false;
    }

    answer.reserve(command.size());

    setMEParameter(command, answer, MCI_TIMEOUT_SYMBOL);

    if (answer != command)
    {
        return false;
    }
    return true;
}

bool SmartMoveLowLevel::uploadFilePermanentlyToFlash(const std::string& filename)
{
    auto fileDescriptor = openFile(filename);
    if (fileDescriptor == -1)
    {
        wmLog(eError, "File is missing!\n");
        return false;
    }

    if (!resetFilePointer(fileDescriptor))
    {
        return false;
    }

    performHardwareReset();

    usleep(250 * 1000);     //250ms

    if (!waitForSynchronizationCharacterThenUploadFile(fileDescriptor))
    {
        wmLog(eError, "Transmittion of correction file failed\n");
        wmLog(eError, "%s\n", filename);
        return false;
    }
    wmLog(eDebug, "%s uploaded\n", filename);

    synchronizeME();

    return true;
}

int SmartMoveLowLevel::convertAnswerToNumber(const std::string& answer, const std::string& errorLogMessage)
{
    try
    {
        return std::stoi(answer);
    }
    catch (...)
    {
        wmLog(eError, "%s\n", errorLogMessage);
        return -1;
    }
}

std::string SmartMoveLowLevel::sendCalibrationFilename(const std::string& getCalibrationFilename)
{
    std::string answer{};

    if (getCalibrationFilename.empty())
    {
        wmLog(eWarning, "Calibration file command is empty!\n");
        return {};
    }

    answer.reserve(getCalibrationFilename.size());

    setMEParameter(getCalibrationFilename, answer, MCI_TIMEOUT_SYMBOL);

    return answer;
}

bool SmartMoveLowLevel::sendSystemOperationMode(const std::string& systemOperationModeOrder)
{
    return sendCommand(systemOperationModeOrder, "System operation mode is empty!");
}

bool SmartMoveLowLevel::sendInputMode(const std::string& inputModeOrder)
{
    return sendCommand(inputModeOrder, "Input mode is empty!");
}

bool SmartMoveLowLevel::sendAlignX(const std::string& alignXOrder)
{
    return sendCommand(alignXOrder, "Align X is empty!");
}

bool SmartMoveLowLevel::sendAlignY(const std::string& alignYOrder)
{
    return sendCommand(alignYOrder, "Align Y is empty!");
}

bool SmartMoveLowLevel::sendLaserOnForAlignment(const std::string& laserOnOrder)
{
    return sendCommand(laserOnOrder, "Laser on is empty!");
}

bool SmartMoveLowLevel::sendLaserOnMaxForAlignment(const std::string& laserOnMaxOrder)
{
    return sendCommand(laserOnMaxOrder, "Laser on max is empty!");
}

bool SmartMoveLowLevel::sendJumpSpeed(const std::string& jumpSpeedOrder)
{
    return sendCommand(jumpSpeedOrder, "Jump speed is empty!");
}

bool SmartMoveLowLevel::sendMarkSpeed(const std::string& markSpeedOrder)
{
    return sendCommand(markSpeedOrder, "Mark speed is empty!");
}

std::pair<bool, double> SmartMoveLowLevel::sendGetPositionFeedbackX(const std::string& getPositionFeedbackOrder)
{
    std::string answer{};

    if (getPositionFeedbackOrder.empty())
    {
        wmLog(eWarning, "Position feedback X is empty\n");
        return std::make_pair(false, 0);
    }

    setMEParameter(getPositionFeedbackOrder, answer, MCI_TIMEOUT_SYMBOL);

    if (answer.front() == '?')
    {
        return std::make_pair(false, 0);
    }

    const auto& numberString = answer.substr(getPositionFeedbackOrder.size() + 1, answer.size() - getPositionFeedbackOrder.size());

    return std::make_pair(true, std::stod(numberString));
}

std::pair<bool, double> SmartMoveLowLevel::sendGetPositionFeedbackY(const std::string& getPositionFeedbackOrder)
{
    std::string answer{};

    if (getPositionFeedbackOrder.empty())
    {
        wmLog(eWarning, "Position feedback Y is empty\n");
        return std::make_pair(false, 0);
    }

    setMEParameter(getPositionFeedbackOrder, answer, MCI_TIMEOUT_SYMBOL);

    if (answer.front() == '?')
    {
        return std::make_pair(false, 0);
    }

    const auto& numberString = answer.substr(getPositionFeedbackOrder.size() + 1, answer.size() - getPositionFeedbackOrder.size());

    return std::make_pair(true, std::stod(numberString));
}

std::optional<double> SmartMoveLowLevel::sendGetSysTs(const std::string& command)
{
    std::string answer{};

    if (command.empty())
    {
        wmLog(eWarning, "SysTs command is empty\n");
        return {};
    }

    setMEParameter(command, answer, MCI_TIMEOUT_SYMBOL);

    if (answer.front() == '?')
    {
        wmLog(eWarning, "Get SYS_TS failed\n");
        return {};
    }

    try
    {
        return std::stod(answer);
    } catch (...)
    {
        wmLog(eWarning, "Failed to parse SYS_TS %s.\n", answer);
        return {};
    }
}

int SmartMoveLowLevel::openFile(const std::string& filePath)
{
    return open(filePath.data(), O_RDONLY);
}

int SmartMoveLowLevel::closeFile(int fileDescriptor)
{
    return close(fileDescriptor);
}

int SmartMoveLowLevel::transmitMarkingFileXModem(int fileDescriptor, std::string& answer)
{
    return transmitFileXModem("U 1", fileDescriptor, answer);
}

int SmartMoveLowLevel::transmitPenFileXModem(int fileDescriptor, std::string& answer)
{
    return transmitFileXModem("U 3", fileDescriptor, answer);
}

int SmartMoveLowLevel::transmitFileXModem(const std::string& command, int fileDescriptor, std::string& answer)
{
    wmLog(eDebug, "transmitFileXModem: start\n");

    // reset file pointer
    if (lseek(fileDescriptor, (off_t) 0, SEEK_SET) == (off_t) -1)
    {
        wmLog(eDebug, "SmartMoveLowLevel: Error while doing lseek: %s\n", strerror(errno));
        wmFatal(eExtEquipment, "QnxMsg.VI.TCPCommFault", "Problem while establishing TCP/IP communication %s\n", "(106)");
        return -1;
    }

    // send download command
    std::string downloadCommand{command + "\r\n"};
    if (send(m_sockDesc, downloadCommand.c_str(), downloadCommand.size(), 0) == -1)
    {
        wmLog(eDebug, "SmartMoveLowLevel: Error while sending data: %s\n", strerror(errno));
        wmFatal(eExtEquipment, "QnxMsg.VI.TCPCommFault", "Problem while establishing TCP/IP communication %s\n", "(107)");
        return -1;
    }

    m_xmodemParameter.MaxRetrans = 3;
    m_xmodemParameter.PacketSize = XMODEM_BUFSIZE_EXTENDED;
    m_xmodemStatus = xmodemReset();

    // read file content
    int fileChar{0};
    int success{1};
    while ((read(fileDescriptor, &fileChar, 1) == 1) && (success == 1))
    {
        success = xmodemWriteChar(&fileChar);
    }
    // finish the upload
    xmodemWriteChar(nullptr);

    char buffer[ANSWER_BUFFER_LEN + 1];
    size_t bytesRead;
    if (readToPrompt(buffer, ANSWER_BUFFER_LEN, &bytesRead, MCI_TIMEOUT_SYMBOL) < 0)
    {
        wmLog(eDebug, "SmartMoveLowLevel: Error while receiving data\n");
        wmFatal(eExtEquipment, "QnxMsg.VI.TCPCommFault", "Problem while establishing TCP/IP communication %s\n", "(108)");
        return -1;
    }
    buffer[bytesRead] = '\0';
    std::string answerDownload(buffer);
    auto first = answerDownload.find("\r\n");
    auto second = answerDownload.find("\r\n", (first + strlen("\r\n")));
    auto length = second - (first + strlen("\r\n"));
    answer = answerDownload.substr((first + strlen("\r\n")), length);

    wmLog(eDebug, "transmitFileXModem: end\n");
    return 0;
}

int SmartMoveLowLevel::xmodemOutBlock(int* data, int bytesToSend)
{
    int bytesSent{0};
    while (bytesSent < bytesToSend)
    {
        int returnValue = send(m_sockDesc, data + bytesSent, 1, 0);
        if (returnValue == -1)
        {
            wmLog(eDebug, "SmartMoveLowLevel: Error while sending data: %s\n", strerror(errno));
            wmFatal(eExtEquipment, "QnxMsg.VI.TCPCommFault", "Problem while establishing TCP/IP communication %s\n", "(109)");
            return -1;
        }
        bytesSent += returnValue;
    }
    return bytesToSend;
}

int SmartMoveLowLevel::xmodemInpByte(int* inputByte, unsigned int timeout)
{
    const auto startTime{std::chrono::steady_clock::now()};

    unsigned char characterRead;
    int loop{0};
    while (1)
    {
        loop++;
        if (recv(m_sockDesc, &characterRead, 1, 0) == 1)
        {
            *inputByte = (int) characterRead;
            return 1;
        }
        const auto waited = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - startTime);
        if (waited.count() > timeout)
        {
            return 0;
        }
        usleep(2 * 1000);
    }
}

int SmartMoveLowLevel::xmodemWriteChar(int *writePtr)
{
    static int writecount = 0;
    int rcnt;

    /* EOF means reset word stream when we are not transmitting */
    if (m_xmodemStatus != XMODEM_STATUS_TRANSMIT && writePtr == nullptr)
    {
        writecount = 0;
        return 0;
    }

    if (xmodemSync() == -1)
    {
        return 0;
    }

    /* if we start a new packet, fill the buffer with EOFCHARs */
    if (writecount == 0)
    {
        memset(&m_xmodemPacketBuf[0], XMODEM_EOFCHAR, (int)m_xmodemParameter.PacketSize);
    }

    /* if stream is flushed, send the packet now */
    if (writePtr == nullptr)
    {
        xmodemSendPacket(&m_xmodemPacketBuf[0], m_xmodemParameter.PacketSize);
        m_xmodemStatus = xmodemEOT();
        return 0;
    }

    /* add the byte to buffer */
    m_xmodemPacketBuf[writecount++] = 0xFF & (*writePtr);

    /* calculate remaining number of bytes in buffer */
    rcnt = (int) m_xmodemParameter.PacketSize - writecount;

    if (rcnt == 0)
    {
        writecount = 0;
        if (xmodemSendPacket(&m_xmodemPacketBuf[0], m_xmodemParameter.PacketSize) != XMODEM_STATUS_TRANSMIT)
        {
            return 0;
        }
    }
    return 1;
}

int SmartMoveLowLevel::xmodemSync(void)
{
    if (m_xmodemStatus == XMODEM_STATUS_INIT)
    {
        // This is original code from the SmartMove sample code
        // This codes causes an unneccessary delay in X-Modem transfer
        // I would like to keep this code for documenting reasons
        //int b;
        //
        //for (int i = 0; i < 3; i++)
        //{
        //    if ((*m_xmodemParameter.InpByte)(&b, 1000000) == 1)
        //    {
        //        if (b == XMODEM_TRYCHAR)
        //        {
        //            m_xmodemStatus = XMODEM_STATUS_TRANSMIT;
        //            return 0;
        //        }
        //    }
        //}
        //
        //fprintf(stderr, "xmodemSync error\n");
        //m_xmodemStatus = XMODEM_STATUS_ERROR;
        //return -1;

        // following is the alternative code
        m_xmodemStatus = XMODEM_STATUS_TRANSMIT;
        return 0;
    }

    if (m_xmodemStatus == XMODEM_STATUS_TRANSMIT)
    {
        return 0;
    }

    return -1;
}

XmodemStatus SmartMoveLowLevel::xmodemReset(void)
{
    m_xmodemPacketNo = 1 ;
    m_xmodemStatus = XMODEM_STATUS_INIT;
    xmodemFlushInput();

    /* reset word stream */
    xmodemWriteChar(nullptr);

    return m_xmodemStatus;
}

void SmartMoveLowLevel::xmodemFlushInput(void)
{
    int garbage;

    int timeout{1000000};
    while (xmodemInpByte(&garbage, timeout) > 0)
    {
        if (timeout == 1000000)
        {
            timeout = 0;
        }
    }
}

XmodemStatus SmartMoveLowLevel::xmodemEOT(void)
{
    int cnt;
    int res;
    int val{XMODEM_EOT};

    /* send EOT */
    cnt = xmodemOutBlock(&val, 1);
    if (cnt != 1)
    {
        wmLog(eDebug, "SmartMoveLowLevel: Error while transmitting data\n");
        wmFatal(eExtEquipment, "QnxMsg.VI.TCPCommFault", "Problem while establishing TCP/IP communication %s\n", "(110)");
        m_xmodemStatus = XMODEM_STATUS_ERROR;
        return m_xmodemStatus;
    }

    /* read the acknowledge */
    cnt = xmodemInpByte(&res, XMODEM_RXTIMEOUT);

    if (cnt < 1)
    {
        wmLog(eDebug, "SmartMoveLowLevel: Timeout while receiving data\n");
        wmFatal(eExtEquipment, "QnxMsg.VI.TCPCommFault", "Problem while establishing TCP/IP communication %s\n", "(111)");
        m_xmodemStatus = XMODEM_STATUS_ERROR;
        return m_xmodemStatus;
    }

    if ((res & 0xFF) == XMODEM_ACK)
    {
        m_xmodemStatus = XMODEM_STATUS_EOT;
        return m_xmodemStatus;
    }

    m_xmodemStatus = XMODEM_STATUS_ERROR;
    return m_xmodemStatus;
}

XmodemStatus SmartMoveLowLevel::xmodemSendPacket(int *data, XmodemBufferSize bufsize)
{
    int cnt;
    int nretrans;
    int res;

    if (xmodemSync() == -1)
    {
        return m_xmodemStatus;
    }

    /* build header */
    int header[3]{};
    if (bufsize == XMODEM_BUFSIZE_EXTENDED)
    {
        header[0] = XMODEM_STX;
    }
    else
    {
        header[0] = XMODEM_SOH;
    }
    header[1] = m_xmodemPacketNo;
    header[2] = 0xFF & (~m_xmodemPacketNo);

    /* calculate CRC */
    int crc{0};
    crc = xmodemCalcCRC(data, (int)bufsize);

    nretrans = 0;
    while (nretrans <= m_xmodemParameter.MaxRetrans)
    {
        /* write the header */
        cnt = xmodemOutBlock(&header[0], 3);
        if (cnt != 3)
        {
            wmLog(eDebug, "SmartMoveLowLevel: Error while transmitting data\n");
            wmFatal(eExtEquipment, "QnxMsg.VI.TCPCommFault", "Problem while establishing TCP/IP communication %s\n", "(112)");
            m_xmodemStatus = XMODEM_STATUS_ERROR;
            return m_xmodemStatus;
        }

        /* write the data */
        cnt = xmodemOutBlock(data, (int)bufsize);
        if (cnt != (int)bufsize)
        {
            wmLog(eDebug, "SmartMoveLowLevel: Error while transmitting data\n");
            wmFatal(eExtEquipment, "QnxMsg.VI.TCPCommFault", "Problem while establishing TCP/IP communication %s\n", "(113)");
            m_xmodemStatus = XMODEM_STATUS_ERROR;
            return m_xmodemStatus;
        }

        /* write the crc */
        int crcbuf[2]{};
        crcbuf[1] = crc & 0xFF;
        crcbuf[0] = (crc >> 8) & 0xFF;
        cnt = xmodemOutBlock(&crcbuf[0], 2) ;
        if (cnt != 2)
        {
            wmLog(eDebug, "SmartMoveLowLevel: Error while transmitting data\n");
            wmFatal(eExtEquipment, "QnxMsg.VI.TCPCommFault", "Problem while establishing TCP/IP communication %s\n", "(114)");
            m_xmodemStatus = XMODEM_STATUS_ERROR;
            return m_xmodemStatus;
        }

        /* read the acknowledge */
        cnt = xmodemInpByte(&res, XMODEM_RXTIMEOUT);

        if (cnt < 1)
        {
            wmLog(eDebug, "SmartMoveLowLevel: Timeout while receiving data\n");
            wmFatal(eExtEquipment, "QnxMsg.VI.TCPCommFault", "Problem while establishing TCP/IP communication %s\n", "(115)");
        }

        if (cnt == 1 && (res & 0xFF) == XMODEM_ACK)
        {
            m_xmodemPacketNo = 0xFF & (m_xmodemPacketNo + 1);
            return m_xmodemStatus;
        }

        /* look if we get cancel characters */
        if (cnt == 1 && (res & 0xFF) == XMODEM_CAN)
        {
            cnt = xmodemInpByte(&res, 10000);
            if (cnt == 1 && (res & 0xFF) == XMODEM_CAN)
            {
                m_xmodemStatus = XMODEM_STATUS_CANCEL;
                return m_xmodemStatus;
            }
        }

        nretrans++;
        xmodemFlushInput();
    }

    wmLog(eDebug, "SmartMoveLowLevel: limit of retries reached\n");
    wmFatal(eExtEquipment, "QnxMsg.VI.TCPCommFault", "Problem while establishing TCP/IP communication %s\n", "(116)");
    m_xmodemStatus = XMODEM_STATUS_ERROR;
    return m_xmodemStatus;
}

int SmartMoveLowLevel::xmodemCalcCRC(int *ptr, int count)
{
    int crc{0};
    while (--count >= 0)
    {
        crc = crc ^ *ptr++ << 8;
        for (int i = 0; i < 8; ++i)
        {
            if (crc & 0x8000)
            {
                crc = crc << 1 ^ 0x1021;
            }
            else
            {
                crc = crc << 1;
            }
        }
    }
    return(crc & 0xFFFF);
}

int SmartMoveLowLevel::transmitMarkingFileBTX(int fileDescriptor, std::string& answer)
{
    return transmitFileBTX("U 101", fileDescriptor, answer);
}

int SmartMoveLowLevel::transmitPenFileBTX(int fileDescriptor, std::string& answer)
{
    return transmitFileBTX("U 103", fileDescriptor, answer);
}

int SmartMoveLowLevel::transmitFileBTX(const std::string& command, int fileDescriptor, std::string& answer)
{
    wmLog(eDebug, "transmitFileBTX: start\n");

    // reset file pointer
    if (lseek(fileDescriptor, (off_t) 0, SEEK_SET) == (off_t) -1)
    {
        wmLog(eDebug, "SmartMoveLowLevel: Error while doing lseek: %s\n", strerror(errno));
        wmFatal(eExtEquipment, "QnxMsg.VI.TCPCommFault", "Problem while establishing TCP/IP communication %s\n", "(117)");
        return -1;
    }

    // send download command
    std::string downloadCommand{command + "\r\n"};
    if (send(m_sockDesc, downloadCommand.c_str(), downloadCommand.size(), 0) == -1)
    {
        wmLog(eDebug, "SmartMoveLowLevel: Error while sending data: %s\n", strerror(errno));
        wmFatal(eExtEquipment, "QnxMsg.VI.TCPCommFault", "Problem while establishing TCP/IP communication %s\n", "(118)");
        return -1;
    }

    // read file content
    uint8_t fileChar{0x00};
    std::vector<int> dataBlock{};
    while (read(fileDescriptor, &fileChar, 1) == 1)
    {
        dataBlock.push_back(static_cast<int>(fileChar));
    }
    wmLog(eDebug, "transmitFileBTX: read %d characters\n", dataBlock.size());

    // calculate CRC
    uint16_t crcValue = xmodemCalcCRC(static_cast<int*>(dataBlock.data()), static_cast<int>(dataBlock.size()));

    // generate and send header
    uint8_t header[8]{};
    header[0] = 0x42;
    header[1] = 1;
    header[2] = (dataBlock.size() >> 24) & 0xFF;
    header[3] = (dataBlock.size() >> 16) & 0xFF;
    header[4] = (dataBlock.size() >> 8) & 0xFF;
    header[5] = dataBlock.size() & 0xFF;
    header[6] = (crcValue >> 8) & 0xFF;
    header[7] = crcValue & 0xFF;
    if (send(m_sockDesc, header, sizeof(header), 0) == -1)
    {
        wmLog(eDebug, "SmartMoveLowLevel: Error while sending data: %s\n", strerror(errno));
        wmFatal(eExtEquipment, "QnxMsg.VI.TCPCommFault", "Problem while establishing TCP/IP communication %s\n", "(119)");
        return -1;
    }

    // copy data to sendBuffer and send sendBuffer
    std::vector<uint8_t> sendBuffer{};
    for (const int& value : dataBlock)
    {
        sendBuffer.push_back(static_cast<uint8_t>(value & 0xFF));
    }
    if (send(m_sockDesc, sendBuffer.data(), sendBuffer.size(), 0) == -1)
    {
        wmLog(eDebug, "SmartMoveLowLevel: Error while sending data: %s\n", strerror(errno));
        wmFatal(eExtEquipment, "QnxMsg.VI.TCPCommFault", "Problem while establishing TCP/IP communication %s\n", "(120)");
        return -1;
    }

    // waiting for answer and extract download result
    int inputBuffer{};
    int returnValue{};
    std::string answerDownload{};
    returnValue = recv(m_sockDesc, &inputBuffer, 1, 0);
    if (returnValue == -1)
    {
        wmLog(eDebug, "transmitFileBTX: after first receive error (%s)\n", strerror(errno));
    }
    else
    {
        answerDownload.push_back(inputBuffer);
    }
    while (returnValue != -1)
    {
        returnValue = recv(m_sockDesc, &inputBuffer, 1, 0);
        if (returnValue == -1)
        {
            wmLog(eDebug, "transmitFileBTX: after next receive error  (%s)\n", strerror(errno));
        }
        else
        {
            answerDownload.push_back(inputBuffer);
            if (static_cast<char>(inputBuffer & 0xFF) == '>')
            {
                break;
            }
        }
    }
    auto first = answerDownload.find("\r\n");
    auto second = answerDownload.find("\r\n", (first + strlen("\r\n")));
    auto length = second - (first + strlen("\r\n"));
    answer = answerDownload.substr((first + strlen("\r\n")), length);

    wmLog(eDebug, "transmitFileBTX: end\n");
    return 0;
}

bool SmartMoveLowLevel::printReady()
{
    std::string answer{};

    readMEParameter("G INT_PRINT_READY", answer, MCI_TIMEOUT_SYMBOL);

    auto number = convertAnswerToNumber(answer, "Answer of INT_PRINT_READY could not be converted to int");

    return number == 1;
}

bool SmartMoveLowLevel::scannerReady()
{
    std::string answer{};
    readMEParameter("G INT_SC_READY", answer, MCI_TIMEOUT_SYMBOL);

    auto number = convertAnswerToNumber(answer, "Answer of SC_READY could not be converted to int");

    return number == 1;
}

void SmartMoveLowLevel::performHardwareReset()
{
    wmLog(eDebug, "Request hardware reset\n");

    std::string downloadHardwareReset{"H\r\n"};
    if (send(m_sockDesc, downloadHardwareReset.c_str(), downloadHardwareReset.size(), 0) == -1)
    {
        wmLog(eError, "Request hardware reset failed\n");
        wmFatal(eExtEquipment, "QnxMsg.VI.TCPCommFault", "Problem while establishing TCP/IP communication %s\n", "(121)");
        return;
    }
}

bool SmartMoveLowLevel::waitForSynchronizationCharacterThenUploadFile(int fileDescriptor)
{
    int timeout = 0;
    int inputBuffer{};
    int returnValue{0};
    std::string answer{};

    while (timeout < 15)
    {
        returnValue = recv(m_sockDesc, &inputBuffer, 1, 0);
        if (returnValue != -1)
        {
            answer.push_back(inputBuffer);
            if(static_cast<char>(inputBuffer & 0xFF) == 'C')
            {
                uploadFileWithXModemProtocol(fileDescriptor);
                return permanentlyStoredFileFeedback();
            }
        }
        timeout++;
    }

    return false;
}

bool SmartMoveLowLevel::resetFilePointer(int fileDescriptor)
{
    if (lseek(fileDescriptor, (off_t) 0, SEEK_SET) == (off_t) -1)
    {
        wmLog(eError, "Error while doing lseek: %s\n", strerror(errno));
        return false;
    }

    return true;
}

void SmartMoveLowLevel::uploadFileWithXModemProtocol(int fileDescriptor)
{
    //Reset XModem
    m_xmodemParameter.MaxRetrans = 3;
    m_xmodemParameter.PacketSize = XMODEM_BUFSIZE_EXTENDED;
    m_xmodemStatus = xmodemReset();

    // read file content && send to ME
    int fileChar{0};
    int success{1};
    while ((read(fileDescriptor, &fileChar, 1) == 1) && (success == 1))
    {
        success = xmodemWriteChar(&fileChar);
    }

    // finish the upload
    xmodemWriteChar(nullptr);
}

bool SmartMoveLowLevel::permanentlyStoredFileFeedback()
{
    std::string answer{};
    char buffer[ANSWER_BUFFER_LEN + 1];
    size_t bytesRead;
    if (readToPrompt(buffer, ANSWER_BUFFER_LEN, &bytesRead, XMODEM_RXTIMEOUT) < 0)
    {
        wmLog(eError, "Error while receiving feedback from file upload\n");
        wmFatal(eExtEquipment, "QnxMsg.VI.TCPCommFault", "Problem while establishing TCP/IP communication %s\n", "(122)");
        return false;
    }
    buffer[bytesRead] = '\0';
    std::string answerDownload(buffer);
    auto first = answerDownload.find("\r\n");
    auto second = answerDownload.find("\r\n", (first + strlen("\r\n")));
    auto length = second - (first + strlen("\r\n"));
    answer = answerDownload.substr((first + strlen("\r\n")), length);
    return true;
}

void SmartMoveLowLevel::synchronizeME()
{
    std::string answer{};
    //Send empty lines for synchronization with ME
    setMEParameter("", answer, MCI_TIMEOUT_SYMBOL);
    usleep(10 * 1000);      //10 ms
    setMEParameter("", answer, MCI_TIMEOUT_SYMBOL);
    usleep(10 * 1000);
}

}
}

