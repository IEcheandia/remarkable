#include "spotWeldingResult.h"
#include <module/moduleLogger.h>
#include <fliplib/TypeToDataTypeImpl.h>
#include <common/definesScanlab.h>

#include <optional>

namespace precitec
{
namespace filter
{

using fliplib::Parameter;
using interface::ResultType;
using interface::GeoDoublearray;
using interface::ResultDoubleArray;
using precitec::filter::eRankMin;


SpotWeldingResult::SpotWeldingResult():
    ResultFilter("SpotWeldingResult", Poco::UUID{"256bb2fc-3f1e-11ed-b878-0242ac120002"})
    , m_pipeInWeldingDuration(nullptr)
    , m_pipeInLaserPowerCenter(nullptr)
    , m_pipeInLaserPowerRing(nullptr)
    , m_pipeResult(this, "SpotWeldingResultOutput")
    , m_outputType(ScanmasterResultType::SpotWelding)
    , m_minMaxRange(-1000., 1000.)
{
     parameters_.add("Result", Parameter::TYPE_int, static_cast<int>(m_outputType));
     setInPipeConnectors({{Poco::UUID("596a1c5a-3fce-11ed-b878-0242ac120002"), m_pipeInWeldingDuration, "weldingDurationIn", 1, "weldingDurationIn"},
     {Poco::UUID("f0a20048-4086-11ed-b878-0242ac120002"), m_pipeInLaserPowerCenter, "laserPowerCenterIn", 1, "laserPowerCenterIn"},
     {Poco::UUID("46fbcc08-4b8b-11ed-bdc3-0242ac120002"), m_pipeInLaserPowerRing, "optionalLaserPowerRingIn", 1, "optionalLaserPowerRingIn", fliplib::PipeConnector::ConnectionType::Optional}});
     setVariantID(Poco::UUID("6bf17f08-3fce-11ed-b878-0242ac120002"));
}


bool SpotWeldingResult::subscribe(fliplib::BasePipe& pipe, int group)
{
    if (pipe.tag() == "weldingDurationIn")
    {
        m_pipeInWeldingDuration = dynamic_cast<pipe_scalar_t*>(&pipe);
    }
    else if (pipe.tag() == "laserPowerCenterIn")
    {
        m_pipeInLaserPowerCenter = dynamic_cast<pipe_scalar_t*>(&pipe);
    }
    else if (pipe.tag() == "optionalLaserPowerRingIn")
    {
         m_pipeInLaserPowerRing = dynamic_cast<pipe_scalar_t*>(&pipe);
    }
    else
    {
        poco_assert_dbg(false); // to be asserted by graph editor
    }
    return BaseFilter::subscribe(pipe, group);
}

void SpotWeldingResult::proceedGroup(const void* sender, fliplib::PipeGroupEventArgs& e)
{
    poco_assert_dbg(m_pipeInWeldingDuration != nullptr); // to be asserted by graph editor
    poco_assert_dbg(m_pipeInLaserPowerCenter != nullptr); // to be asserted by graph editor

    const auto& geoPowerCenterIn = m_pipeInLaserPowerCenter->read(m_oCounter);
    const auto& geoDurationIn = m_pipeInWeldingDuration->read(m_oCounter);
    const GeoDoublearray* geoPowerRingIn;

    if (m_pipeInLaserPowerRing != nullptr)
    {
        geoPowerRingIn = &m_pipeInLaserPowerRing->read(m_oCounter);
    }

    auto analysisResult = geoDurationIn.analysisResult();

    ResultType resultType = ResultType::ScanmasterSpotWelding;
    if (analysisResult != interface::AnalysisOK
    || geoDurationIn.ref().size() == 0
    || geoPowerCenterIn.ref().size() == 0
    || (m_pipeInLaserPowerRing != nullptr && geoPowerRingIn->ref().size() == 0)
    )
    {
        m_resultArray.resize(0);
        const auto geoValueOut = GeoDoublearray
        {
            geoDurationIn.context(),
            m_resultArray,
            analysisResult,
            interface::NotPresent
        };

        const auto resultNIO = ResultDoubleArray
        {
            id(),
            resultType,
            analysisResult,
            geoDurationIn.context(),
            geoValueOut,
            m_minMaxRange,
            true
        };
        preSignalAction();
        m_pipeResult.signal(resultNIO);
        return;
    }

    // get duration and power input values from pipes
    m_resultArray.resize(3);
    const auto durationIn = geoDurationIn.ref().getData().at(0);
    const auto powerCenterIn = geoPowerCenterIn.ref().getData().at(0);

    const auto durationRankIn = geoDurationIn.ref().getRank().at(0);
    const auto powerCenterRankIn = geoPowerCenterIn.ref().getRank().at(0);

    const auto posWeldingDuration = static_cast<int>(InputType::WeldingDuration);
    const auto posLaserPowerCenter = static_cast<int>(InputType::LaserPowerCenter);
    const auto posLaserPowerRing = static_cast<int>(InputType::LaserPowerRing);

    m_resultArray.getData()[posWeldingDuration] = durationIn;
    m_resultArray.getData()[posLaserPowerCenter] = powerCenterIn;

    m_resultArray.getRank()[posWeldingDuration] = durationRankIn;
    m_resultArray.getRank()[posLaserPowerCenter] = powerCenterRankIn;

    //handle optional laserPowerRing pipe
    std::optional<double> powerRingIn;
    std::optional<int> powerRingRankIn;
    if (m_pipeInLaserPowerRing != nullptr)
    {
        powerRingIn = geoPowerRingIn->ref().getData().at(0);
        powerRingRankIn = geoPowerRingIn->ref().getRank().at(0);
        m_resultArray.getData()[posLaserPowerRing] = powerRingIn.value();
        m_resultArray.getRank()[posLaserPowerRing] = powerRingRankIn.value();
    }
    else
    {
        m_resultArray.getData()[posLaserPowerRing] = SCANMASTERWELDINGDATA_UNDEFINEDVALUE;
    }

    bool validOut = true;
    double rankOut = 1;

    if (durationRankIn == eRankMin || powerCenterRankIn == eRankMin || (powerRingRankIn.has_value() && powerRingRankIn.value() == eRankMin))
    {
        validOut = false;
    }

    if (durationIn <= 0)
    {
        wmLog(eWarning, "Result %d contains invalid weldingDurationIn, setting rank to 0 \n", resultType);
        validOut = false;
    }

    if (powerCenterIn < 0)
    {
        wmLog(eWarning, "Result %d contains invalid laserPowerIn, setting rank to 0 \n", resultType);
        validOut = false;
    }

    if (m_pipeInLaserPowerRing != nullptr && powerRingIn.has_value() && powerRingIn.value() < 0)
    {
        wmLog(eWarning, "Result %d contains invalid laserPowerRingIn, setting rank to 0 \n", resultType);
        validOut = false;
    }

    if(m_oCounter != 0)
    {
        validOut = false;
    }

    if(!validOut)
    {
        std::fill(m_resultArray.getRank().begin(), m_resultArray.getRank().end(), eRankMin);
        rankOut = 0;
    }

    const auto geoValueOut = GeoDoublearray
    {
        geoDurationIn.context(),
        m_resultArray,
        analysisResult,
        rankOut
    };

    auto resultDoubleOut = ResultDoubleArray
    {
        id(),
        resultType,
        ResultType::ValueOutOfLimits,
        geoDurationIn.context(),
        geoValueOut,
        m_minMaxRange,
        false
    };

    resultDoubleOut.setValid(validOut);
    preSignalAction();
    m_pipeResult.signal(resultDoubleOut);
}


void SpotWeldingResult::setParameter()
{
    ResultFilter::setParameter();
}

}
}
