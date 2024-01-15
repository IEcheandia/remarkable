/**
 *  @file
 *  @copyright  Precitec Vision GmbH & Co. KG
 *  @author     EA
 *  @date       2022
 *  @brief      Working stuff for uploading files to an external server
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <pwd.h>
#include <iostream>

#include "pipeLogger.h"
#include "fileUpload.h"

namespace precitec
{

namespace scheduler
{

namespace fs = std::filesystem;

const int MAX_LOGGER_MESSAGE_LENGTH = 150;

FileUpload::FileUpload(void):
    m_oNumberOfTransmittedFiles(0),
    m_oTotalTransmittedBytes(0),
    m_oDurationOfTransmissionMS(0)
{
    curl_global_init(CURL_GLOBAL_ALL);
}

FileUpload::~FileUpload(void)
{
    if (m_mime)
    {
        curl_mime_free(m_mime);
    }
    curl_global_cleanup();
}

void FileUpload::UploadFile(void)
{
    PreTransfer();

    if (m_pCurlHandle)
    {
        try
        {
            std::error_code oErrorCode;
            std::uintmax_t oSize = fs::file_size(fs::path{m_oFileToSend}, oErrorCode);
            if (oSize == static_cast<std::uintmax_t>(-1))
            {
                std::cerr << "error in stat " << m_oFileToSend << ": " << errno << "," << strerror(errno) << std::endl;
                char oLogMsg[200];
                sprintf(oLogMsg, "File transfer to a remote system has failed (%s)\n", "001");
                precitec::pipeLogger::SendLoggerMessage(m_oWritePipeFd, eError, oLogMsg);
                sprintf(oLogMsg, "error in stat %s: %d,%s\n", m_oFileToSend.c_str(), errno, strerror(errno));
                precitec::pipeLogger::SendLoggerMessage(m_oWritePipeFd, eDebug, oLogMsg);
                return;
            }
            unsigned long oFileSize = (unsigned long)oSize;
            if (m_oDebugFileIsOn)
            {
                std::cerr << "Local oFileSize: " << oFileSize << " bytes" << std::endl;
            }
            curl_easy_setopt(m_pCurlHandle, CURLOPT_INFILESIZE_LARGE, (curl_off_t)oFileSize);

            const auto oDestination{buildUrl(TransferMode::File)};
            if (m_oDebugFileIsOn)
            {
                std::cerr << "[" << oDestination << "]" << std::endl;
            }
            curl_easy_setopt(m_pCurlHandle, CURLOPT_URL, oDestination.c_str());

            FILE* pFileHandle = fopen(m_oFileToSend.c_str(), "rb");
            curl_easy_setopt(m_pCurlHandle, CURLOPT_READDATA, pFileHandle); // now specify which file to upload

            initHttpMime(m_oFileToSend);

            CURLcode res = curl_easy_perform(m_pCurlHandle); // Now run off and do what you have been told
            if(res != CURLE_OK) // Check for errors
            {
                std::cerr << "error in curl_easy_perform: " << static_cast<int>(res) << "," << curl_easy_strerror(res) << std::endl;
                char oLogMsg[200];
                sprintf(oLogMsg, "File transfer to a remote system has failed (%s)\n", "002");
                precitec::pipeLogger::SendLoggerMessage(m_oWritePipeFd, eError, oLogMsg);
                sprintf(oLogMsg, "error in curl_easy_perform: %d,%s\n", res, curl_easy_strerror(res));
                precitec::pipeLogger::SendLoggerMessage(m_oWritePipeFd, eDebug, oLogMsg);
            }
            else
            {
                m_oNumberOfTransmittedFiles++;
                m_oTotalTransmittedBytes += oFileSize;
            }
            fclose(pFileHandle);
        }
        catch(fs::filesystem_error const& oFilesystemException)
        {
            std::cerr << "UploadFile failed: " << oFilesystemException.code().message() << std::endl;
            char oLogMsg[200];
            sprintf(oLogMsg, "File transfer to a remote system has failed (%s)\n", "003");
            precitec::pipeLogger::SendLoggerMessage(m_oWritePipeFd, eError, oLogMsg);
            sprintf(oLogMsg, "UploadFile failed: %s\n", oFilesystemException.code().message().c_str());
            precitec::pipeLogger::SendLoggerMessage(m_oWritePipeFd, eDebug, oLogMsg);
        }
    }

    PostTransfer();
}

void FileUpload::UploadDirectory(void)
{
    PreTransfer();

    if (m_pCurlHandle)
    {
        try
        {
            if (m_oDebugFileIsOn)
            {
                std::cerr << m_oDirectoryToSend << std::endl;
            }
            std::error_code oErrorCode;
            fs::path start{m_oDirectoryToSend};
            fs::recursive_directory_iterator verzeichnisIt{start}, ende;
            while (verzeichnisIt != ende)
            {
                if (!fs::is_directory(verzeichnisIt->path()))
                {
                    if (m_oDebugFileIsOn)
                    {
                        std::cerr << std::endl;
                        std::cerr << verzeichnisIt->path().string() << std::endl;
                    }

                    std::uintmax_t oSize = fs::file_size(verzeichnisIt->path(), oErrorCode);
                    if (oSize == static_cast<std::uintmax_t>(-1))
                    {
                        std::cerr << "error in stat " << verzeichnisIt->path() << ": " << errno << "," << strerror(errno) << std::endl;
                        char oLogMsg[200];
                        sprintf(oLogMsg, "Directory transfer to a remote system has failed (%s)\n", "001");
                        precitec::pipeLogger::SendLoggerMessage(m_oWritePipeFd, eError, oLogMsg);
                        sprintf(oLogMsg, "error in stat %s: %d,%s\n", m_oFileToSend.c_str(), errno, strerror(errno));
                        precitec::pipeLogger::SendLoggerMessage(m_oWritePipeFd, eDebug, oLogMsg);
                        return;
                    }
                    unsigned long oFileSize = (unsigned long)oSize;
                    if (m_oDebugFileIsOn)
                    {
                        std::cerr << "Local oFileSize: " << oFileSize << " bytes" << std::endl;
                    }
                    curl_easy_setopt(m_pCurlHandle, CURLOPT_INFILESIZE_LARGE, (curl_off_t)oFileSize);

                    // the path is shortened by the first level
                    int oLoop{1};
                    std::stringstream oNewPath{};
                    auto it = verzeichnisIt->path().begin();
                    while(it != verzeichnisIt->path().end())
                    {
                        if (oLoop > 1)
                        {
                            oNewPath << "/" << it->string();
                        }
                        ++it;
                        ++oLoop;
                    }
                    const auto oDestination{buildUrl(TransferMode::Directory)  + oNewPath.str()};
                    if (m_oDebugFileIsOn)
                    {
                        std::cerr << "[" << oDestination << "]" << std::endl;
                    }
                    curl_easy_setopt(m_pCurlHandle, CURLOPT_URL, oDestination.c_str());

                    FILE* pFileHandle = fopen(verzeichnisIt->path().string().c_str(), "rb");
                    curl_easy_setopt(m_pCurlHandle, CURLOPT_READDATA, pFileHandle); // now specify which file to upload

                    initHttpMime(verzeichnisIt->path().string());

                    CURLcode res = curl_easy_perform(m_pCurlHandle); // Now run off and do what you have been told
                    if(res != CURLE_OK) // Check for errors
                    {
                        std::cerr << "error in curl_easy_perform: " << static_cast<int>(res) << "," << curl_easy_strerror(res) << std::endl;
                        char oLogMsg[200];
                        sprintf(oLogMsg, "Directory transfer to a remote system has failed (%s)\n", "002");
                        precitec::pipeLogger::SendLoggerMessage(m_oWritePipeFd, eError, oLogMsg);
                        sprintf(oLogMsg, "error in curl_easy_perform: %d,%s\n", res, curl_easy_strerror(res));
                        precitec::pipeLogger::SendLoggerMessage(m_oWritePipeFd, eDebug, oLogMsg);
                    }
                    else
                    {
                        m_oNumberOfTransmittedFiles++;
                        m_oTotalTransmittedBytes += oFileSize;
                    }
                    fclose(pFileHandle);
                }
                ++verzeichnisIt;
            }
        }
        catch(fs::filesystem_error const& oFilesystemException)
        {
            std::cerr << "UploadDirectory failed: " << oFilesystemException.code().message() << std::endl;
            char oLogMsg[200];
            sprintf(oLogMsg, "Directory transfer to a remote system has failed (%s)\n", "003");
            precitec::pipeLogger::SendLoggerMessage(m_oWritePipeFd, eError, oLogMsg);
            sprintf(oLogMsg, "UploadDirectory failed: %s\n", oFilesystemException.code().message().c_str());
            precitec::pipeLogger::SendLoggerMessage(m_oWritePipeFd, eDebug, oLogMsg);
        }
    }

    PostTransfer();
}

std::string FileUpload::buildUrl(TransferMode mode) const
{
    std::stringstream oDestination{};
    switch (m_protocol)
    {
    case Protocol::Sftp:
        oDestination << "sftp://";
        break;
    case Protocol::Http:
        oDestination << "http://";
        break;
    case Protocol::Https:
        oDestination << "https://";
        break;
    }
    if (!m_oRemoteUserName.empty())
    {
        oDestination << m_oRemoteUserName << "@";
    }
    oDestination << "sftp.nonamedummy.com";
    if (m_port)
    {
        oDestination << ":" << std::to_string(m_port.value());
    }

    if (m_oTargetDirectory.rfind("/", 0) == std::string::npos)
    {
        oDestination << "/";
    }
    oDestination << m_oTargetDirectory;
    switch (mode)
    {
    case TransferMode::File:
        oDestination << m_oTargetFileName;
        break;
    case TransferMode::Directory:
        oDestination << m_oTargetDirectoryName;
        break;
    }
    return oDestination.str();
}

void FileUpload::initHttpMime(const std::string& fileToSend)
{
    if ((m_protocol == Protocol::Http || m_protocol == Protocol::Https) && m_httpMethod == HttpMethod::Post)
    {
        if (m_mime)
        {
            curl_mime_free(m_mime);
            m_mime = nullptr;
        }
        m_mime = curl_mime_init(m_pCurlHandle);

        auto field = curl_mime_addpart(m_mime);
        // TODO: this might need to be adjusted based on what the server expects
        curl_mime_name(field, "fileupload");
        curl_mime_filedata(field, fileToSend.c_str());
        if (!m_oTargetFileName.empty())
        {
            curl_mime_filename(field, m_oTargetFileName.c_str());
        }

        curl_easy_setopt(m_pCurlHandle, CURLOPT_MIMEPOST, m_mime);
    }
}

void FileUpload::PreTransfer(void)
{
    m_oStartTime = std::chrono::high_resolution_clock::now();

    if (m_oDebugFileName.empty())
    {
        m_oDebugFileIsOn = false;
    }
    if (m_oDebugFileIsOn)
    {
        fflush(stderr);
        m_oStderrFd = dup(STDERR_FILENO);

        std::string filePath{getenv("WM_BASE_DIR")};
        filePath.append("/logfiles/");
        filePath.append(m_oDebugFileName);
        m_oDebugFileFd = open(filePath.c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
        if (m_oDebugFileFd == -1)
        {
            std::cerr << "m_oDebugFileFd: " << m_oDebugFileFd << " (" << strerror(errno) << ")" << std::endl;
            char oLogMsg[200];
            sprintf(oLogMsg, "Cannot create debug file for transfer operation\n");
            precitec::pipeLogger::SendLoggerMessage(m_oWritePipeFd, eError, oLogMsg);
            sprintf(oLogMsg, "m_oDebugFileFd: %d (%s)\n", m_oDebugFileFd, strerror(errno));
            precitec::pipeLogger::SendLoggerMessage(m_oWritePipeFd, eDebug, oLogMsg);
        }
        dup2(m_oDebugFileFd, STDERR_FILENO);
        close(m_oDebugFileFd);
    }

    try
    {
        m_oCurrentPath = fs::current_path();
    }
    catch(fs::filesystem_error const& oFilesystemException)
    {
        std::cerr << "current_path() failed: " << oFilesystemException.code().message() << std::endl;
        char oLogMsg[200];
        sprintf(oLogMsg, "Change directory has failed (%s)\n", "001");
        precitec::pipeLogger::SendLoggerMessage(m_oWritePipeFd, eError, oLogMsg);
        sprintf(oLogMsg, "current_path() failed: %s\n", oFilesystemException.code().message().c_str());
        precitec::pipeLogger::SendLoggerMessage(m_oWritePipeFd, eDebug, oLogMsg);
        return;
    }
    if (m_oDebugFileIsOn)
    {
        std::cerr << "Current path: " << m_oCurrentPath << std::endl;
    }

    m_pCurlHandle = curl_easy_init();
    if (m_pCurlHandle)
    {
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////7

        m_oNumberOfTransmittedFiles = 0;
        m_oTotalTransmittedBytes = 0;

        m_oCurlVersion.assign(curl_version());
        if (m_oDebugFileIsOn)
        {
            std::cerr << "libcurl version:   " << m_oCurlVersion << std::endl;
            if (m_port)
            {
                std::cerr << "m_port:            " << m_port.value() << std::endl;
            }
            std::cerr << "m_oIPAddress:      " << m_oIPAddress << std::endl;
            std::cerr << "m_oRemoteUserName: " << m_oRemoteUserName << std::endl;
            std::cerr << "m_oPassword:       " << m_oPassword << std::endl;
            std::cerr << std::endl;
        }

        if (m_oDebugFileIsOn)
        {
            curl_easy_setopt(m_pCurlHandle, CURLOPT_VERBOSE, 1L);
        }
        else
        {
            curl_easy_setopt(m_pCurlHandle, CURLOPT_VERBOSE, 0L);
        }

        std::string oHostName{};
        oHostName.assign("sftp.nonamedummy.com:");
        if (m_port)
        {
            oHostName.append(std::to_string(m_port.value()));
            oHostName.append(":");
        }
        oHostName.append(m_oIPAddress);
        m_pHostList = curl_slist_append(nullptr, oHostName.c_str());
        curl_easy_setopt(m_pCurlHandle, CURLOPT_RESOLVE, m_pHostList);

        curl_easy_setopt(m_pCurlHandle, CURLOPT_READFUNCTION, readCallbackFunction); // we want to use our own read function

        auto setupHttpMethod = [this]
        {
            switch (m_httpMethod)
            {
            case HttpMethod::Post:
                curl_easy_setopt(m_pCurlHandle, CURLOPT_POST, 1L);
                break;
            case HttpMethod::Put:
                curl_easy_setopt(m_pCurlHandle, CURLOPT_UPLOAD, 1L);
                break;
            }
        };

        if (m_protocol == Protocol::Sftp)
        {
            curl_easy_setopt(m_pCurlHandle, CURLOPT_UPLOAD, 1L); // enable uploading
            curl_easy_setopt(m_pCurlHandle, CURLOPT_FTP_CREATE_MISSING_DIRS, CURLFTP_CREATE_DIR_RETRY);

            curl_easy_setopt(m_pCurlHandle, CURLOPT_SSH_AUTH_TYPES, CURLSSH_AUTH_ANY);
            curl_easy_setopt(m_pCurlHandle, CURLOPT_SSH_KEYFUNCTION, sshCallbackFunction);
            curl_easy_setopt(m_pCurlHandle, CURLOPT_SSH_KEYDATA, nullptr);
            std::string oKnownHostsPath{getenv("HOME")};
            oKnownHostsPath += "/.ssh/known_hosts";
            curl_easy_setopt(m_pCurlHandle, CURLOPT_SSH_KNOWNHOSTS, oKnownHostsPath.c_str());
            curl_easy_setopt(m_pCurlHandle, CURLOPT_PASSWORD, m_oPassword.c_str());
        }
        else if (m_protocol == Protocol::Http)
        {
            curl_easy_setopt(m_pCurlHandle, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_2_0);
            setupHttpMethod();
        }
        else if (m_protocol == Protocol::Https)
        {
            curl_easy_setopt(m_pCurlHandle, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_2TLS);
            // the next two commands disable SSL Verification, all certificates are accepted
            curl_easy_setopt(m_pCurlHandle, CURLOPT_SSL_VERIFYPEER, 0);
            curl_easy_setopt(m_pCurlHandle, CURLOPT_SSL_VERIFYHOST, 0);
            setupHttpMethod();
        }

        try
        {
            fs::current_path(m_oSourceDirectory);
            if (m_oDebugFileIsOn)
            {
                std::cerr << "Current path is " << fs::current_path() << std::endl;
            }
        }
        catch(fs::filesystem_error const& oFilesystemException)
        {
            std::cerr << "current_path() failed: " << oFilesystemException.code().message() << std::endl;
            char oLogMsg[200];
            sprintf(oLogMsg, "Change directory has failed (%s)\n", "002");
            precitec::pipeLogger::SendLoggerMessage(m_oWritePipeFd, eError, oLogMsg);
            sprintf(oLogMsg, "current_path() failed: %s\n", oFilesystemException.code().message().c_str());
            precitec::pipeLogger::SendLoggerMessage(m_oWritePipeFd, eDebug, oLogMsg);
        }
    }
}

void FileUpload::PostTransfer(void)
{
    if (m_pCurlHandle)
    {
        curl_slist_free_all(m_pHostList);
        curl_easy_cleanup(m_pCurlHandle);
    }

    try
    {
        fs::current_path(m_oCurrentPath);
    }
    catch(fs::filesystem_error const& oFilesystemException)
    {
        std::cerr << "current_path() failed: " << oFilesystemException.code().message() << std::endl;
        char oLogMsg[200];
        sprintf(oLogMsg, "Change directory has failed (%s)\n", "003");
        precitec::pipeLogger::SendLoggerMessage(m_oWritePipeFd, eError, oLogMsg);
        sprintf(oLogMsg, "current_path() failed: %s\n", oFilesystemException.code().message().c_str());
        precitec::pipeLogger::SendLoggerMessage(m_oWritePipeFd, eDebug, oLogMsg);
    }

    if (m_oDebugFileIsOn)
    {
        fflush(stderr);
        dup2(m_oStderrFd, STDERR_FILENO);
        close(m_oStderrFd);
    }

    m_oStopTime = std::chrono::high_resolution_clock::now();
    m_oDurationOfTransmissionMS = std::chrono::duration_cast<std::chrono::milliseconds>(m_oStopTime - m_oStartTime).count();
}

size_t FileUpload::readCallbackFunction(char* ptr, size_t size, size_t nmemb, void* stream)
{
    size_t retcode = fread(ptr, size, nmemb, (FILE *)stream);
    return retcode;
}

int FileUpload::sshCallbackFunction(CURL* easy, const struct curl_khkey* knownkey, const struct curl_khkey* foundkey, enum curl_khmatch khmatch, void* clientp)
{
    // caution: check clientp for nullptr before use !
    return CURLKHSTAT_FINE;
}

} // namespace scheduler

} // namespace precitec

