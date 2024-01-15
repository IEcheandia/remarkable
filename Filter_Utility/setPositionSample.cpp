/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		SB
 * 	@date		2013
 * 	@brief		This filter retrieves data elements from a global a buffer. The data elements were stored in the buffer by the BufferRecorder filter.
 */

// WM includes
#include "setPositionSample.h"
#include <filter/buffer.h>
#include <fliplib/PipeEventArgs.h>
#include <fliplib/Parameter.h>
#include <image/image.h>
#include <module/moduleLogger.h>
#include <filter/armStates.h>
#include <filter/productData.h>
#include "event/resultType.h"
#include "common/geoContext.h"
#include <fliplib/TypeToDataTypeImpl.h>

namespace precitec {
using namespace interface;
using namespace image;
namespace filter {

const std::string SetPositionSample::m_oFilterName 		( "SetPositionSample" );
const std::string SetPositionSample::m_oPipeOutName("SampleFrame");				///< Pipe: Data out-pipe.


SetPositionSample::SetPositionSample() :
TransformFilter(SetPositionSample::m_oFilterName, Poco::UUID{"E04E4989-76A6-464F-B5C8-B11E2C9A354B"}),
	m_pPipeInSample			( nullptr ),
	m_pPipeInValue(nullptr),
	m_oPipeOutSample(this, SetPositionSample::m_oPipeOutName)
{
    setInPipeConnectors({{Poco::UUID("894600F3-8A9B-44BA-B2E9-F2B307B07864"), m_pPipeInSample, "SampleFrame", 1, "SampleFrame"},
    {Poco::UUID("A1DD02D7-6651-41D0-9058-770096058B26"), m_pPipeInValue, "Value", 1, "Value"}});
    setOutPipeConnectors({{Poco::UUID("48E2CEEB-A43E-4BD1-BD15-80B937C801B2"), &m_oPipeOutSample, m_oPipeOutName, 0, ""}});
    setVariantID(Poco::UUID("8D48CDC9-F0FD-48F7-AD64-919EBBF85F5D"));
} // CTor.



SetPositionSample::~SetPositionSample()
{

} // DTor.



void SetPositionSample::setParameter()
{
	TransformFilter::setParameter();
} // setParameter.



/*virtual*/ void SetPositionSample::arm(const fliplib::ArmStateBase& state)
{
	if(m_oVerbosity >= eHigh)
	{
		wmLog(eDebug, "Filter '%s' armed at armstate %i\n", m_oFilterName.c_str(),  state.getStateID());

	} // if

} // arm



bool SetPositionSample::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup)
{
	if (p_rPipe.type() == typeid(GeoDoublearray) && p_rPipe.tag() == "SampleFrame")
	{
		m_pPipeInSample = dynamic_cast<fliplib::SynchronePipe <GeoDoublearray> *>(&p_rPipe);
	}
	else if (p_rPipe.type() == typeid(GeoDoublearray) && p_rPipe.tag() == "Value") {
		m_pPipeInValue = dynamic_cast<fliplib::SynchronePipe < GeoDoublearray > *>(&p_rPipe);
	}

	return BaseFilter::subscribe( p_rPipe, p_oGroup );

} // subscribe



void SetPositionSample::proceedGroup(const void* sender, fliplib::PipeGroupEventArgs& e)
{
	poco_assert_dbg( m_pPipeInSample != nullptr); // to be asserted by graph editor
	poco_assert_dbg(m_pPipeInValue != nullptr); // to be asserted by graph editor

	const GeoDoublearray& rSample(m_pPipeInSample->read(m_oCounter));

	interface::ImageContext oSampleContext(rSample.context());             // erzeuge eine neue Instanz eines ImageContexts aus dem alten ImageContext

	const std::vector<double>& rValueInputData = m_pPipeInValue->read(m_oCounter).ref().getData();
	const std::vector<int>& rValueInputRank = m_pPipeInValue->read(m_oCounter).ref().getRank();

	// lies die Position aus dem Input - wenn die einen guten Rank hat, dann trage die Position in den Kontext ein:
	if (rValueInputRank.size() > 0 && rValueInputRank[0] > eRankMin)
	{
		long oPositionValue = static_cast<long>(rValueInputData[0] + 0.5);
		oSampleContext.setPosition(oPositionValue);
	}

	// send the data out ...
	GeoDoublearray oSampleOut(oSampleContext, rSample.ref(), rSample.analysisResult()); // oImageContext ist der modifizierte Kontext mit der neuen Position, rImage.data() holt die Bilddaten, rImage.analysisResult holt das Resultat,
	preSignalAction(); m_oPipeOutSample.signal( oSampleOut );

} // proceedGroup


} // namespace filter
} // namespace precitec
