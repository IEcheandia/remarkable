/*!
 *  @copyright:		Precitec Vision GmbH & Co. KG
 *  @author			Simon Hilsenbeck (HS), LB
 *  @date			2018
 *  @file
 *  @brief			Binarizes an image depending on threshold.
 */

// local includes
#include "binarizeDynamic.h"

#include "image/image.h"				///< BImage
#include "overlay/overlayPrimitive.h"	///< overlay
#include "filter/algoImage.h"
#include "module/moduleLogger.h"

#include <fliplib/TypeToDataTypeImpl.h>


namespace precitec
{
namespace filter
{

void binarizeImage(const geo2d::Doublearray& thresholdIn, const image::BImage& imageIn, const ComparisonType& comparisonType,
                            const BinarizeType& binarizeType, image::BImage& rBinarizedImageOut)
{
    const image::Size2d& sizeImageIn (imageIn.size());

    // ignore ranks
    const auto& rCurrentInputData = thresholdIn.getData()[0];

    // input is double, but our parameter is byte
    const byte distanceToMeanIntensity = rCurrentInputData > 0 ? (rCurrentInputData < 255 ? rCurrentInputData : 255) : 0;

    // if the threshold is set to 0, the binarization makes no sense and is therefore disabled - the input image is passed through directly ...
    if (distanceToMeanIntensity == 0)
    {
        rBinarizedImageOut.resize(sizeImageIn);
        const auto& width = sizeImageIn.width;
        const auto& height = sizeImageIn.height;
        for (int i = 0; i < height; ++i)
        {
            memcpy(rBinarizedImageOut[i], imageIn[i], width);
        }
        return;
    }

    rBinarizedImageOut.resize(sizeImageIn);

    // image processing
    switch (binarizeType)
    {
        case BinarizeType::eGlobal: // global and dynamically
        calcBinarizeDynamic(imageIn, comparisonType, distanceToMeanIntensity, rBinarizedImageOut);
        break;
        case BinarizeType::eLocal: // local and dynamically
        calcBinarizeLocal(imageIn, comparisonType, distanceToMeanIntensity, rBinarizedImageOut);
        break;
        case BinarizeType::eStatic: // global and statically
        calcBinarizeStatic(imageIn, comparisonType, distanceToMeanIntensity, rBinarizedImageOut);
        break;
    default:
        calcBinarizeDynamic(imageIn, comparisonType, distanceToMeanIntensity, rBinarizedImageOut);
        break;
    }
}

const std::string BinarizeDynamic::m_oFilterName 		( std::string("BinarizeDynamic") );
const std::string BinarizeDynamic::m_oPipeOut1Name		( std::string("ImageFrame") );

BinarizeDynamic::BinarizeDynamic() :
	TransformFilter(BinarizeDynamic::m_oFilterName, Poco::UUID{"4a62138d-4a6d-48f4-ae49-48cfd46539ec"}),
	m_pPipeInImageFrame			( nullptr ),
	m_pPipeInThreshold          ( nullptr ),
	m_oPipeOutImgFrame			( this, m_oPipeOut1Name ),
	m_oComparisonType			( eLess ),
	m_oBinarizeType				( BinarizeType::eGlobal )
{
	// Defaultwerte der Parameter setzen
	parameters_.add("ComparisonType",	fliplib::Parameter::TYPE_int,	static_cast<int>(m_oComparisonType));
	parameters_.add("BinarizeType",		fliplib::Parameter::TYPE_int,	static_cast<int>(m_oBinarizeType));

    setInPipeConnectors({{Poco::UUID("218854a5-9536-46fe-88e8-95c3ba548398"), m_pPipeInImageFrame, "Image", 1, "image"},
    {Poco::UUID("3b93a294-2c6c-4b24-b4be-6bd85f1ac650"), m_pPipeInThreshold, "Threshold", 1, "threshold"}});
    setOutPipeConnectors({{Poco::UUID("6daf9e60-a916-4f10-b768-116ffba224c5"), &m_oPipeOutImgFrame, "ImageFrame", 0, ""}});
    setVariantID(Poco::UUID("ef52b11c-b5cc-484a-8052-f79a7b7eb097"));
}



void BinarizeDynamic::setParameter()
{
	TransformFilter::setParameter();
	m_oComparisonType		= static_cast<ComparisonType>(parameters_.getParameter("ComparisonType").convert<int>());
	m_oBinarizeType			= static_cast<BinarizeType>(parameters_.getParameter("BinarizeType").convert<int>());
}



bool BinarizeDynamic::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup)
{
	if ( p_rPipe.tag() == "image" )
	{
		m_pPipeInImageFrame = dynamic_cast<image_pipe_t*>(&p_rPipe);
	}
	if ( p_rPipe.tag() == "threshold" )
	{
        m_pPipeInThreshold = dynamic_cast<threshold_pipe_t *>(&p_rPipe);
	}

	return BaseFilter::subscribe( p_rPipe, p_oGroup );
}



void BinarizeDynamic::proceedGroup(const void* sender, fliplib::PipeGroupEventArgs& e)
{
	// get data from frame
	const interface::ImageFrame& rFrameIn(m_pPipeInImageFrame->read(m_oCounter));

	const image::BImage& rImageIn (rFrameIn.data());
    const auto & rThresholdIn(m_pPipeInThreshold->read(m_oCounter).ref());

	m_oSpTrafo = rFrameIn.context().trafo();

    // input validity check
	if (rImageIn.isValid() == false || rThresholdIn.size() == 0)
    {
        // signal null image
		const interface::ImageFrame oNewFrame(rFrameIn.context(), image::BImage(), rFrameIn.analysisResult());
		preSignalAction();
        m_oPipeOutImgFrame.signal(oNewFrame);

		return;
	}

	auto& rBinarizedImageOut = m_oBinImageOut[m_oCounter % g_oNbPar];

    binarizeImage(rThresholdIn, rImageIn, m_oComparisonType, m_oBinarizeType, rBinarizedImageOut);
    const interface::ImageFrame oFrameOut(rFrameIn.context(), rBinarizedImageOut, rFrameIn.analysisResult());
    preSignalAction();
    m_oPipeOutImgFrame.signal(oFrameOut);
}

void BinarizeDynamic::paint()
{
	if ((m_oVerbosity < eMedium) || m_oSpTrafo.isNull()) {
		return;
	}

	const interface::Trafo		&rTrafo					( *m_oSpTrafo );
	image::OverlayCanvas		&rCanvas				( canvas<image::OverlayCanvas>(m_oCounter) );
	image::OverlayLayer			&rLayerImage			( rCanvas.getLayerImage());

	const auto		oPosition	=	rTrafo(geo2d::Point(0, 0));
	const auto		oTitle		=	image::OverlayText("Binarized image", image::Font(), geo2d::Rect(150, 18), image::Color::Black());

	rLayerImage.add<image::OverlayImage>(oPosition, m_oBinImageOut[m_oCounter % g_oNbPar], oTitle);
} // paint

} // namespace filter
} // namespace precitec
