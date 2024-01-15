/**
* 	@file
* 	@copyright	Precitec Vision GmbH & Co. KG
* 	@author		LB
* 	@date		2019
* 	@brief		Filter 'SurfaceCalculatorAdaptInput'. Equal to SurfaceCalculator, with an additional input to enlarge the input ROI
*/

// WM includes

#include "surfaceCalculator.h"
#include "surfaceCalculatorAdaptInput.h"

#include "fliplib/Parameter.h"
#include "overlay/overlayPrimitive.h"
#include "image/image.h"
#include "module/moduleLogger.h"
#include <fliplib/TypeToDataTypeImpl.h>

using namespace fliplib;
namespace precitec {
	using namespace geo2d;
	using namespace interface;
	using namespace image;
	namespace filter
{

const std::string SurfaceCalculatorAdaptInput::m_oFilterName("SurfaceCalculatorAdaptInput");		///< Filter name
const std::string SurfaceCalculatorAdaptInput::m_oPipeOutSurfaceInfoName("SurfaceInfo");	///< out-pipe for surface information

SurfaceCalculatorAdaptInput::SurfaceCalculatorAdaptInput()
	:
	TransformFilter(m_oFilterName, Poco::UUID{"B8BF5B2B-0574-4F1C-8F9A-3C6C7EB561E2"}),
	m_pPipeInImageFrame(nullptr),
	m_pPipeInExtendedROIFrame(nullptr),
	m_oPipeOutSurfaceInfo(this, m_oPipeOutSurfaceInfoName),
	m_oTileWidth(30),
	m_oTileJumpX(30),
	m_oTileHeight(30),
	m_oTileJumpY(30),
    m_oEnsureTile(false),
	m_lastImageTmp(Size2d(0,0)),
	m_lastTitleImageTmp("")
{
    m_oSurfaceInfo.reset();
	SurfaceCalculator::initializeFilterParameterContainer(m_oSurfaceInfo, parameters_);
	parameters_.add("TileWidth", Parameter::TYPE_int, m_oTileWidth);
	parameters_.add("TileJumpX", Parameter::TYPE_int, m_oTileJumpX);
	parameters_.add("TileHeight", Parameter::TYPE_int, m_oTileHeight);
	parameters_.add("TileJumpY", Parameter::TYPE_int, m_oTileJumpY);
    parameters_.add("EnsureTile", Parameter::TYPE_bool, m_oEnsureTile);

    setInPipeConnectors({{Poco::UUID("E8D91529-DA84-482C-898E-32A77D197A3C"), m_pPipeInImageFrame, "image", 1, "image"},
    {Poco::UUID("5FE21309-567C-42AB-8016-6D48FA018AB6"), m_pPipeInExtendedROIFrame, "extendedROI", 1, "extendedROI"}});
    setOutPipeConnectors({{Poco::UUID("328B54D9-BB19-4108-A268-4EF1B7232CA4"), &m_oPipeOutSurfaceInfo, m_oPipeOutSurfaceInfoName, 0, ""}});
    setVariantID(Poco::UUID("CCCD2D95-B967-4A41-8E16-7848B20DD248"));
}

void SurfaceCalculatorAdaptInput::setParameter()
{

	TransformFilter::setParameter();

	m_oTileWidth = parameters_.getParameter("TileWidth");
	m_oTileJumpX = parameters_.getParameter("TileJumpX");
	m_oTileHeight = parameters_.getParameter("TileHeight");
	m_oTileJumpY = parameters_.getParameter("TileJumpY");
    m_oEnsureTile = parameters_.getParameter("EnsureTile");
    SurfaceCalculator::updateFromParameterContainer(m_oSurfaceInfo, parameters_);

	if (m_oTileJumpX <= 0)
    {
        wmLog(eWarning, "Invalid value for Parameter TileJumpX (%d), set to 1 \n", m_oTileJumpX);
        m_oTileJumpX = 1; // Eine Sprungweite kleiner/gleich Null sollte hier ankommen
    }
	if (m_oTileJumpY <= 0)
    {
        wmLog(eWarning, "Invalid value for Parameter TileJumpY (%d), set to 1 \n", m_oTileJumpY);
        m_oTileJumpY = 1;
    }
    if (m_oTileWidth <= 0)
    {
        wmLog(eWarning, "Invalid value for Parameter TileWidth (%d), set to 1 \n", m_oTileWidth);
        m_oTileWidth = 1;
    }
    if (m_oTileHeight <= 0)
    {
        wmLog(eWarning, "Invalid value for Parameter TileHeight (%d), set to 1 \n", m_oTileHeight);
        m_oTileHeight = 1;
    }


} // setParameter.

bool SurfaceCalculatorAdaptInput::subscribe(BasePipe& p_rPipe, int p_oGroup)
{
	if ( p_rPipe.tag() == "image" )
	{
		m_pPipeInImageFrame = dynamic_cast<const image_pipe_t*>(&p_rPipe);
	}
	if ( p_rPipe.tag() == "extendedROI" )
	{
		m_pPipeInExtendedROIFrame = dynamic_cast<const image_pipe_t*>(&p_rPipe);
	}

	return BaseFilter::subscribe(p_rPipe, p_oGroup);
} // subscribe

SmpTrafo SurfaceCalculatorAdaptInput::updateNewROI(const Size2d minOutSize, const SmpTrafo& roiInTrafo, const Size2d& roiInSize, const SmpTrafo& sourceImageTrafo, const BImage& sourceImage)
{
	//compute the ROI size
	int w = std::max(minOutSize.width, roiInSize.width);
	int h = std::max(minOutSize.height, roiInSize.height);

	//compute the ROI borders in context(0,0)
	geo2d::Point imageCenter = roiInTrafo->apply(geo2d::Point(roiInSize.width / 2, roiInSize.height / 2));
	int x_0 = imageCenter.x - w/2;
	int y_0 = imageCenter.y - h/2;

	//compute the ROI border in context of the extended ROI
	geo2d::Point offset = sourceImageTrafo->apply(geo2d::Point{0, 0});
	SmpTrafo oNewTrafo{new LinearTrafo{std::max(x_0, sourceImageTrafo->dx()), std::max(y_0, sourceImageTrafo->dy())}};
	assert(oNewTrafo->dx() >= sourceImageTrafo->dx());
	assert(oNewTrafo->dy() >= sourceImageTrafo->dy());

	int x_ext = oNewTrafo->dx() - sourceImageTrafo->dx();
	int y_ext = oNewTrafo->dy() - sourceImageTrafo->dy();

	//clip ROI borders if necessary
	w = std::min(w, sourceImage.width() - x_ext);
	h = std::min(h, sourceImage.height() - y_ext);

	m_oNewROI = sourceImage.isValid() ? BImage(sourceImage, Rect{x_ext, y_ext, w, h}, true) : sourceImage;
	return oNewTrafo;
}

void SurfaceCalculatorAdaptInput::proceedGroup(const void* p_pSender, PipeGroupEventArgs& p_rEvent)
{
	m_hasPainting = false;
	poco_assert_dbg(m_pPipeInImageFrame != nullptr);
	poco_assert_dbg(m_pPipeInExtendedROIFrame != nullptr);

	m_badInput = false;

	geo2d::SurfaceInfoarray oOutInfo;

	//compute the input image and update trafo
	bool inputROIfitsImage = true;
	interface::ResultType oAnalysisResult;
	interface::ImageContext oContext;
	const BImage & rImageIn = [&oAnalysisResult, &oContext, &inputROIfitsImage, this]
					(int oTileWidth, int oTileHeight, const ImageFrame& rFrameROIIn, const ImageFrame& rFrameExtendedROIIn) -> const BImage &
	{

		//check input pipes
		const BImage&			rROIIn(rFrameROIIn.data());
		const BImage&			rExtendedROIIn(rFrameExtendedROIIn.data());

		oAnalysisResult = std::max(rFrameROIIn.analysisResult(), rFrameExtendedROIIn.analysisResult());

		auto oTrafoROI = rFrameROIIn.context().trafo();
		auto oTrafoExtendedROI = rFrameExtendedROIIn.context().trafo();

		if ( !rROIIn.isValid() )
		{
			//input ROI is not valid,  but we can use the trafo information to build a newROI with the same center
			inputROIfitsImage = false;

			SmpTrafo oNewTrafo = updateNewROI(Size2D{oTileWidth, oTileHeight},
				oTrafoROI, rROIIn.size(),
				oTrafoExtendedROI, rExtendedROIIn);
			oContext = ImageContext(rFrameExtendedROIIn.context(), oNewTrafo);
			return m_oNewROI;
		}

		int oImageWidth = rROIIn.width();
		int oImageHeight = rROIIn.height();
		int oExtendedROIWidth = rExtendedROIIn.width();
		int oExtendedROIHeight = rExtendedROIIn.height();

		auto ROIStart_InExtendedROI = oTrafoExtendedROI->applyReverse(oTrafoROI->apply(geo2d::Point{0, 0}));

		//ROIs are shallow copies of images, we can check if the ROI belong to the same image by checking the pixel address
		if ( ROIStart_InExtendedROI.x >= oExtendedROIWidth
			|| ROIStart_InExtendedROI.y >= oExtendedROIHeight
			|| rROIIn.begin() != rExtendedROIIn.rowBegin(ROIStart_InExtendedROI.y) + ROIStart_InExtendedROI.x )
		{
			wmLog(eWarning, "ROI is not part of extended ROI \n");
			inputROIfitsImage = false;
			oContext = rFrameROIIn.context();
			return rROIIn;
		}

		if ( oTileWidth <= oImageWidth && oTileHeight <= oImageHeight )
		{
			//input image is valid, no need to check extended ROI
			inputROIfitsImage = true;
			oContext = rFrameROIIn.context();
			return rROIIn;
		}

		inputROIfitsImage = false;

		if ( oTrafoExtendedROI->dx() > oTrafoROI->dx()
			|| oTrafoExtendedROI->dy() > oTrafoROI->dy()
			|| oTrafoExtendedROI->dx() + oExtendedROIWidth  < oTrafoROI->dx() + oImageWidth
			|| oTrafoExtendedROI->dy() + oExtendedROIHeight < oTrafoROI->dy() + oImageHeight
			)
		{
			wmLog(eWarning, "Extended ROI (context %d, %d size %d x %d) is not compatible with input ROI (context %d, %d size %d x %d) \n",
				oTrafoExtendedROI->dx(), oTrafoExtendedROI->dy(), oExtendedROIWidth, oExtendedROIHeight,
				oTrafoROI->dx(), oTrafoROI->dy(), oImageWidth, oImageHeight);

			//we can't create a new ROI, just return the current one
			oContext = rFrameROIIn.context();
			return rROIIn;
		}

		//build a new ROI centered on the first
		SmpTrafo oNewTrafo = updateNewROI(Size2D{oTileWidth, oTileHeight},
			oTrafoROI, rROIIn.size(),
			oTrafoExtendedROI, rExtendedROIIn);
		oContext = ImageContext(rFrameExtendedROIIn.context(), oNewTrafo);
		return m_oNewROI;
	}
	(m_oTileWidth, m_oTileHeight, m_pPipeInImageFrame->read(m_oCounter), m_pPipeInExtendedROIFrame->read(m_oCounter));

	m_oSpTrafo = oContext.trafo();

	m_oSurfaceInfo.setTileContainer(TileContainer{});
    int oImageWidth = rImageIn.width();
    int oImageHeight = rImageIn.height();
    int oActualTileWidth = m_oTileWidth;
    int oActualTileHeight = m_oTileHeight;

    //compute the number of tiles

    int oNumberOfTilesX = 0;
    int oNumberOfTilesY = 0;

	// Calculation of tiles in x direction (number and width)
	{
		int boxGapX = m_oTileJumpX - m_oTileWidth; // Gap zwischen den Tiles. Negativ bei Ueberlappung, pos. bei echtem Abstand

		// Calculation of tiles in x direction
		if ( oActualTileWidth > oImageWidth ) // box width is too large for one tile
		{
			if ( m_oEnsureTile && oImageWidth > 0 )
			{
				oActualTileWidth = oImageWidth;
				oNumberOfTilesX = 1;
				inputROIfitsImage = false;
				//now boxGapX would be wrong, but it's not needed anymore
			}
			else
			{
				oNumberOfTilesX = 0;
			}
		}
		else if ( 2 * oActualTileWidth + boxGapX > oImageWidth ) // 2 boxes doesn't fit in image width
		{
			oNumberOfTilesX = 1;
		}
		else
		{
			oNumberOfTilesX = 1 + (int) ((oImageWidth - oActualTileWidth) / ((double) m_oTileJumpX));
		}
	}

    // Calculation of tiles in y direction (number and height)
	{
		int boxGapY = m_oTileJumpY - m_oTileHeight;

		if ( oActualTileHeight > oImageHeight ) // box height is too large for one tile
		{
			if ( m_oEnsureTile && oImageHeight > 0 )
			{
				oActualTileHeight = oImageHeight;
				oNumberOfTilesY = 1;
				inputROIfitsImage = false;
				//now boxGapY would be wrong, but it's not needed anymore
			}
			else
			{
				oNumberOfTilesY = 0;
			}
		}
		else if ( 2 * oActualTileHeight + boxGapY > oImageHeight ) // 2 boxes doesn't fit in image height
		{
			oNumberOfTilesY = 1;
		}
		else
		{
			oNumberOfTilesY = 1 + (int) ((oImageHeight - oActualTileHeight) / ((double) m_oTileJumpY));
		}
	}
	int oNumberOfAllTiles = oNumberOfTilesX * oNumberOfTilesY; // Geamtzahl der Tiles

	if (oNumberOfAllTiles == 0) // nix zu machen => schnell abschliessen und raus
	{ // ToDo
		m_badInput = true;
		assert(m_oSurfaceInfo.getTileContainer().empty());
		oOutInfo.getData().push_back(m_oSurfaceInfo);
		oOutInfo.getRank().push_back(0);
	}
	else
	{
		m_hasPainting = true;
		// Calc space for tiles in x
		int oTilesSpaceX = m_oTileJumpX * (oNumberOfTilesX - 1) + oActualTileWidth; // hier ist oNumberOfTilesX > 0! Sonst waere Produkt oben Null
		int oTileOffsetX = (int) ((oImageWidth - oTilesSpaceX) / 2.0); // Offset fuer leeren Bereich links und rechts => Tiles liegen dann in der Mitte

		// Calc space for tiles in y
		int oTilesSpaceY = m_oTileJumpY * (oNumberOfTilesY - 1) + oActualTileHeight; // hier ist oNumberOfTilesY > 0! Sonst waere Produkt oben Null
		int oTileOffsetY = (int) ((oImageHeight - oTilesSpaceY) / 2.0); // Offset fuer leeren Bereich oben und unten => Tiles liegen dann in der Mitte

		TileContainer tileContainer(oNumberOfTilesX, oNumberOfTilesY,
			oActualTileWidth, oActualTileHeight,
			oTileOffsetX, oTileOffsetY,
			m_oTileJumpX, m_oTileJumpY
			);


		SurfaceCalculator::computeTiles(m_oSurfaceInfo, tileContainer, rImageIn, m_lastImageTmp, m_lastTitleImageTmp);

        m_oSurfaceInfo.setTileContainer(tileContainer);

        oOutInfo.getData().push_back(m_oSurfaceInfo);
		oOutInfo.getRank().push_back(255);

	}
	double dRank = (m_badInput) ? 0.0 : 1.0;

	const GeoSurfaceInfoarray oGeoInfo = GeoSurfaceInfoarray(oContext, oOutInfo, oAnalysisResult, dRank);

	preSignalAction();
	m_oPipeOutSurfaceInfo.signal(oGeoInfo);			// invoke linked filter(s)
} // proceed


void SurfaceCalculatorAdaptInput::paint()
{
	if (!m_hasPainting) return;
	m_hasPainting = false;

	if (m_oVerbosity < eLow || m_oSpTrafo.isNull()){
		return;
	}
	try
	{
		const Trafo		&rTrafo(*m_oSpTrafo);


		OverlayCanvas&			rOverlayCanvas = canvas<OverlayCanvas>(m_oCounter);

		SurfaceCalculator::paintTiles(rOverlayCanvas, m_oSurfaceInfo.getTileContainer(), rTrafo, m_lastImageTmp, m_lastTitleImageTmp, m_oVerbosity >= eHigh);


	}
	catch (...)
	{
		// Paint ging schief => nur loggen
		wmLog(eWarning, "Exception during painting in SurfaceCalculatorAdaptInput");
	}

} // paint



} // namespace filter
} // namespace precitec
