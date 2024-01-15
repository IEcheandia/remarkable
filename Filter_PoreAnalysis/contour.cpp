/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		HS
 * 	@date		2013
 * 	@brief		Filter which computes the contour of a pore.
 */

// WM includes
#include "contour.h"
#include "direction.h"

#include "fliplib/PipeEventArgs.h"
#include "fliplib/Parameter.h"
#include "overlay/overlayCanvas.h"
#include "overlay/overlayPrimitive.h"
#include "image/image.h"
#include "module/moduleLogger.h"
#include "filter/algoStl.h"
#include <fliplib/TypeToDataTypeImpl.h>

using namespace fliplib;
namespace precitec {
	using namespace geo2d;
	using namespace interface;
	using namespace image;
namespace filter {


const std::string Contour::m_oFilterName 			( std::string("Contour") );			///< Filter name.
const std::string Contour::m_oPipeOutBlobName 		( std::string("ContourBlobsOut") );	///< Pipe: Scalar out-pipe.
const std::string Contour::m_oPipeOutPointXName 		( std::string("ContourPointXOut") );	///< Pipe: Scalar out-pipe.
const std::string Contour::m_oPipeOutPointYName 		( std::string("ContourPointYOut") );	///< Pipe: Scalar out-pipe.
const std::string Contour::m_oPipeOutPointsName 		( std::string("ContourPointsOut") );	///< Pipe:  out-pipe.

Contour::Contour()
	:
	TransformFilter			( m_oFilterName, Poco::UUID{"5E6FFD4E-9CD8-4d61-91D0-CB7369899F86"} ),
	m_pPipeInImageFrame		( nullptr ),
	m_pPipeInBlob			( nullptr ),
	m_oPipeOutBlob			( this, m_oPipeOutBlobName ),
	m_oPipeOutPointX			( this, m_oPipeOutPointXName ),
	m_oPipeOutPointY			( this, m_oPipeOutPointYName ),
	m_oPipeOutPoints			( this, m_oPipeOutPointsName )
{
    setInPipeConnectors({{Poco::UUID("56BA5E35-718A-453a-A22D-0D7B0D9C7A99"), m_pPipeInImageFrame, "ContourImageIn", 1, ""},
    {Poco::UUID("1A1414E7-30A2-4aa9-81C5-CD480821D597"), m_pPipeInBlob, "ContourBlobsIn", 1, ""}});
    setOutPipeConnectors({{Poco::UUID("182A85E2-CF22-4bbd-B953-A7F31775B582"), &m_oPipeOutBlob, m_oPipeOutBlobName, 0, ""},
    {Poco::UUID("44282030-3FC9-429A-93E7-10EE044A688F"), &m_oPipeOutPointX, m_oPipeOutPointXName, 0, ""},
    {Poco::UUID("58F36BC9-8F5B-4197-9592-B3A703563AAD"), &m_oPipeOutPointY, m_oPipeOutPointYName, 0, ""},
    {Poco::UUID("88B07345-BD39-4DF7-A717-EA34FAD28F95"), &m_oPipeOutPoints, m_oPipeOutPointsName, 0, ""}});
    setVariantID(Poco::UUID("64164211-46B2-4735-8D7E-81A14C1AC5D7"));
} // Contour



void Contour::setParameter() {
	TransformFilter::setParameter();
} // setParameter.



void Contour::paint() {
	if(m_oVerbosity < eLow){
		return;
	} // if

	if (m_oSpTrafo.isNull())
    {
        return;
    }


	const Trafo					&rTrafo					( *m_oSpTrafo );
	OverlayCanvas				&rCanvas				( canvas<OverlayCanvas>(m_oCounter) );
	OverlayLayer				&rLayerContour			( rCanvas.getLayerContour());
	OverlayLayer				&rLayerPosition			( rCanvas.getLayerPosition());
	const std::vector<Blob>&	rBlobVector				( m_oBlobsOut.getData() );

	for(auto oBlobIt = std::begin(rBlobVector); oBlobIt != std::end(rBlobVector); ++oBlobIt) {
		if (oBlobIt->si == 0) {
			continue;
		}

		if(m_oVerbosity >= eMedium){
			const Point		oStartPoint		(oBlobIt->startx, oBlobIt->starty);
			rLayerPosition.add<OverlayCross>(rTrafo(oStartPoint), Color::Green() );
		} // if

		for(auto oContourPosIt = std::begin(oBlobIt->m_oContour); oContourPosIt != std::end(oBlobIt->m_oContour); ++oContourPosIt) {
			rLayerContour.add<OverlayPoint>(rTrafo(*oContourPosIt), Color::Orange() );
		} // for
	} // for
} // paint



bool Contour::subscribe(BasePipe& p_rPipe, int p_oGroup) {
	if ( p_rPipe.type() == typeid(ImageFrame) ) {
		m_pPipeInImageFrame = dynamic_cast<image_pipe_t*>(&p_rPipe);
	} // if
	else {
		m_pPipeInBlob		= dynamic_cast<blob_pipe_t*>(&p_rPipe);
	} // else

	return BaseFilter::subscribe( p_rPipe, p_oGroup );
} // subscribe



void Contour::proceedGroup(const void* p_pSender, PipeGroupEventArgs&)
{
	poco_assert_dbg(m_pPipeInBlob != nullptr && m_pPipeInImageFrame != nullptr);

	const ImageFrame&			rFrame				( m_pPipeInImageFrame->read(m_oCounter) );
	const GeoBlobarray&			rGeoBlobsIn			( m_pPipeInBlob->read(m_oCounter) );

    poco_assert(rFrame.context() == rGeoBlobsIn.context());

	m_oBlobsOut	= rGeoBlobsIn.ref();
	m_oSpTrafo	= rFrame.context().trafo();

	Doublearray	oValX;
	Doublearray	oValY;
	std::vector<geo2d::AnnotatedDPointarray> vecDPointArray;
	AnnotatedDPointarray oValPoint;

	if ( rFrame.data().isValid() == false )
	{
		oValX.getData().push_back(0.0);
		oValY.getData().push_back(0.0);
		DPoint point;
		point.x = 0.0;
		point.y = 0.0;
		oValPoint.getData().push_back(point);
		oValX.getRank().push_back(0);
		oValY.getRank().push_back(0);
		oValPoint.getRank().push_back(0);
		vecDPointArray.push_back(oValPoint);

		GeoBlobarray oGeoBlobarray(rGeoBlobsIn.context(), m_oBlobsOut, rGeoBlobsIn.analysisResult(), NotPresent);
		preSignalAction();
		m_oPipeOutBlob.signal(oGeoBlobarray);
		m_oPipeOutPointX.signal( GeoDoublearray(rGeoBlobsIn.context(), oValX, rGeoBlobsIn.analysisResult(), NotPresent) );
		m_oPipeOutPointY.signal( GeoDoublearray(rGeoBlobsIn.context(), oValY, rGeoBlobsIn.analysisResult(), NotPresent) );
		m_oPipeOutPoints.signal( GeoVecAnnotatedDPointarray (rGeoBlobsIn.context(), vecDPointArray, rGeoBlobsIn.analysisResult(), NotPresent) );

		return;
	}

	calcContour(rFrame.data(), m_oBlobsOut, oValX, oValY, vecDPointArray, m_oFilterName);

	GeoBlobarray				oGeoBlobarray		( rGeoBlobsIn.context(), m_oBlobsOut, rGeoBlobsIn.analysisResult(), rGeoBlobsIn.rank() ); // full geo rank, detailed rank in array
	preSignalAction();
	m_oPipeOutBlob.signal(oGeoBlobarray);
	m_oPipeOutPointX.signal( GeoDoublearray(rGeoBlobsIn.context(), oValX, rGeoBlobsIn.analysisResult(),  rGeoBlobsIn.rank() ) );
	m_oPipeOutPointY.signal( GeoDoublearray(rGeoBlobsIn.context(), oValY, rGeoBlobsIn.analysisResult(),  rGeoBlobsIn.rank() ) );
	m_oPipeOutPoints.signal( GeoVecAnnotatedDPointarray (rGeoBlobsIn.context(), vecDPointArray, rGeoBlobsIn.analysisResult(),  rGeoBlobsIn.rank() ) );

} // proceed

} // namespace filter
} // namespace precitec
