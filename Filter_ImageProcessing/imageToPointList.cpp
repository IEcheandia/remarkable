/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		OS
 * 	@date		02/2015
 * 	@brief		Gets a bunch of points out of an image and sends it to the next filter
 */

#include "imageToPointList.h"

#include "image/image.h"
#include "overlay/overlayPrimitive.h"
#include "module/moduleLogger.h"
#include "geo/array.h"

#include <fliplib/TypeToDataTypeImpl.h>

using namespace fliplib;
namespace precitec {
	using namespace interface;
	using namespace image;
	using namespace geo2d;
namespace filter {

const std::string ImageToPointList::m_oFilterName 			= std::string("ImageToPointList");
const std::string ImageToPointList::PIPENAME				= std::string("ImageFrame");
const std::string ImageToPointList::PIPENAME_POINTARRAY_X	= std::string("PointListX");
const std::string ImageToPointList::PIPENAME_POINTARRAY_Y	= std::string("PointListY");
const std::string ImageToPointList::PIPENAME_POINTLISTLIST	= std::string("Contours");

ImageToPointList::ImageToPointList() :
	TransformFilter		( ImageToPointList::m_oFilterName, Poco::UUID{"0C212A95-35DE-4989-A24B-2C09191C2B3D"} ),
	m_pPipeInImageFrame	(NULL),
	pipeXValues_		( new SynchronePipe< GeoDoublearray >( this, ImageToPointList::PIPENAME_POINTARRAY_X ) ),
	pipeYValues_		( new SynchronePipe< GeoDoublearray >( this, ImageToPointList::PIPENAME_POINTARRAY_Y ) ),
	pipeValues_			( new SynchronePipe< GeoVecAnnotatedDPointarray >( this, ImageToPointList::PIPENAME_POINTLISTLIST ) ),
	m_oMode      		( 0 )
{
	parameters_.add("Mode", Parameter::TYPE_int, static_cast<int>(m_oMode));
	m_isValid = true;

    setInPipeConnectors({{Poco::UUID("80BD4709-CE5A-4E18-B1DC-028E54FBD088"), m_pPipeInImageFrame, "ImageFrame", 0, ""}});
    setOutPipeConnectors({{Poco::UUID("4EC11151-2DEF-49FD-9F39-D2B577241626"), pipeValues_, "Contours", 0, ""}, {Poco::UUID("FAE0FE07-B451-4B30-BF67-A05F2F7BED89"), pipeXValues_, "PointListX", 0, ""},
    {Poco::UUID("045D5782-C13D-4A24-886C-FC617CF2D1E7"), pipeYValues_, "PointListY", 0, ""}});
    setVariantID(Poco::UUID("65B12FCA-8B5D-4EC6-B73C-A61BD6092D1A"));
}

ImageToPointList::~ImageToPointList()
{
	delete pipeXValues_;
	delete pipeYValues_;
	delete pipeValues_;
}

void ImageToPointList::setParameter()
{
	TransformFilter::setParameter();
	m_oMode = parameters_.getParameter("Mode");
}

void ImageToPointList::paint()
{
	using namespace precitec::image;
	if(m_oVerbosity <= eMedium || m_oSpTrafo.isNull() || m_outVecDPointArray.size() == 0)
	{
		return;
	} // if

    OverlayCanvas	&rCanvas ( canvas<OverlayCanvas>(m_oCounter) );
    OverlayLayer	&rLayerPosition ( rCanvas.getLayerPosition());

    auto oColor = Color::Cyan();


    const auto & points = m_outVecDPointArray.front().getData();

    for (auto & rPoint : points)
    {
        auto oCanvasPoint = m_oSpTrafo->apply(geo2d::Point( (int)std::round(rPoint.x), (int)std::round(rPoint.y)));
        rLayerPosition.add<OverlayPoint>(oCanvasPoint, oColor);
    }

	return;
}

bool ImageToPointList::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup)
{
	m_pPipeInImageFrame  = dynamic_cast< fliplib::SynchronePipe < ImageFrame > * >(&p_rPipe);
	return BaseFilter::subscribe( p_rPipe, p_oGroup );
}

void ImageToPointList::proceed(const void* sender, PipeEventArgs& e)
{
	m_isValid = true;

	poco_assert_dbg(m_pPipeInImageFrame != nullptr); // to be asserted by graph editor
	if (m_pPipeInImageFrame == nullptr) m_isValid = false;

	// Empfangenes Frame auslesen
	const ImageFrame &rFrame (m_pPipeInImageFrame->read(m_oCounter));
	m_oSpTrafo	= rFrame.context().trafo();

	BImage image = rFrame.data();
	Doublearray	oValX;
	Doublearray	oValY;

	m_outVecDPointArray.clear();
	AnnotatedDPointarray oValPoint;

	switch (m_oMode)
	{
	case 0:
		DoSimple(image, oValX, oValY, oValPoint);
		break;

	case 1:
		DoQualas(image, oValX, oValY, oValPoint, true, true);
		break;

	case 2:
		DoQualas(image, oValX, oValY, oValPoint, true, false);
		break;

	case 3:
		DoQualas(image, oValX, oValY, oValPoint, false, true);
		break;
	}

	m_outVecDPointArray.push_back(oValPoint);

	double rank = (m_isValid) ? 1.0 : 0.0;
	preSignalAction();
	pipeXValues_->signal( GeoDoublearray(rFrame.context(), oValX, rFrame.analysisResult(), rank) );
	pipeYValues_->signal( GeoDoublearray(rFrame.context(), oValY, rFrame.analysisResult(), rank) );
	pipeValues_->signal( GeoVecAnnotatedDPointarray (rFrame.context(), m_outVecDPointArray, rFrame.analysisResult(), rank) );

	return;
}

void ImageToPointList::DoQualas(BImage & image, Doublearray & oValX, Doublearray & oValY, AnnotatedDPointarray & oValPoint, bool useTop, bool useBottom)
{
	int maxY = image.size().height;
	int maxX = image.size().width;

	for (int x=0; x<maxX; x++)
	{
		if (useTop)	for(int y=0; y<maxY; y++)
		{
			if (image[y][x]>250)
			{
				oValX.getData().push_back(x);
				oValY.getData().push_back(y);
				DPoint point;
				point.x = x;
				point.y = y;
				oValPoint.getData().push_back(point);
				oValX.getRank().push_back(255);
				oValY.getRank().push_back(255);
				oValPoint.getRank().push_back(255);
				break;
			}
		}

		if (useBottom) for(int y=maxY-1; y>=0; y--)
		{
			if (image[y][x]>250)
			{
				oValX.getData().push_back(x);
				oValY.getData().push_back(y);
				DPoint point;
				point.x = x;
				point.y = y;
				oValPoint.getData().push_back(point);
				oValX.getRank().push_back(255);
				oValY.getRank().push_back(255);
				oValPoint.getRank().push_back(255);
				break;
			}
		}
	}
}

void ImageToPointList::DoSimple(BImage & image, Doublearray & oValX, Doublearray & oValY, AnnotatedDPointarray & oValPoint)
{
	int maxY = image.size().height;
	int maxX = image.size().width;

	for (int x=0; x<maxX; x++)
	{
		for(int y=0; y<maxY; y++)
		{
			if (image[y][x]>250)
			{
				oValX.getData().push_back(x);
				oValY.getData().push_back(y);

				DPoint point;
				point.x = x;
				point.y = y;
				oValPoint.getData().push_back(point);

				oValX.getRank().push_back(255);
				oValY.getRank().push_back(255);
				oValPoint.getRank().push_back(255);
			}
		}
	}
}

}}

