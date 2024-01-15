
#pragma once
#include <string>
#include <fliplib/FilterControlInterface.h>
#include <Poco/UUID.h>
#include <map>
#include <unordered_map>
#include <optional>

namespace precitec
{
namespace analyzer
{

class HardwareData: public fliplib::ExternalData
{
public:

    std::optional<double> get(unsigned int appId, const std::string& parameterName) const;
    void set(unsigned int appId, const std::string& parameterName, double value);
    void remove(unsigned int appId, const std::string& parameterName);

private:
    std::map<unsigned int, std::unordered_map<std::string, double>> m_parameterMap;
};

}
}
