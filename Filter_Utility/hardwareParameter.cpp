#include "hardwareParameter.h"
#include <fliplib/Parameter.h>
#include <fliplib/TypeToDataTypeImpl.h>
#include <module/moduleLogger.h>
#include <analyzer/inspectManager.h>


#define FILTER_ID "3041d376-aebf-11ed-afa1-0242ac120002"
#define VARIANT_ID "355a8470-aebf-11ed-afa1-0242ac120002"
#define PIPE_ID_IMAGEIN "38322df6-aebf-11ed-afa1-0242ac120002"
#define PIPE_ID_CONTOUROUT "5a196172-aec0-11ed-afa1-0242ac120002"

namespace precitec
{
namespace filter
{
const std::string HardwareParameter::m_filterName("HardwareParameter");
const std::string HardwareParameter::m_imageInName("ImageIn");
const std::string HardwareParameter::m_pipeOutName("DataOut");

HardwareParameter::HardwareParameter():
    TransformFilter{HardwareParameter::m_filterName, Poco::UUID(FILTER_ID)}
    , m_pipeInImage{nullptr}
    , m_pipeOutData{this, HardwareParameter::m_pipeOutName}
    , m_app{0}
    , m_hardwareParameter("xcc")
    , m_setExternal{false}
{
    parameters_.add("AppName", fliplib::Parameter::TYPE_int, static_cast<unsigned int>(m_app));
    parameters_.add("ParameterName", fliplib::Parameter::TYPE_string, static_cast<std::string>(m_hardwareParameter));
    unsigned int group = 1;
    setInPipeConnectors({{Poco::UUID(PIPE_ID_IMAGEIN), m_pipeInImage, m_imageInName, group, m_imageInName}});
    group = 0;
    setOutPipeConnectors({{Poco::UUID(PIPE_ID_CONTOUROUT), &m_pipeOutData, m_pipeOutName, group, m_pipeOutName}});
    setVariantID(Poco::UUID(VARIANT_ID));
};


void HardwareParameter::arm(const fliplib::ArmStateBase& state)
{
    if (state.getStateID() == eSeamStart)
    {
        m_parameterValue.reset();
    }
}

void HardwareParameter::setParameter()
{
    TransformFilter::setParameter();
    m_app = parameters_.getParameter("AppName").convert<unsigned int>();
    m_hardwareParameter = parameters_.getParameter("ParameterName").convert<std::string>();
    m_parameterValue.reset();
    m_setExternal = false;
}

bool HardwareParameter::subscribe(fliplib::BasePipe& pipe, int group)
{
    if (pipe.tag() == m_imageInName)
    {
        m_pipeInImage = dynamic_cast<fliplib::SynchronePipe<interface::ImageFrame>*>(&pipe);
    }
    return BaseFilter::subscribe(pipe, group);
}

void HardwareParameter::proceedGroup(const void* sender, fliplib::PipeGroupEventArgs& event)
{
    if (!m_setExternal)
    {
        m_parameterValue = m_hardwareData.get(m_app, m_hardwareParameter);
        m_hardwareData.remove(m_app, m_hardwareParameter);
        m_setExternal = true;
    }

    const auto& imageIn = m_pipeInImage->read(m_oCounter);
    geo2d::Doublearray out(1);
    auto rank = eRankMin;
    auto rankOut = 1.;
    auto result = 0.0;

    if (m_parameterValue.has_value())
    {
        rank = eRankMax;
        result = m_parameterValue.value();
    }
    else
    {
        rank = eRankMin;
        rankOut = 0.;
        wmLog(eWarning, "%s is not supported.\n", m_hardwareParameter);
    }

    out[0] = std::tie(result, rank);
    const interface::GeoDoublearray geoDoubleOut(imageIn.context(), out, imageIn.analysisResult(), rankOut);
    preSignalAction();
    m_pipeOutData.signal(geoDoubleOut);
}

void HardwareParameter::setExternalData(const fliplib::ExternalData* p_pExternalData)
{
    if (auto* hardwareData = dynamic_cast<const analyzer::HardwareData*>(p_pExternalData))
    {
        m_hardwareData = *hardwareData;
        m_setExternal = false;
    }
    else
    {
        FilterControlInterface::setExternalData(p_pExternalData);
    }
}

}
}
