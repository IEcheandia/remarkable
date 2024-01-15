/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		SB
 * 	@date		2013
 * 	@brief		This filter retrieves data elements from a global a buffer. The data elements were stored in the buffer by the BufferRecorder filter.
 */

// WM includes
#include "setPosition.h"
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

const std::string SetPosition::m_oFilterName 		( "SetPosition" );
const std::string SetPosition::m_oPipeOutName("ImageFrame");				///< Pipe: Data out-pipe.


SetPosition::SetPosition() :
TransformFilter(SetPosition::m_oFilterName, Poco::UUID{"4530189D-F613-4966-B46B-3C9960318DCF"}),
	m_pPipeInImage			( nullptr ),
	m_pPipeInValue(nullptr),
	m_oPipeOutImage(this, SetPosition::m_oPipeOutName)
{
    setInPipeConnectors({{Poco::UUID("09992471-1207-4E8D-B852-758C163788F7"), m_pPipeInImage, "ImageFrame", 1, ""},
    {Poco::UUID("0B790415-404C-4E9D-910B-D88325B66E20"), m_pPipeInValue, "Value", 1, ""}});
    setOutPipeConnectors({{Poco::UUID("40965E18-2898-4CE6-8EDA-058A4083FB36"), &m_oPipeOutImage, m_oPipeOutName, 0, ""}});
    setVariantID(Poco::UUID("5E0F901F-85F0-48A2-9FC2-4C9B2DC5A1DC"));
} // CTor.



SetPosition::~SetPosition()
{

} // DTor.



void SetPosition::setParameter()
{
	TransformFilter::setParameter();
} // setParameter.



/*virtual*/ void SetPosition::arm(const fliplib::ArmStateBase& state)
{
	if(m_oVerbosity >= eHigh)
	{
		wmLog(eDebug, "Filter '%s' armed at armstate %i\n", m_oFilterName.c_str(),  state.getStateID());

	} // if

} // arm



bool SetPosition::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup)
{
	if (p_rPipe.type() == typeid(ImageFrame))
	{
		m_pPipeInImage = dynamic_cast<fliplib::SynchronePipe <ImageFrame> *>(&p_rPipe);
	}
	else if (p_rPipe.type() == typeid(GeoDoublearray)) {
		m_pPipeInValue = dynamic_cast<fliplib::SynchronePipe < GeoDoublearray > *>(&p_rPipe);
	}

	return BaseFilter::subscribe( p_rPipe, p_oGroup );

} // subscribe



void SetPosition::proceedGroup(const void* sender, fliplib::PipeGroupEventArgs& e)
{
	poco_assert_dbg( m_pPipeInImage != nullptr); // to be asserted by graph editor

	const ImageFrame& rImage(m_pPipeInImage->read(m_oCounter));

	interface::ImageContext oImageContext(rImage.context());             // erzeuge eine neue Instanz eines ImageContexts aus dem alten ImageContext

	const std::vector<double>& rValueInputData = m_pPipeInValue->read(m_oCounter).ref().getData();
	const std::vector<int>& rValueInputRank = m_pPipeInValue->read(m_oCounter).ref().getRank();

	// lies die Position aus dem Input - wenn die einen guten Rank hat, dann trage die Position in den Kontext ein:
	if (rValueInputRank.size() > 0 && rValueInputRank[0] > eRankMin)
	{
		long oPositionValue = static_cast<long>(rValueInputData[0] + 0.5);
		oImageContext.setPosition(oPositionValue);
	}

	// send the data out ...
	ImageFrame oImageOut(oImageContext, rImage.data(), rImage.analysisResult()); // oImageContext ist der modifizierte Kontext mit der neuen Position, rImage.data() holt die Bilddaten, rImage.analysisResult holt das Resultat,
	preSignalAction(); m_oPipeOutImage.signal( oImageOut );

} // proceedGroup


} // namespace filter
} // namespace precitec
