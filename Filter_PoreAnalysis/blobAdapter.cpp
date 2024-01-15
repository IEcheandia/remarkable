/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		OS
 * 	@date		02 / 2015
 * 	@brief		Filter that allows the access to every value coming out of the pore analysis
 */

// WM includes
#include "fliplib/PipeEventArgs.h"
#include "fliplib/Parameter.h"
#include "overlay/overlayCanvas.h"
#include "overlay/overlayPrimitive.h"
#include "image/image.h"
#include "module/moduleLogger.h"
#include "filter/algoStl.h"
#include "util/calibDataSingleton.h"
#include "blobAdapter.h"
#include <fliplib/TypeToDataTypeImpl.h>

using namespace fliplib;
namespace precitec {
	using namespace geo2d;
	using namespace interface;
	using namespace image;
namespace filter {


const std::string BlobAdapter::m_oFilterName 				( "BlobAdapter" );
const std::string BlobAdapter::m_oPipeOutDataName 			( "BlobAdapterDataOut" );


BlobAdapter::BlobAdapter() :
	TransformFilter			( m_oFilterName, Poco::UUID{"E908F654-A31D-49ED-BB11-6B974ABECE88"} ),
	m_pPipeInBlob			( nullptr ),
	m_oPipeOutData			( this, m_oPipeOutDataName ),
	m_oHwRoiY				( 0 ),
	m_oBlobAdapter			( eArea )
{
	parameters_.add("BlobComponent", Parameter::TYPE_int, static_cast<int>(m_oBlobAdapter));

    setInPipeConnectors({{Poco::UUID("C062B90D-26C3-40AB-A175-E292AF24C6EA"), m_pPipeInBlob, "BlobAdapterBlobsIn", 0, ""}});
    setOutPipeConnectors({{Poco::UUID("1E967935-E335-43A8-A2FC-02D4BDE16800"), &m_oPipeOutData, m_oPipeOutDataName, 0, ""}});
    setVariantID(Poco::UUID("4F28FDA7-6F09-4B73-AB13-9DD3FD0000F0"));
} // BlobAdapter



void BlobAdapter::setParameter()
{
	TransformFilter::setParameter();
	m_oBlobAdapter			= static_cast<BlobAdapterType>(parameters_.getParameter("BlobComponent").convert<int>());
}

void BlobAdapter::paint()
{
	if(m_oVerbosity < eLow)
	{
		return;
	}
	return;
}

bool BlobAdapter::subscribe(BasePipe& p_rPipe, int p_oGroup)
{
	m_pPipeInBlob = dynamic_cast<blob_pipe_t*>(&p_rPipe);
	return BaseFilter::subscribe( p_rPipe, p_oGroup );
}

void BlobAdapter::proceed(const void* p_pSender, PipeEventArgs& )
{
	if ( m_pPipeInBlob == nullptr )
		return;

	bool isValid = true;

	if (m_pPipeInBlob == nullptr) isValid = false;
	poco_assert_dbg( m_pPipeInBlob != nullptr );

	// Read in pipe
	const GeoBlobarray&			rGeoBlobList		( m_pPipeInBlob->read(m_oCounter) );
	const Blobarray&			rBlobList			( rGeoBlobList.ref() );
	const std::size_t			oNbBlobsIn			( rBlobList.size() );

	if (oNbBlobsIn <= 0) isValid = false; // Keine Blobs, Rank schlecht
	if (rGeoBlobList.rank() <=0 ) isValid = false; // Rank schon vorher schlecht

	m_oSpTrafo	= rGeoBlobList.context().trafo();
	m_oHwRoiY	= rGeoBlobList.context().HW_ROI_y0;

	if(oNbBlobsIn > 0)
	{
		m_oArrayData.assign(oNbBlobsIn, 0, eRankMax);
	}
	else
	{
		m_oArrayData.assign(1, 0, eRankMin);
	}

	auto		oBlobInIt			= rBlobList.getData().cbegin();
	const auto	oBlobInEndIt		= rBlobList.getData().cend();
	auto		oArrayOutIt			= m_oArrayData.getData().begin();
	double h=0, w=0;

	if (isValid)
	{
		while (oBlobInIt != oBlobInEndIt)
		{
			switch (m_oBlobAdapter)
			{
			case eMassCenterX:
				*oArrayOutIt = (double)oBlobInIt->sx;
				break;
			case eMassCenterY:
				*oArrayOutIt = (double)oBlobInIt->sy;
				break;
			case eMassCenterNormalizingFactor:
				*oArrayOutIt = (double)oBlobInIt->si;
				break;
			case eMassCenterXNormalized:
				*oArrayOutIt = 0;
				if (oBlobInIt->si == 0) break;
				*oArrayOutIt = oBlobInIt->sx / (double)oBlobInIt->si;
				break;
			case eMassCenterYNormalized:
				*oArrayOutIt = 0;
				if (oBlobInIt->si == 0) break;
				*oArrayOutIt = oBlobInIt->sy / (double)oBlobInIt->si;
				break;
			case eArea:
				*oArrayOutIt = (double)oBlobInIt->npix;
				break;
			case eXMin:
				*oArrayOutIt = (double)oBlobInIt->xmin;
				break;
			case eXMax:
				*oArrayOutIt = (double)oBlobInIt->xmax;
				break;
			case eYMin:
				*oArrayOutIt = (double)oBlobInIt->ymin;
				break;
			case eYMax:
				*oArrayOutIt = (double)oBlobInIt->ymax;
				break;
			case eWidth:
				w = oBlobInIt->xmax - oBlobInIt->xmin;
				if (w < 0) w = -w;
				*oArrayOutIt = w;
				break;
			case eHeight:
				h = oBlobInIt->ymax - oBlobInIt->ymin;
				if (h < 0) h = -h;
				*oArrayOutIt = h;
				break;
			case eStartX:
				*oArrayOutIt = (double)oBlobInIt->startx;
				break;
			case eStartY:
				*oArrayOutIt = (double)oBlobInIt->starty;
				break;

			default:
				std::ostringstream oMsg;
				oMsg << "No case for switch argument: " << m_oBlobAdapter;
				wmLog(eWarning, oMsg.str().c_str());
				isValid = false;
			}
			++oBlobInIt;
			++oArrayOutIt;
		} // while
	}
	if (isValid)
	{
		const GeoDoublearray		oGeoArrayOut(rGeoBlobList.context(), m_oArrayData, rGeoBlobList.analysisResult(), rGeoBlobList.rank()); // detailed rank in array
        preSignalAction();
		m_oPipeOutData.signal(oGeoArrayOut);	// invoke linked filter(s)
	}
	else
	{
		const GeoDoublearray		oGeoArrayOut(rGeoBlobList.context(), m_oArrayData, rGeoBlobList.analysisResult(), 0.0); // detailed rank in array
        preSignalAction();
		m_oPipeOutData.signal(oGeoArrayOut);	// invoke linked filter(s)
	}
} // proceed



} // namespace filter
} // namespace precitec
