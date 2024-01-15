#pragma once
#include <string>
#include <fliplib/TransformFilter.h>
#include <common/frame.h>
#include <fliplib/PipeEventArgs.h>
#include <fliplib/SynchronePipe.h>
#include <filter/armStates.h>
#include <Poco/UUID.h>
#include <filter/hardwareData.h>

namespace precitec
{
namespace filter
{

class FILTER_API HardwareParameter: public fliplib::TransformFilter
{

public:

    HardwareParameter();
    void setParameter() override;
    void arm(const fliplib::ArmStateBase& state) override;
    bool subscribe(fliplib::BasePipe& pipe, int group) override;
    void proceedGroup(const void* sender, fliplib::PipeGroupEventArgs& event) override;

    void setExternalData(const fliplib::ExternalData* p_pExternalData) override;

private:
    static const std::string m_filterName;
    static const std::string m_imageInName;
    static const std::string m_pipeOutName;

    const fliplib::SynchronePipe<interface::ImageFrame>* m_pipeInImage;
    fliplib::SynchronePipe<interface::GeoDoublearray> m_pipeOutData;
    analyzer::HardwareData m_hardwareData;
    unsigned int m_app;
    std::string m_hardwareParameter;
    std::optional<double> m_parameterValue;
    bool m_setExternal;
};
}
}
