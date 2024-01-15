/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		KIH, HS
 * 	@date		2012
 * 	@brief		Filter that extracts blobs of a binary image by means of a labeling algorithm.
 */

#include "blobDetection.h"

#include "overlay/overlayCanvas.h"
#include "overlay/overlayPrimitive.h"
#include "filter/algoStl.h"
#include <fliplib/TypeToDataTypeImpl.h>

using namespace fliplib;
namespace precitec {
	using namespace image;
	using namespace geo2d;
	using namespace interface;
namespace filter {

const std::string BlobDetection::FILTERNAME 		= std::string("BlobDetection");
const std::string BlobDetection::PIPENAME0			= std::string("Blobs");
const std::string BlobDetection::PIPENAME1			= std::string("PosX");
const std::string BlobDetection::PIPENAME2			= std::string("PosY");


BlobDetection::BlobDetection() :
	TransformFilter		( BlobDetection::FILTERNAME, Poco::UUID{"EA64DF62-A328-4e30-929D-0BF63331CC74"} ),
	m_pPipeInImageFrame	( nullptr ),
	m_oPipeOutBlob		( this, BlobDetection::PIPENAME0  ),
	m_oPipeOutPosX		( this, BlobDetection::PIPENAME1  ),
	m_oPipeOutPosY		( this, BlobDetection::PIPENAME2  ),
	m_oMaxNbBlobs		( 300 ),
	m_oMinBlobSize		( 4 )
{
	// Defaultwerte der Parameter setzen
	parameters_.add("nspotsmax",	Parameter::TYPE_UInt32, m_oMaxNbBlobs);
	parameters_.add("MinBlobSize",	Parameter::TYPE_UInt32, m_oMinBlobSize);

    setInPipeConnectors({{Poco::UUID("4C297EFF-2FE2-4d83-8F90-C63A690BD933"), m_pPipeInImageFrame, "ImageFrame", 0, ""}});
    setOutPipeConnectors({{Poco::UUID("F3DFB563-6E37-4e12-BDF7-30526CE27201"), &m_oPipeOutBlob, PIPENAME0, 0, ""},
    {Poco::UUID("421D7235-732E-4114-897F-83ED61F5C728"), &m_oPipeOutPosX, PIPENAME1, 0, ""},
    {Poco::UUID("BC45D024-D106-411e-A965-AF0CC1E31784"), &m_oPipeOutPosY, PIPENAME2, 0, ""}});
    setVariantID(Poco::UUID("BD3A9302-8FE1-4856-AD4D-22C8E64C8437"));

}



BlobDetection::~BlobDetection() {
	m_oDataBlobDetection.free();
}



void BlobDetection::setParameter() {
	BaseFilter::setParameter();
	m_oMaxNbBlobs	= parameters_.getParameter("nspotsmax").convert<unsigned int>();
	m_oMinBlobSize	= parameters_.getParameter("MinBlobSize").convert<unsigned int>();

	m_oDataBlobDetection.alloc(m_oMaxNbBlobs); // free im destruktor nicht vergessen !!!!!!
}

void BlobDetection::paint() {
	if(m_oVerbosity < eLow){
		return;
	} // if

    if (m_oSpTrafo.isNull())
    {
        return;
    }


	const Trafo					&rTrafo					( *m_oSpTrafo );
	OverlayCanvas				&rCanvas				( canvas<OverlayCanvas>(m_oCounter) );
	OverlayLayer				&rLayerPosition			( rCanvas.getLayerPosition());

	std::vector<Blob>&	rBlobVector		( m_oBlobArray.getData() );
	for(auto oBlobIt = std::begin(rBlobVector); oBlobIt != std::end(rBlobVector); ++oBlobIt) {
		if (oBlobIt->si == 0) {
			continue;
		}
		const double	oCenterOfMassX		( static_cast<double>(oBlobIt->sx / oBlobIt->si) );
		const double	oCenterOfMassY		( static_cast<double>(oBlobIt->sy / oBlobIt->si) );
		const Point		oBlob				( roundToT<int>(oCenterOfMassX), roundToT<int>(oCenterOfMassY) );
		rLayerPosition.add( new  OverlayCross(rTrafo(oBlob), Color::Orange() ) );
	}
}



bool BlobDetection::subscribe(BasePipe& p_rPipe, int p_oGroup) {
	m_pPipeInImageFrame  = dynamic_cast< fliplib::SynchronePipe < ImageFrame > * >(&p_rPipe);

	return BaseFilter::subscribe( p_rPipe, p_oGroup );
} // subscribe



void BlobDetection::proceed(const void* sender, PipeEventArgs& e) {
#if !defined(NDEBUG)
	poco_check_ptr(m_pPipeInImageFrame); // derzeit ist nicht garantiert, dass alle in-pipes verbunden sind.
#endif

	// Empfangenes Frame auslesen
	const ImageFrame&				rFrame			( m_pPipeInImageFrame->read(m_oCounter) );
	m_oSpTrafo	= rFrame.context().trafo();
	const BImage& rImageIn = rFrame.data();

	if ( rImageIn.isValid() == false ) {
		GeoBlobarray		oGeoBlobarray		( rFrame.context(), m_oBlobArray, rFrame.analysisResult(), NotPresent ); // bad geo rank
		preSignalAction(); m_oPipeOutBlob.signal(oGeoBlobarray);

		return; // RETURN
	} // if


	// translate BImage to SSF_SF_InputStruct

	SSF_SF_InputStruct sgmima;
	sgmima.img=rImageIn.begin();
	sgmima.npixx =rImageIn.width();
	sgmima.npixy=rImageIn.height();
	sgmima.pitch=rImageIn.rowBegin(1) - rImageIn.rowBegin(0);
	sgmima.roistart=sgmima.img;
	sgmima.roix0=0;
	sgmima.roiy0=0;
	sgmima.roidx=sgmima.npixx;
	sgmima.roidy=sgmima.npixy;

	segmentateimage(sgmima, m_oDataBlobDetection, m_oMinBlobSize);

	// translate blob DataBlobDetectionT to array

	m_oBlobArray.assign(m_oDataBlobDetection.nspots, Blob(), eRankMax); // we do not obtain rank information, however all output should be ok
	m_oArrayPosX.assign(m_oDataBlobDetection.nspots, 0, eRankMax); // we do not obtain rank information, however all output should be ok
	m_oArrayPosY.assign(m_oDataBlobDetection.nspots, 0, eRankMax); // we do not obtain rank information, however all output should be ok

	std::vector<Blob>&		rBlobVector		( m_oBlobArray.getData() );
	std::vector<double>&	rPosXVector		( m_oArrayPosX.getData() );
	std::vector<double>&	rPosYVector		( m_oArrayPosY.getData() );

	for(int i = 0; i != m_oDataBlobDetection.nspots; ++i) {
		rBlobVector[i]	= m_oDataBlobDetection.outspot[i];

		const double	oCenterOfMassX		= static_cast<double>(rBlobVector[i].sx / rBlobVector[i].si);
		const double	oCenterOfMassY		= static_cast<double>(rBlobVector[i].sy / rBlobVector[i].si);
		rPosXVector[i]	= oCenterOfMassX;
		rPosYVector[i]	= oCenterOfMassY;
	} // for

	GeoBlobarray		oGeoBlobarray		( rFrame.context(), m_oBlobArray, rFrame.analysisResult(), Limit ); // full geo rank, detailed rank in array
	GeoDoublearray		oGeoPosXArray		( rFrame.context(), m_oArrayPosX, rFrame.analysisResult(), Limit ); // full geo rank, detailed rank in array
	GeoDoublearray		oGeoPosYArray		( rFrame.context(), m_oArrayPosY, rFrame.analysisResult(), Limit ); // full geo rank, detailed rank in array

	preSignalAction();
	m_oPipeOutBlob.signal(oGeoBlobarray);
	m_oPipeOutPosX.signal(oGeoPosXArray);
	m_oPipeOutPosY.signal(oGeoPosYArray);
} // proceed

} // namespace filter
} // namespace precitec

