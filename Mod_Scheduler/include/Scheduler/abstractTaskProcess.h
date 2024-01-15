#pragma once

#include <map>
#include <string>
#include <vector>

namespace precitec
{

namespace scheduler
{

class AbstractTaskProcess
{
public:
    AbstractTaskProcess() = default;
    virtual ~AbstractTaskProcess();

    void setInfo(const std::map<std::string, std::string> &info);
    const std::map<std::string, std::string> &info() const;
    bool run();

    void setPath(const std::string &path);
protected:
    std::string path() const;

    virtual std::string name() = 0;
    virtual std::vector<std::string> processArguments(const int loggerPipeFds[2]) = 0;
private:
    static std::vector<char*> transformedProcessArguments(const std::string &fullProcessName, const std::vector<std::string> &processArguments);
    static bool log(int loggerPipeFds[2]);

    std::map<std::string, std::string> m_info;
    std::string m_path{};
};

}
}