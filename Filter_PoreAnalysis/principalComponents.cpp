/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		HS
 * 	@date		2013
 * 	@brief		Filter which computes the half axes of a pore.
 */

// WM includes
#include "principalComponents.h"

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


const std::string PrincipalComponents::m_oFilterName 				( "PrincipalComponents" );				///< Filter name.
const std::string PrincipalComponents::m_oPipeOutLength1Name 		( "PrincipalComponentsLength1Out" );	///< Pipe: Scalar out-pipe.
const std::string PrincipalComponents::m_oPipeOutLength2Name 		( "PrincipalComponentsLength2Out" );	///< Pipe: Scalar out-pipe.
const std::string PrincipalComponents::m_oPipeOutAxesRatioName 		( "PrincipalComponentsRatioOut" );		///< Pipe: Scalar out-pipe.


PrincipalComponents::PrincipalComponents()
	:
	TransformFilter			( m_oFilterName, Poco::UUID{"FA52950C-49F4-4d81-BFDF-6E4F75B49972"} ),
	m_pPipeInImageFrame		( nullptr ),
	m_pPipeInBlob			( nullptr ),
	m_oPipeOutLength1		( this, m_oPipeOutLength1Name ),
	m_oPipeOutLength2		( this, m_oPipeOutLength2Name ),
	m_oPipeOutAxesRatio		( this, m_oPipeOutAxesRatioName )
{
    setInPipeConnectors({{Poco::UUID("56C5FA6D-1A0A-482d-A09B-F2A892B13596"), m_pPipeInImageFrame, "PrincipalComponentsImageIn", 1, ""},
    {Poco::UUID("70EB0EB9-56D3-4ba9-BF88-192500AC7615"), m_pPipeInBlob, "PrincipalComponentsBlobsIn", 1, ""}});
    setOutPipeConnectors({{Poco::UUID("AA641DE9-9BF6-4667-8D65-23FBBCD96837"), &m_oPipeOutLength1, m_oPipeOutLength1Name, 0, ""},
    {Poco::UUID("034D7CE9-8D40-4d1f-A7E8-8040324918C8"), &m_oPipeOutLength2, m_oPipeOutLength2Name, 0, ""},
    {Poco::UUID("90F4C8D3-7F58-4a06-97AC-9465322078FA"), &m_oPipeOutAxesRatio, m_oPipeOutAxesRatioName, 0, ""}});
    setVariantID(Poco::UUID("8D8CD7B5-137C-4641-8523-3A5C5D8CA24A"));
} // PrincipalComponents



void PrincipalComponents::setParameter() {
	TransformFilter::setParameter();
} // setParameter.



void PrincipalComponents::paint() {
	if(m_oVerbosity < eLow){
		return;
	} // if

    if (m_oSpTrafo.isNull())
    {
        return;
    }


	const Trafo					&rTrafo					( *m_oSpTrafo );
	OverlayCanvas				&rCanvas				( canvas<OverlayCanvas>(m_oCounter) );
	OverlayLayer				&rLayerLine				( rCanvas.getLayerLine());
	OverlayLayer				&rLayerText				( rCanvas.getLayerText());

	const std::vector<Blob>&	rBlobVector				( m_oInputBlob.ref().getData() );

	auto						oMajorAxesIt			( std::begin(m_oMajorAxes) );
	for(auto oBlobIt = std::begin(rBlobVector); oBlobIt != std::end(rBlobVector); ++oBlobIt) {
		//if (oBlobIt->si == 0) {
		//	continue;
		//} // if
		const Point		oCenterOfMass		( static_cast<int>(oBlobIt->sx / oBlobIt->si), static_cast<int>(oBlobIt->sy / oBlobIt->si) );

		const Point		oEv1End				( oCenterOfMass + Point( roundToT<int>(oMajorAxesIt->m_oEigenVector1.x * oMajorAxesIt->m_oEigenVector1Length / 2),
																	 roundToT<int>(oMajorAxesIt->m_oEigenVector1.y * oMajorAxesIt->m_oEigenVector1Length / 2) ) );
		rLayerLine.add( new  OverlayLine( rTrafo(oCenterOfMass), rTrafo(oEv1End), Color::Yellow() ) );

		const Point		oEv2End				( oCenterOfMass + Point( roundToT<int>(oMajorAxesIt->m_oEigenVector2.x * oMajorAxesIt->m_oEigenVector2Length / 2),
																	 roundToT<int>(oMajorAxesIt->m_oEigenVector2.y * oMajorAxesIt->m_oEigenVector2Length / 2) ) );
		rLayerLine.add( new  OverlayLine( rTrafo(oCenterOfMass), rTrafo(oEv2End), Color::Yellow() ) );

		if(m_oVerbosity > eLow){
			std::ostringstream	oMsg;
			oMsg << "PcRatio: " << std::setprecision(2) << std::fixed << oMajorAxesIt->m_oEigenVector1Length / oMajorAxesIt->m_oEigenVector2Length;
			rLayerText.add(new OverlayText(oMsg.str(), Font(14), rTrafo(Rect(oBlobIt->xmax, oBlobIt->ymin + 3*15, 200, 20)), Color::Yellow()));  // +3*15 because 4th
		} // if

		++oMajorAxesIt;
	} // for
} // paint



bool PrincipalComponents::subscribe(BasePipe& p_rPipe, int p_oGroup) {
	if ( p_rPipe.type() == typeid(ImageFrame) ) {
		m_pPipeInImageFrame = dynamic_cast<image_pipe_t*>(&p_rPipe);
	} // if
	else {
		m_pPipeInBlob		= dynamic_cast<blob_pipe_t*>(&p_rPipe);
	} // else

	return BaseFilter::subscribe( p_rPipe, p_oGroup );
} // subscribe



void PrincipalComponents::proceedGroup(const void* p_pSender, PipeGroupEventArgs&) {
	poco_assert_dbg(m_pPipeInBlob != nullptr && m_pPipeInImageFrame != nullptr);

	// Read in pipes

	const ImageFrame&			rFrame				( m_pPipeInImageFrame->read(m_oCounter) );

    m_oInputBlob    =	m_pPipeInBlob->read(m_oCounter);

	const std::vector<Blob>&	rBlobVector			( m_oInputBlob.ref().getData() );

	m_oSpTrafo	= rFrame.context().trafo();

	if ( rFrame.data().isValid() == false ) {
		const GeoDoublearray		oGeoLength1Out		( m_oInputBlob.context(), m_oArrayLength1, m_oInputBlob.analysisResult(), NotPresent); // bad rank
		const GeoDoublearray		oGeoLength2Out		( m_oInputBlob.context(), m_oArrayLength2, m_oInputBlob.analysisResult(), NotPresent); // bad rank
		const GeoDoublearray		oGeoAxesRatioOut	( m_oInputBlob.context(), m_oArrayAxesRatio, m_oInputBlob.analysisResult(), NotPresent); // bad rank

		preSignalAction();
		m_oPipeOutLength1.signal(oGeoLength1Out);	// invoke linked filter(s)
		m_oPipeOutLength2.signal(oGeoLength2Out);	// invoke linked filter(s)
		m_oPipeOutAxesRatio.signal(oGeoAxesRatioOut);	// invoke linked filter(s)

		return; // RETURN
	} // if

	m_oMajorAxes.clear();
	for(auto oBlobIt = std::begin(rBlobVector); oBlobIt != std::end(rBlobVector); ++oBlobIt) {
		const Point		oBoundinxBoxStart	( oBlobIt->xmin, oBlobIt->ymin );
		const Point		oBoundinxBoxEnd		( oBlobIt->xmax, oBlobIt->ymax );
		const Rect		oBoundinxBox		( oBoundinxBoxStart, oBoundinxBoxEnd );

		if (oBoundinxBox.isEmpty() == true) {
			//wmLog(eDebug, "Filter '%s': Empty pore discarded. Size of bounding box: (%iX%i)\n", m_oFilterName.c_str(),  oBoundinxBox.width(), oBoundinxBox.height() );
			m_oMajorAxes.push_back(MajorAxes()); // dummy value 1 will lead to bad rank as done in PoreStatistics::calcEigenValues ()
			continue;
		} // if

		m_oPoreStatistics.reset();
		m_oPoreStatistics.calcMomentsV2(oBoundinxBox, rFrame.data());
		m_oPoreStatistics.calcEigenValues();
		m_oPoreStatistics.calcEigenVectors();
		m_oPoreStatistics.calcCenterOfMass();
		m_oMajorAxes.push_back(m_oPoreStatistics.getMajorAxes());
	} // for


	m_oArrayLength1.assign(m_oMajorAxes.size(), 0, eRankMax); // we do not obtain rank information, however all input should be ok
	m_oArrayLength2.assign(m_oMajorAxes.size(), 0, eRankMax); // we do not obtain rank information, however all input should be ok
	m_oArrayAxesRatio.assign(m_oMajorAxes.size(), 0, eRankMax); // we do not obtain rank information, however all input should be ok

	auto						oMajorAxesIt			( m_oMajorAxes.cbegin() );
	const auto					oMajorAxesEndIt			( m_oMajorAxes.cend() );
	auto						oLength1OutIt			( m_oArrayLength1.getData().begin() );
	auto						oLength2OutIt			( m_oArrayLength2.getData().begin() );
	auto						oAxesRatioOutIt			( m_oArrayAxesRatio.getData().begin() );
	auto						oLength1RankOutIt		( m_oArrayLength1.getRank().begin() );
	auto						oLength2RankOutIt		( m_oArrayLength2.getRank().begin() );
	auto						oAxesRatioRankOutIt		( m_oArrayAxesRatio.getRank().begin() );

	while(oMajorAxesIt != oMajorAxesEndIt) {
		poco_assert_dbg(oMajorAxesIt->m_oEigenVector2Length != 0); // should be asserted in calculation in PoreStatistics

		*oLength1OutIt		= oMajorAxesIt->m_oEigenVector1Length;
		*oLength2OutIt		= oMajorAxesIt->m_oEigenVector2Length;
		*oAxesRatioOutIt	= *oLength1OutIt / *oLength2OutIt;

		if (oMajorAxesIt->m_oEigenVector2Length == PoreStatistics::m_oEvBadValue) { // dummy value used in PoreStatistics::calcEigenValues ()
			*oLength1RankOutIt		= eRankMin;
			*oLength2RankOutIt		= eRankMin;
			*oAxesRatioRankOutIt	= eRankMin;
		} // if

		++oMajorAxesIt;
		++oLength1OutIt;
		++oLength2OutIt;
		++oAxesRatioOutIt;
		++oLength1RankOutIt;
		++oLength2RankOutIt;
		++oAxesRatioRankOutIt;
	} // while

	const GeoDoublearray		oGeoLength1Out		( m_oInputBlob.context(), m_oArrayLength1, m_oInputBlob.analysisResult(), m_oInputBlob.rank() ); // detailed rank in array
	const GeoDoublearray		oGeoLength2Out		( m_oInputBlob.context(), m_oArrayLength2, m_oInputBlob.analysisResult(), m_oInputBlob.rank() ); // detailed rank in array
	const GeoDoublearray		oGeoAxesRatioOut	( m_oInputBlob.context(), m_oArrayAxesRatio, m_oInputBlob.analysisResult(), m_oInputBlob.rank() ); // detailed rank in array

	preSignalAction();
	m_oPipeOutLength1.signal(oGeoLength1Out);	// invoke linked filter(s)
	m_oPipeOutLength2.signal(oGeoLength2Out);	// invoke linked filter(s)
	m_oPipeOutAxesRatio.signal(oGeoAxesRatioOut);	// invoke linked filter(s)

} // proceed


} // namespace filter
} // namespace precitec
