#include "commandLineParser.h"
#include "fileUpload.h"

namespace precitec
{
namespace scheduler
{

static const std::string s_debugArg{"--debug"};
static const std::string s_ipArg{"--ip="};
static const std::string s_portArg{"--port="};
static const std::string s_userArg{"--user="};
static const std::string s_passwordArg{"--password="};
static const std::string s_sourcePathArg{"--sourcePath="};
static const std::string s_sourceFileNameArg{"--sourceFile="};
static const std::string s_sourceDirectoryNameArg{"--sourceDir="};
static const std::string s_remotePathArg{"--remotePath="};
static const std::string s_remoteFileNameArg{"--remoteFile="};
static const std::string s_remoteDirectoryNameArg{"--remoteDir="};
static const std::string s_protocol{"--protocol="};
static const std::string s_protocolSftp{"sftp"};
static const std::string s_protocolHttp{"http"};
static const std::string s_protocolHttps{"https"};
static const std::string s_httpMethodArg{"--httpMethod="};
static const std::string s_httpPost{"POST"};
static const std::string s_httpPut{"PUT"};

void parseCommandLine(int argc, char* argv[], FileUpload& fileUpload)
{
    fileUpload.SetDebugFileIsOn(false);
    for (int i = 0; i < argc; i++)
    {
        std::string arg{argv[i]};
        if (arg == s_debugArg)
        {
            fileUpload.SetDebugFileIsOn(true);
        }
        else if (arg.rfind(s_ipArg, 0) == 0)
        {
            fileUpload.SetIPAddress(arg.substr(s_ipArg.length()));
        }
        else if (arg.rfind(s_portArg, 0) == 0)
        {
            try
            {
                fileUpload.setPort(stoi(arg.substr(s_portArg.length())));
            }
            catch (...)
            {
            }
        }
        else if (arg.rfind(s_userArg, 0) == 0)
        {
            fileUpload.SetRemoteUserName(arg.substr(s_userArg.length()));
        }
        else if (arg.rfind(s_passwordArg, 0) == 0)
        {
            fileUpload.SetPassword(arg.substr(s_passwordArg.length()));
        }
        else if (arg.rfind(s_sourcePathArg, 0) == 0)
        {
            fileUpload.SetSourceDirectory(arg.substr(s_sourcePathArg.length()));
        }
        else if (arg.rfind(s_sourceFileNameArg, 0) == 0)
        {
            fileUpload.SetFileToSend(arg.substr(s_sourceFileNameArg.length()));
        }
        else if (arg.rfind(s_sourceDirectoryNameArg, 0) == 0)
        {
            fileUpload.SetDirectoryToSend(arg.substr(s_sourceDirectoryNameArg.length()));
        }
        else if (arg.rfind(s_remotePathArg, 0) == 0)
        {
            fileUpload.SetTargetDirectory(arg.substr(s_remotePathArg.length()));
        }
        else if (arg.rfind(s_remoteFileNameArg, 0) == 0)
        {
            fileUpload.SetTargetFileName(arg.substr(s_remoteFileNameArg.length()));
        }
        else if (arg.rfind(s_remoteDirectoryNameArg, 0) == 0)
        {
            fileUpload.SetTargetDirectoryName(arg.substr(s_remoteDirectoryNameArg.length()));
        }
        else if (arg.rfind(s_protocol, 0) == 0)
        {
            const auto& protocol{arg.substr(s_protocol.length())};
            if (protocol == s_protocolSftp)
            {
                fileUpload.setProtocol(FileUpload::Protocol::Sftp);
            }
            else if (protocol == s_protocolHttp)
            {
                fileUpload.setProtocol(FileUpload::Protocol::Http);
            }
            else if (protocol == s_protocolHttps)
            {
                fileUpload.setProtocol(FileUpload::Protocol::Https);
            }
        }
        else if (arg.rfind(s_httpMethodArg, 0) ==0)
        {
            const auto& method{arg.substr(s_httpMethodArg.length())};
            if (method == s_httpPost)
            {
                fileUpload.setHttpMethod(FileUpload::HttpMethod::Post);
            }
            else if (method == s_httpPut)
            {
                fileUpload.setHttpMethod(FileUpload::HttpMethod::Put);
            }
        }
    }
}

}
}
