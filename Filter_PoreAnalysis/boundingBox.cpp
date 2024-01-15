/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		HS
 * 	@date		2013
 * 	@brief		Filter that computes the x and y dimension (bounding box) for each pore in a pore list.
 */

// WM includes
#include "boundingBox.h"
#include "fliplib/PipeEventArgs.h"
#include "fliplib/Parameter.h"
#include "overlay/overlayCanvas.h"
#include "overlay/overlayPrimitive.h"
#include "image/image.h"
#include "module/moduleLogger.h"
#include "filter/algoStl.h"
#include "util/calibDataSingleton.h"
#include <fliplib/TypeToDataTypeImpl.h>

using namespace fliplib;
namespace precitec {
	using namespace geo2d;
	using namespace interface;
	using namespace image;
namespace filter {


const std::string BoundingBox::m_oFilterName 				( "BoundingBox" );					///< Filter name.
const std::string BoundingBox::m_oPipeOutDilationXName 		( "BoundingBoxDilationXOut" );		///< Pipe: Scalar out-pipe.
const std::string BoundingBox::m_oPipeOutDilationYName 		( "BoundingBoxDilationYOut" );		///< Pipe: Scalar out-pipe.


BoundingBox::BoundingBox()
	:
	TransformFilter			( m_oFilterName, Poco::UUID{"CDF8BD70-A7FE-47bc-9BDF-E423FF468E74"} ),
	m_pPipeInBlob			( nullptr ),
	m_oPipeOutDilationX		( this, m_oPipeOutDilationXName ),
	m_oPipeOutDilationY		( this, m_oPipeOutDilationYName )
{
    setInPipeConnectors({{Poco::UUID("27475DB7-3920-4559-9FA9-7FF0EEDE44F5"), m_pPipeInBlob, "BoundingBoxBlobIn", 0, ""}});
    setOutPipeConnectors({{Poco::UUID("C4888747-8C8E-4e8c-AE55-3FCFAAC5A88D"), &m_oPipeOutDilationX, m_oPipeOutDilationXName, 0, ""},
    {Poco::UUID("CE9A1875-3B5D-46d2-AB60-79D1430BB7BC"), &m_oPipeOutDilationY, m_oPipeOutDilationYName, 0, ""}});
    setVariantID(Poco::UUID("7F13EF46-E2E1-4fc6-93C8-A2AF64D2423F"));
} // BoundingBox



void BoundingBox::setParameter() {
	TransformFilter::setParameter();

} // setParameter.



void BoundingBox::paint() {
	if(m_oVerbosity < eLow){
		return;
	} // if

    if (m_oSpTrafo.isNull())
    {
        return;
    }

	const Trafo					&rTrafo				( *m_oSpTrafo );
	OverlayCanvas				&rCanvas			( canvas<OverlayCanvas>(m_oCounter) );
	OverlayLayer				&rLayerLine			( rCanvas.getLayerLine());
	OverlayLayer				&rLayerText			( rCanvas.getLayerText());

	const GeoBlobarray&			rGeoBlobList		( m_pPipeInBlob->read(m_oCounter) );
	const std::vector<Blob>&	rBlobVector			( rGeoBlobList.ref().getData() );
	auto						oItDilationX		( std::begin(m_oArrayDilationX.getData()) );
	auto						oItDilationY		( std::begin(m_oArrayDilationY.getData()) );

	for(auto oBlobIt = std::begin(rBlobVector); oBlobIt != std::end(rBlobVector); ++oBlobIt) {
		const Point		oBoundinxBoxStart	( oBlobIt->xmin, oBlobIt->ymin );
		const Point		oBoundinxBoxEnd		( oBlobIt->xmax, oBlobIt->ymax );
		const Rect		oBoundinxBox		( oBoundinxBoxStart, oBoundinxBoxEnd );
		rLayerLine.add( new  OverlayRectangle( rTrafo(oBoundinxBox), Color::Green() ) );

		if(m_oVerbosity > eLow){
			std::ostringstream	oMsg;
			oMsg << "DX:" << g_oLangKeyUnitMm << ":" << std::setprecision(2) << std::fixed << *oItDilationX;
			rLayerText.add(new OverlayText(oMsg.str(), Font(14), rTrafo(Rect(oBlobIt->xmax, oBlobIt->ymin + 1*15, 200, 20)), Color::Yellow()));  // +1*15 because 2nd
			oMsg.str("");
			oMsg << "DY:" << g_oLangKeyUnitMm << ":" << std::setprecision(2) << std::fixed << *oItDilationY;
			rLayerText.add(new OverlayText(oMsg.str(), Font(14), rTrafo(Rect(oBlobIt->xmax, oBlobIt->ymin + 2*15, 200, 20)), Color::Yellow()));  // +2*15 because 3rd
		} // if

		++oItDilationX;
		++oItDilationY;
	}
}



bool BoundingBox::subscribe(BasePipe& p_rPipe, int p_oGroup) {
	m_pPipeInBlob = dynamic_cast<blob_pipe_t*>(&p_rPipe);

	return BaseFilter::subscribe( p_rPipe, p_oGroup );
} // subscribe



void BoundingBox::proceed(const void* p_pSender, PipeEventArgs& ) {
	if ( m_pPipeInBlob == nullptr )
		return;

	poco_assert_dbg( m_pPipeInBlob != nullptr );

	// Read in pipe
	const GeoBlobarray&			rGeoBlobList		( m_pPipeInBlob->read(m_oCounter) );
	const Blobarray&			rBlobList			( rGeoBlobList.ref() );
	const std::size_t			oNbBlobsIn			( rBlobList.size() );

	m_oSpTrafo	= rGeoBlobList.context().trafo();

	m_oArrayDilationX.assign(oNbBlobsIn, 0, eRankMax); // we do not obtain rank information, however all input should be ok
	m_oArrayDilationY.assign(oNbBlobsIn, 0, eRankMax); // we do not obtain rank information, however all input should be ok

	auto		oBlobInIt			= rBlobList.getData().cbegin();
	const auto	oBlobInEndIt		= rBlobList.getData().cend();
	auto		oDilationXOutIt		= m_oArrayDilationX.getData().begin();
	auto		oDilationYOutIt		= m_oArrayDilationY.getData().begin();

    auto &rCalib(system::CalibDataSingleton::getCalibrationCoords(math::SensorId::eSensorId0));

	while(oBlobInIt != oBlobInEndIt) {

		const auto oDistXMm =
			rCalib.distanceOnHorizontalPlane(oBlobInIt->xmin, oBlobInIt->ymin, oBlobInIt->xmax, oBlobInIt->ymin) / rGeoBlobList.context().SamplingX_;
		const auto oDistYMm =
			rCalib.distanceOnHorizontalPlane(oBlobInIt->xmin, oBlobInIt->ymin, oBlobInIt->xmin, oBlobInIt->ymax) / rGeoBlobList.context().SamplingY_;


		*oDilationXOutIt = oDistXMm;
		*oDilationYOutIt = oDistYMm;

		++oBlobInIt;
		++oDilationXOutIt;
		++oDilationYOutIt;
	} // while

	const GeoDoublearray		oGeoDilationXOut	( rGeoBlobList.context(), m_oArrayDilationX, rGeoBlobList.analysisResult(), rGeoBlobList.rank() ); // detailed rank in array
	const GeoDoublearray		oGeoDilationYOut	( rGeoBlobList.context(), m_oArrayDilationY, rGeoBlobList.analysisResult(), rGeoBlobList.rank() ); // detailed rank in array
	preSignalAction();
	m_oPipeOutDilationX.signal(oGeoDilationXOut);	// invoke linked filter(s)
	m_oPipeOutDilationY.signal(oGeoDilationYOut);	// invoke linked filter(s)

} // proceed



} // namespace filter
} // namespace precitec
