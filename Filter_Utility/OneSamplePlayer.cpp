/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		SB
 * 	@date		2013
 * 	@brief		This filter retrieves data elements from a global a buffer. The data elements were stored in the buffer by the BufferRecorder filter.
 */

// WM includes
#include "OneSamplePlayer.h"
#include "OneSampleRecorder.h"   // fuer die statische Methode
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

const std::string OneSamplePlayer::m_oFilterName 		( "OneSamplePlayer" );
const std::string OneSamplePlayer::m_oPipeOutName("Sample");				///< Pipe: Data out-pipe.


OneSamplePlayer::OneSamplePlayer() :
TransformFilter(OneSamplePlayer::m_oFilterName, Poco::UUID{"43518555-141F-4824-A1AA-16C5590E5628"}),
	m_oSlotNumber(1),
	m_pPipeInImage			( nullptr ),
	m_oPipeOutData			(this, OneSamplePlayer::m_oPipeOutName)
{

	parameters_.add("SlotNumber", fliplib::Parameter::TYPE_Int32, m_oSlotNumber);

    setInPipeConnectors({{Poco::UUID("47006B22-68C4-4DF1-A672-3EB32153A44F"), m_pPipeInImage, "ImageFrame", 0, ""}});
    setOutPipeConnectors({{Poco::UUID("531B089B-BC01-41CE-9B56-6CBEA65BB5EF"), &m_oPipeOutData, m_oPipeOutName, 0, ""}});
    setVariantID(Poco::UUID("441F8197-512E-488D-8071-114C62DD5E1F"));
} // CTor.



OneSamplePlayer::~OneSamplePlayer()
{

} // DTor.



void OneSamplePlayer::setParameter()
{
	TransformFilter::setParameter();

	m_oSlotNumber = parameters_.getParameter("SlotNumber").convert<int>();

} // setParameter.



/*virtual*/ void OneSamplePlayer::arm(const fliplib::ArmStateBase& state)
{
	if(m_oVerbosity >= eHigh)
	{
		wmLog(eDebug, "Filter '%s' armed at armstate %i\n", m_oFilterName.c_str(),  state.getStateID());

	} // if

} // arm



bool OneSamplePlayer::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup)
{
	m_pPipeInImage = dynamic_cast< fliplib::SynchronePipe < ImageFrame > * >(&p_rPipe);

	return BaseFilter::subscribe( p_rPipe, p_oGroup );

} // subscribe



void OneSamplePlayer::proceed(const void* p_pSender, fliplib::PipeEventArgs& e)
{
	poco_assert_dbg( m_pPipeInImage != nullptr); // to be asserted by graph editor

	const ImageFrame& rFrame(m_pPipeInImage->read(m_oCounter));
	m_oCurrentNumber = rFrame.context().imageNumber();

	double oData = 0.0;
	ValueRankType oRank;
	ResultType oAnalysisResult;
	interface::SmpTrafo oTrafo;
	OneSampleRecorder::GetSampleValue(m_oSlotNumber - 1, m_oCurrentNumber, oData, oRank, oTrafo, oAnalysisResult);
	assert(!oTrafo.isNull());
	interface::ImageContext oImageContext(rFrame.context(), oTrafo);             // erzeuge eine neue Instanz eines ImageContexts aus dem alten ImageContext, wobei die Transformation die ist, die aus der Zwischenablage kam.

	// store result
	geo2d::Doublearray oOut;
	oOut.assign( 1 );
	oOut[0] = std::tie( oData, oRank );

	// some debug output
	if( m_oVerbosity >= eHigh )
	{
		wmLog( eInfo, "OneSamplePlayer: Reading %f !\n", oData );
	}

	// send the data out ...
	const GeoDoublearray oGeoDoubleOut(oImageContext, oOut, oAnalysisResult, (double)(oRank) / 255.0);
	preSignalAction(); m_oPipeOutData.signal( oGeoDoubleOut );

} // proceedGroup


} // namespace filter
} // namespace precitec
