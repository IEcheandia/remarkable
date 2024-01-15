/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		Simon Hilsenbeck (HS)
 * 	@date		2011
 * 	@brief		Fliplib filter 'TemporalLowPass'.
 * 				Low pass filter, with selectable 'filtering' start.
 */
#include "temporalLowPass.h"

#include "module/moduleLogger.h"
#include "filter/armStates.h"			///< arm state seam start
#include <fliplib/TypeToDataTypeImpl.h>

namespace precitec {
namespace filter {

using interface::GeoDoublearray;

const std::string TemporalLowPass::m_oFilterName 	= std::string("TemporalLowPass");
const std::string TemporalLowPass::m_oPipeOutName0	= std::string("Value");

TemporalLowPass::TemporalLowPass()
    : TransformFilter(TemporalLowPass::m_oFilterName, Poco::UUID{"C7886CA1-C0AE-472e-A925-0405F74DFC43"})
    , m_pPipeInGeoDoubleArray(nullptr)
    , m_oPipeOutDouble(this, m_oPipeOutName0)
    , m_calculator{}
{
    // Defaultwerte der Parameter setzen
    m_calculator.addToParameterContainer(parameters_);

    setInPipeConnectors({{Poco::UUID("5B19949A-3A6C-4464-A304-067A7FDA117D"), m_pPipeInGeoDoubleArray, "Value", 0, ""}});
    setOutPipeConnectors({{Poco::UUID("92E4B2A4-EBFD-4849-820E-BB81EC1AB945"), &m_oPipeOutDouble, m_oFilterName, 0, ""}});
    setVariantID(Poco::UUID("4C6BF27F-C66B-46a9-9112-88692EF9ACBE"));
} // TemporalLowPass

void TemporalLowPass::setParameter()
{
    TransformFilter::setParameter();
    m_calculator.setParameter(parameters_);

} // setParameter


void TemporalLowPass::arm (const fliplib::ArmStateBase& p_rArmtate)
{
	if (p_rArmtate.getStateID() == eSeamStart)
	{
        m_calculator.clear();
    } // if

	if (m_oVerbosity >= eHigh)
	{
		wmLog(eDebug, "Filter '%s' armed at armstate %i\n", m_oFilterName.c_str(),  p_rArmtate.getStateID());
	} // if
} // arm



bool TemporalLowPass::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup)
{
	m_pPipeInGeoDoubleArray  = dynamic_cast<pipe_scalar_t*>(&p_rPipe);
	return BaseFilter::subscribe( p_rPipe, p_oGroup );
} // subscribe



void TemporalLowPass::proceed(const void* sender, fliplib::PipeEventArgs& e)
{
	poco_assert_dbg(m_pPipeInGeoDoubleArray != nullptr); // to be asserted by graph editor

	// get input data
	const GeoDoublearray &rGeoDoubleArrayIn	= m_pPipeInGeoDoubleArray->read(m_oCounter);

    const auto oGeoDoubleOut = m_calculator.temporalLowPass<0>(rGeoDoubleArrayIn);

    preSignalAction();
	m_oPipeOutDouble.signal(oGeoDoubleOut); // invoke linked filter(s)

} // proceed


} // namespace filter
} // namespace precitec
