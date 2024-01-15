/**
 *  @file
 *  @copyright  Precitec Vision GmbH & Co. KG
 *  @author     EA
 *  @date       2022
 *  @brief      Working stuff for uploading files to an external server
 */

#pragma once

#include <chrono>
#include <filesystem>

#include <curl/curl.h>
#include <optional>

#include "module/logType.h"

namespace precitec
{

namespace scheduler
{

namespace fs = std::filesystem;

class FileUpload
{

public:
    FileUpload(void);
    virtual ~FileUpload(void);

    void UploadFile(void);
    void UploadDirectory(void);

    uint32_t GetNumberOfTransmittedFiles(void) {return m_oNumberOfTransmittedFiles;}
    uint64_t GetTotalTransmittedBytes(void) {return m_oTotalTransmittedBytes;}
    uint64_t GetDurationOfTransmissionMS(void) {return m_oDurationOfTransmissionMS;}

    void SetIPAddress(const std::string& p_oIPAddress) {m_oIPAddress.assign(p_oIPAddress);}
    std::string& GetIPAddress(void) {return m_oIPAddress;}
    void SetPassword(const std::string& p_oPassword) {m_oPassword.assign(p_oPassword);}
    std::string& GetPassword(void) {return m_oPassword;}
    void SetRemoteUserName(const std::string& p_oRemoteUserName) {m_oRemoteUserName.assign(p_oRemoteUserName);}
    std::string& GetRemoteUserName(void) {return m_oRemoteUserName;}

    void SetSourceDirectory(const std::string& p_oSourceDirectory) {m_oSourceDirectory.assign(p_oSourceDirectory);}
    std::string& GetSourceDirectory(void) {return m_oSourceDirectory;}
    void SetFileToSend(const std::string& p_oFileToSend) {m_oFileToSend.assign(p_oFileToSend);}
    std::string& GetFileToSend(void) {return m_oFileToSend;}
    void SetDirectoryToSend(const std::string& p_oDirectoryToSend) {m_oDirectoryToSend.assign(p_oDirectoryToSend);}
    std::string& GetDirectoryToSend(void) {return m_oDirectoryToSend;}

    void SetTargetDirectory(const std::string& p_oTargetDirectory) {m_oTargetDirectory.assign(p_oTargetDirectory);}
    std::string& GetTargetDirectory(void) {return m_oTargetDirectory;}
    void SetTargetFileName(const std::string& p_oTargetFileName) {m_oTargetFileName.assign(p_oTargetFileName);}
    std::string& GetTargetFileName(void) {return m_oTargetFileName;}
    void SetTargetDirectoryName(const std::string& p_oTargetDirectoryName) {m_oTargetDirectoryName.assign(p_oTargetDirectoryName);}
    std::string& GetTargetDirectoryName(void) {return m_oTargetDirectoryName;}

    void SetDebugFileIsOn(const bool p_oDebugFileIsOn) {m_oDebugFileIsOn = p_oDebugFileIsOn;}
    void SetDebugFileName(const std::string& p_oDebugFileName) {m_oDebugFileName = p_oDebugFileName;}
    void SetWritePipeFd(const int p_oWritePipeFd) {m_oWritePipeFd = p_oWritePipeFd;}

    void setPort(int port)
    {
        m_port = port;
    }

    enum class Protocol
    {
        Sftp,
        Http,
        Https
    };
    void setProtocol(Protocol protocol)
    {
        m_protocol = protocol;
    }

    enum class HttpMethod
    {
        Post,
        Put
    };
    void setHttpMethod(HttpMethod method)
    {
        m_httpMethod = method;
    }

private:
    void PreTransfer(void);
    void PostTransfer(void);

    enum class TransferMode
    {
        File,
        Directory,
    };
    std::string buildUrl(TransferMode mode) const;
    void initHttpMime(const std::string& fileToSend);

    static size_t readCallbackFunction(char* ptr, size_t size, size_t nmemb, void* stream);
    static int sshCallbackFunction(CURL* easy, const struct curl_khkey* knownkey, const struct curl_khkey* foundkey, enum curl_khmatch khmatch, void* clientp);

    CURL* m_pCurlHandle{nullptr};
    curl_mime* m_mime{nullptr};
    struct curl_slist* m_pHostList{nullptr};
    fs::path m_oCurrentPath{};

    std::chrono::time_point<std::chrono::high_resolution_clock> m_oStartTime{};
    std::chrono::time_point<std::chrono::high_resolution_clock> m_oStopTime{};

    std::string m_oCurlVersion{};
    uint32_t m_oNumberOfTransmittedFiles{};
    uint64_t m_oTotalTransmittedBytes{};
    uint64_t m_oDurationOfTransmissionMS{};

    std::string m_oIPAddress{"128.0.0.1"};
    std::string m_oRemoteUserName{};
    std::string m_oPassword{};

    std::string m_oSourceDirectory{};
    std::string m_oFileToSend{};
    std::string m_oDirectoryToSend{};

    std::string m_oTargetDirectory{};
    std::string m_oTargetFileName{};
    std::string m_oTargetDirectoryName{};

    bool m_oDebugFileIsOn{false};
    std::string m_oDebugFileName{};
    int m_oStderrFd{};
    int m_oDebugFileFd{};

    int m_oWritePipeFd{}; // fd for the write side of a pipe
    std::optional<int> m_port{};

    Protocol m_protocol{Protocol::Sftp};
    HttpMethod m_httpMethod{HttpMethod::Post};
};

} // namespace scheduler

} // namespace precitec


