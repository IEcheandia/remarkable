#include "temporalLowPass2.h"

#include "module/moduleLogger.h"
#include "filter/armStates.h" ///< arm state seam start
#include <fliplib/TypeToDataTypeImpl.h>

namespace precitec
{
namespace filter
{

TemporalLowPass2::TemporalLowPass2()
    : TransformFilter("TemporalLowPass2", Poco::UUID{"6a6d8147-fb00-4800-ac0d-4540a10e60e4"})
    , m_pPipeIn1(nullptr)
    , m_pPipeIn2(nullptr)
    , m_oPipeOut1(this, "Value1")
    , m_oPipeOut2(this, "Value2")
    , m_calculator{}
{
    // Defaultwerte der Parameter setzen
    m_calculator.addToParameterContainer(parameters_);

    setInPipeConnectors({
        {Poco::UUID("d5777c66-d8f2-45d9-8698-d24fc9f99144"), m_pPipeIn1, "input1", 1, "input1"},
        {Poco::UUID("7da41331-bbe8-4a9f-bbfb-0c750b5086c6"), m_pPipeIn2, "input2", 1, "input2"},
    });
    setOutPipeConnectors({{Poco::UUID("df0ff821-3821-40a8-ab4f-dd3029cdfb1d"), &m_oPipeOut1},
                          {Poco::UUID("3621cc9e-8cae-4dbd-8f0f-cce4019644ed"), &m_oPipeOut2}});
    setVariantID(Poco::UUID("63c14352-77c2-4a8f-87e1-81703981f37e"));
}

void TemporalLowPass2::setParameter()
{
    TransformFilter::setParameter();
    m_calculator.setParameter(parameters_);
}

void TemporalLowPass2::arm(const fliplib::ArmStateBase& p_rArmtate)
{
    if (p_rArmtate.getStateID() == eSeamStart)
    {
        m_calculator.clear();
    }

    if (m_oVerbosity >= eHigh)
    {
        wmLog(eDebug, "Filter '%s' armed at armstate %i\n", name().c_str(), p_rArmtate.getStateID());
    }
}

bool TemporalLowPass2::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup)
{
    if (p_rPipe.tag() == "input1")
    {
        m_pPipeIn1 = dynamic_cast<fliplib::SynchronePipe<interface::GeoDoublearray>*>(&p_rPipe);
    }
    else if (p_rPipe.tag() == "input2")
    {
        m_pPipeIn2 = dynamic_cast<fliplib::SynchronePipe<interface::GeoDoublearray>*>(&p_rPipe);
    }
    else
    {
        return false;
    }
    return BaseFilter::subscribe(p_rPipe, p_oGroup);
} // subscribe


void TemporalLowPass2::proceedGroup(const void* sender, fliplib::PipeGroupEventArgs& e)
{
    poco_assert_dbg(m_pPipeIn1 != nullptr); // to be asserted by graph editor
    poco_assert_dbg(m_pPipeIn2 != nullptr); // to be asserted by graph editor

    const auto& rGeoArrayIn1 = m_pPipeIn1->read(m_oCounter);
    const auto& rGeoArrayIn2 = m_pPipeIn2->read(m_oCounter);

    const auto oGeoDoubleOut1 = m_calculator.temporalLowPass<0>(rGeoArrayIn1);
    const auto oGeoDoubleOut2 = m_calculator.temporalLowPass<1>(rGeoArrayIn2);

    preSignalAction();
    m_oPipeOut1.signal(oGeoDoubleOut1);
    m_oPipeOut2.signal(oGeoDoubleOut2);

} // proceed

}
}
