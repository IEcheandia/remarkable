/**
*  @file
*  @copyright   Precitec Vision GmbH & Co. KG
*  @author      GUR
*  @date        2022
*  @brief       Filter 'SurfaceCalculator2Rows'. Calculates 2 rows of tiles in the ROI center.
*/

// WM includes

#include "surfaceCalculator2Rows.h"

#include "fliplib/Parameter.h"
#include "overlay/overlayPrimitive.h"
#include "image/image.h"
#include "module/moduleLogger.h"
#include "filter/algoImage.h"
#include "filter/algoStl.h"
#include "system/typeTraits.h"
#include <fliplib/TypeToDataTypeImpl.h>

using namespace fliplib;

namespace precitec
{
    using namespace geo2d;
    using namespace interface;
    using namespace image;

namespace filter
{

const std::string SurfaceCalculator2Rows::m_oFilterName("SurfaceCalculator2Rows");    ///< Filter name
const std::string SurfaceCalculator2Rows::m_oPipeOutSurfaceInfoName("SurfaceInfo");   ///< out-pipe for surface information



void SurfaceCalculator2Rows::initializeFilterParameterContainer(SurfaceInfo & surfaceInfo, fliplib::ParameterContainer & parameters_)
{
    using fliplib::Parameter;
    parameters_.add("CalcMean", Parameter::TYPE_bool, surfaceInfo.usesMean);
    parameters_.add("CalcRelInt", Parameter::TYPE_bool, surfaceInfo.usesRelBrightness);
    parameters_.add("CalcVariation", Parameter::TYPE_bool, surfaceInfo.usesVariation);
    parameters_.add("CalcMinMiaxDistance", Parameter::TYPE_bool, surfaceInfo.usesMinMaxDistance);
    parameters_.add("CalcSurface", Parameter::TYPE_bool, surfaceInfo.usesSurface);
    parameters_.add("CalcSurfaceX", Parameter::TYPE_bool, surfaceInfo.usesSurfaceX);
    parameters_.add("CalcSurfaceY", Parameter::TYPE_bool, surfaceInfo.usesSurfaceY);
    parameters_.add("CalcStructure", Parameter::TYPE_bool, surfaceInfo.usesStructure);
    parameters_.add("CalcTexture", Parameter::TYPE_bool, surfaceInfo.usesTexture);
}



void SurfaceCalculator2Rows::updateFromParameterContainer(SurfaceInfo & surfaceInfo, fliplib::ParameterContainer & parameters_)
{
    surfaceInfo.usesMean = parameters_.getParameter("CalcMean");
    surfaceInfo.usesRelBrightness = parameters_.getParameter("CalcRelInt");
    surfaceInfo.usesVariation = parameters_.getParameter("CalcVariation");
    surfaceInfo.usesMinMaxDistance = parameters_.getParameter("CalcMinMiaxDistance");
    surfaceInfo.usesSurface = parameters_.getParameter("CalcSurface");
    surfaceInfo.usesSurfaceX = parameters_.getParameter("CalcSurfaceX");
    surfaceInfo.usesSurfaceY = parameters_.getParameter("CalcSurfaceY");
    surfaceInfo.usesTexture = parameters_.getParameter("CalcTexture");
    surfaceInfo.usesStructure = parameters_.getParameter("CalcStructure");
}



SurfaceCalculator2Rows::SurfaceCalculator2Rows()
    :
    TransformFilter(m_oFilterName, Poco::UUID{"2550d408-8164-4838-80a1-f65ba779c189"}),
    m_pPipeInImageFrame(nullptr),
    m_pPipeInExtendedROIFrame(nullptr),
    m_oPipeOutSurfaceInfo(this, m_oPipeOutSurfaceInfoName),
    m_oTileWidth(30),
    m_oTileJumpX(500),
    m_oTileHeight(30),
    m_oTileJumpY(30),
    m_oInnerRow(50),
    m_oEnsureTile(false),
    m_lastImageTmp(Size2d(0,0)),
    m_lastTitleImageTmp("")
{
    m_oSurfaceInfo.reset();
    SurfaceCalculator2Rows::initializeFilterParameterContainer(m_oSurfaceInfo, parameters_);
    parameters_.add("TileWidth", Parameter::TYPE_int, m_oTileWidth);
    parameters_.add("TileHeight", Parameter::TYPE_int, m_oTileHeight);
    parameters_.add("TileJumpY", Parameter::TYPE_int, m_oTileJumpY);
    parameters_.add("InnerRow", Parameter::TYPE_int, m_oInnerRow);
    parameters_.add("EnsureTile", Parameter::TYPE_bool, m_oEnsureTile);

    setInPipeConnectors({  {Poco::UUID("a270f2e6-f0bb-4728-8da2-8d630b3761bd"), m_pPipeInImageFrame, "image", 1, "image"},
                           {Poco::UUID("d3388bbc-98d1-44c1-bd43-9aed6467fc43"), m_pPipeInExtendedROIFrame, "extendedROI", 1, "extendedROI"}
                        });
    setOutPipeConnectors({ {Poco::UUID("b89d9953-8be5-4626-a445-0fecdeab94b7"), &m_oPipeOutSurfaceInfo, m_oPipeOutSurfaceInfoName, 0, ""} });
    setVariantID(Poco::UUID("7368e579-91c6-40b9-8b1b-7dc650d5d2e7"));
}



void SurfaceCalculator2Rows::setParameter()
{

    TransformFilter::setParameter();

    m_oTileWidth = parameters_.getParameter("TileWidth");
    m_oTileHeight = parameters_.getParameter("TileHeight");
    m_oTileJumpY = parameters_.getParameter("TileJumpY");
    m_oInnerRow = parameters_.getParameter("InnerRow");
    m_oEnsureTile = parameters_.getParameter("EnsureTile");

    updateFromParameterContainer(m_oSurfaceInfo, parameters_);

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

    if (m_oInnerRow <= 0)
    {
        wmLog(eWarning, "Invalid value for Parameter InnerRow (%d), set to 1 \n", m_oInnerRow);
        m_oInnerRow = 1;
    }

    if (m_oInnerRow >= 100)
    {
        wmLog(eWarning, "Invalid value for Parameter InnerRow (%d), set to 99 \n", m_oInnerRow);
        m_oInnerRow = 99;
    }

} // setParameter.



bool SurfaceCalculator2Rows::subscribe(BasePipe& p_rPipe, int p_oGroup)
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



void SurfaceCalculator2Rows::computeTiles ( const SurfaceInfo   & rSurfaceInfo,
                                            TileContainer       & tileContainer,
                                            const BImage        & rImageIn,
                                            BImage              & r_lastImageTmp,
                                            std::string         & r_lastTitleImageTmp )
{
    int oNumberOfTilesX = tileContainer.m_sizeX;
    int oNumberOfTilesY = tileContainer.m_sizeY;

    auto fGetTileImage = [&rImageIn] (const SingleTile & tile)
    {
        const Rect oTileRoi(tile.m_startX, tile.m_startY, tile.m_width, tile.m_height);
        BImage oTileImgIn(rImageIn, oTileRoi, true);

        assert(    ( oTileImgIn.width()  == tile.m_width )
                && ( oTileImgIn.height() == tile.m_height )
              );

        return oTileImgIn;
    };

    auto meanValueSum = 0.0;

    if ( rSurfaceInfo.usesMean || rSurfaceInfo.usesRelBrightness )
    {
        for ( auto j = 0; j < oNumberOfTilesY; j++ )
        {
            for ( auto i = 0; i < oNumberOfTilesX; i++ )
            {
                SingleTile tile = tileContainer.getSingleTile(i, j);

                const BImage oTileImgIn = fGetTileImage(tile);

                const auto meanValue = calcMeanIntensity(oTileImgIn);
                meanValueSum += meanValue;

                tile.m_MeanValue = meanValue;
                tile.m_isMeanValid = rSurfaceInfo.usesMean;

                // Save tile at the end
                tileContainer.putSingleTile(i, j, tile);
            }
        }
    }

    auto totalMean = meanValueSum / ((double) (oNumberOfTilesX * oNumberOfTilesY));

    if ( std::abs(totalMean) < 0.00001 )
    {
        totalMean = 1;
    }

    for ( auto j = 0; j < oNumberOfTilesY; j++ )
    {
        for ( auto i = 0; i < oNumberOfTilesX; i++ )
        {
            SingleTile tile = tileContainer.getSingleTile(i, j);

            if ( rSurfaceInfo.usesRelBrightness )
            {
                tile.setRelIntensity(100.0 * (tile.m_MeanValue / totalMean));
                tile.m_isRelIntensityValid = true;
            }

            // Now all the other calculations

            const BImage oTileImgIn = fGetTileImage(tile);

            if ( rSurfaceInfo.usesVariation )
            {
                const auto variation = calcVariance(oTileImgIn);

                tile.m_Variation = variation;
                tile.m_isVariationValid = true;
            }

            if ( rSurfaceInfo.usesMinMaxDistance )
            {
                const auto minMaxDistance = calcMinMaxDistanceDeleteHighLow(oTileImgIn, 1);

                tile.m_MinMaxDistance = minMaxDistance;
                tile.m_isMinMaxDistanceValid = true;
            }

            if ( rSurfaceInfo.usesSurface || rSurfaceInfo.usesSurfaceX )
            {
                const auto surfaceX = calcGradientSumX(oTileImgIn);

                tile.m_SurfaceX = surfaceX;
                tile.m_isSurfaceXValid = rSurfaceInfo.usesSurfaceX;
            }

            if ( rSurfaceInfo.usesSurface || rSurfaceInfo.usesSurfaceY )
            {
                const auto surfaceY = calcGradientSumY(oTileImgIn);

                tile.m_SurfaceY = surfaceY;
                tile.m_isSurfaceYValid = rSurfaceInfo.usesSurfaceY;
            }

            if ( rSurfaceInfo.usesSurface )
            {
                const auto surface = 0.5 * (tile.m_SurfaceX + tile.m_SurfaceY);

                tile.m_Surface = surface;
                tile.m_isSurfaceValid = true;
            }

            if ( rSurfaceInfo.usesStructure )
            {
                const auto structure = calcStructure(oTileImgIn, r_lastImageTmp);
                r_lastTitleImageTmp = "Structure";
                tile.m_Structure = structure;
                tile.m_isStructureValid = true;
            }

            if ( rSurfaceInfo.usesTexture )
            {
                const auto texture = calcTexture(oTileImgIn, r_lastImageTmp);
                r_lastTitleImageTmp = "Texture";
                tile.m_Texture = texture;
                tile.m_isTextureValid = true;
            }

            // Tile back into list
            tileContainer.putSingleTile(i, j, tile);
        }
    }
};



void SurfaceCalculator2Rows::paintTiles   ( OverlayCanvas         & rOverlayCanvas,
                                            const TileContainer   & r_tileContainer,
                                            const Trafo           & rTrafo,
                                            const BImage          & r_lastImageTmp,
                                            const std::string     & r_lastTitleImageTmp,
                                            bool                  paintLastImage )
{

    const int oNumberOfTilesX = r_tileContainer.m_sizeX;
    const int oNumberOfTilesY = r_tileContainer.m_sizeY;

    OverlayLayer & rLayerLine  = rOverlayCanvas.getLayerLine();
    OverlayLayer & rLayerImage = rOverlayCanvas.getLayerImage();

    for ( auto j = 0; j < oNumberOfTilesY; j++ )
    {
        for ( auto i = 0; i < oNumberOfTilesX; i++ )
        {
            const SingleTile &tile = r_tileContainer.getSingleTile(i, j);
            const Rect oTileRoi(tile.m_startX, tile.m_startY, tile.m_width, tile.m_height);
            rLayerLine.add(new OverlayRectangle(rTrafo(oTileRoi), Color::m_oAluminium2));
        }
    }

    if ( r_lastImageTmp.isValid() && paintLastImage )
    {
        // Print a green "number" of the tile in top left tile corner

        SingleTile tile = r_tileContainer.getSingleTile(oNumberOfTilesX - 1, oNumberOfTilesY - 1);

        const auto oPosition = rTrafo(Point(tile.m_startX, tile.m_startY));
        const auto oTitle = OverlayText(r_lastTitleImageTmp, Font(), Rect(150, 18), Color::Green());

        rLayerImage.add(new OverlayImage(oPosition, r_lastImageTmp, oTitle));
    }
}



SmpTrafo SurfaceCalculator2Rows::updateNewROI(const Size2d minOutSize, const SmpTrafo& roiInTrafo, const Size2d& roiInSize, const SmpTrafo& sourceImageTrafo, const BImage& sourceImage)
{
    // Compute the ROI size
    int width  = std::max(minOutSize.width, roiInSize.width);
    int height = std::max(minOutSize.height, roiInSize.height);

    // Compute the ROI borders in context(0,0)
    const geo2d::Point imageCenter = roiInTrafo->apply(geo2d::Point(roiInSize.width / 2, roiInSize.height / 2));
    const int x_0 = imageCenter.x - width / 2;
    const int y_0 = imageCenter.y - height / 2;

    // Compute the ROI border in context of the extended ROI
    geo2d::Point offset = sourceImageTrafo->apply(geo2d::Point{0, 0});
    SmpTrafo oNewTrafo{new LinearTrafo{std::max(x_0, sourceImageTrafo->dx()), std::max(y_0, sourceImageTrafo->dy())}};
    assert(oNewTrafo->dx() >= sourceImageTrafo->dx());
    assert(oNewTrafo->dy() >= sourceImageTrafo->dy());

    const int x_ext = oNewTrafo->dx() - sourceImageTrafo->dx();
    const int y_ext = oNewTrafo->dy() - sourceImageTrafo->dy();

    // Clip ROI borders if necessary
    width = std::min(width, sourceImage.width() - x_ext);
    height = std::min(height, sourceImage.height() - y_ext);

    m_oNewROI = sourceImage.isValid()
                ? BImage(sourceImage, Rect{x_ext, y_ext, width, height}, true)
                : sourceImage;

    return oNewTrafo;
}



void SurfaceCalculator2Rows::proceedGroup(const void* p_pSender, PipeGroupEventArgs& p_rEvent)
{
    m_hasPainting = false;
    poco_assert_dbg(m_pPipeInImageFrame != nullptr);
    poco_assert_dbg(m_pPipeInExtendedROIFrame != nullptr);

    m_badInput = false;

    geo2d::SurfaceInfoarray oOutInfo;

    // Compute the input image and update trafo
    bool inputROIfitsImage = true;
    interface::ResultType oAnalysisResult;
    interface::ImageContext oContext;

    const BImage & rImageIn = [&oAnalysisResult, &oContext, &inputROIfitsImage, this]
                (int oTileWidth, int oTileHeight, const ImageFrame& rFrameROIIn, const ImageFrame& rFrameExtendedROIIn) -> const BImage &
    {

        // Check input pipes
        const BImage & rROIIn(rFrameROIIn.data());
        const BImage & rExtendedROIIn(rFrameExtendedROIIn.data());

        oAnalysisResult = std::max(rFrameROIIn.analysisResult(), rFrameExtendedROIIn.analysisResult());

        auto oTrafoROI = rFrameROIIn.context().trafo();
        auto oTrafoExtendedROI = rFrameExtendedROIIn.context().trafo();

        if ( !rROIIn.isValid() )
        {
            // Input ROI is not valid,  but we can use the trafo information to build a new ROI with the same center
            inputROIfitsImage = false;

            SmpTrafo oNewTrafo = updateNewROI( Size2D{oTileWidth, oTileHeight},
                                               oTrafoROI, rROIIn.size(),
                                               oTrafoExtendedROI, rExtendedROIIn );
            oContext = ImageContext(rFrameExtendedROIIn.context(), oNewTrafo);
            return m_oNewROI;
        }

        const auto oImageWidth = rROIIn.width();
        const auto oImageHeight = rROIIn.height();
        const auto oExtendedROIWidth = rExtendedROIIn.width();
        const auto oExtendedROIHeight = rExtendedROIIn.height();

        const auto ROIStart_InExtendedROI = oTrafoExtendedROI->applyReverse(oTrafoROI->apply(geo2d::Point{0, 0}));

        // ROIs are shallow copies of images, we can check if the ROI belong to the same image by checking the pixel address
        if (    ROIStart_InExtendedROI.x >= oExtendedROIWidth
             || ROIStart_InExtendedROI.y >= oExtendedROIHeight
             || rROIIn.begin() != rExtendedROIIn.rowBegin(ROIStart_InExtendedROI.y) + ROIStart_InExtendedROI.x
           )
        {
            wmLog(eWarning, "ROI is not part of extended ROI \n");
            inputROIfitsImage = false;
            oContext = rFrameROIIn.context();
            return rROIIn;
        }

        if ( oTileWidth <= oImageWidth && oTileHeight <= oImageHeight )
        {
            // Input image is valid, no need to check extended ROI
            inputROIfitsImage = true;
            oContext = rFrameROIIn.context();
            return rROIIn;
        }

        inputROIfitsImage = false;

        if (    oTrafoExtendedROI->dx() > oTrafoROI->dx()
             || oTrafoExtendedROI->dy() > oTrafoROI->dy()
             || oTrafoExtendedROI->dx() + oExtendedROIWidth  < oTrafoROI->dx() + oImageWidth
             || oTrafoExtendedROI->dy() + oExtendedROIHeight < oTrafoROI->dy() + oImageHeight
           )
        {
            wmLog(eWarning, "Extended ROI (context %d, %d size %d x %d) is not compatible with input ROI (context %d, %d size %d x %d) \n",
                            oTrafoExtendedROI->dx(), oTrafoExtendedROI->dy(), oExtendedROIWidth, oExtendedROIHeight,
                            oTrafoROI->dx(), oTrafoROI->dy(), oImageWidth, oImageHeight );

            // We can't create a new ROI, just return the current one
            oContext = rFrameROIIn.context();
            return rROIIn;
        }

        // Build a new ROI centered on the first
        SmpTrafo oNewTrafo = updateNewROI( Size2D{oTileWidth, oTileHeight},
                                           oTrafoROI, rROIIn.size(),
                                           oTrafoExtendedROI, rExtendedROIIn );
        oContext = ImageContext(rFrameExtendedROIIn.context(), oNewTrafo);
        return m_oNewROI;
    }
    ( m_oTileWidth, m_oTileHeight, m_pPipeInImageFrame->read(m_oCounter), m_pPipeInExtendedROIFrame->read(m_oCounter) );

    m_oSpTrafo = oContext.trafo();

    m_oSurfaceInfo.setTileContainer(TileContainer{});
    const int oImageWidth = rImageIn.width();
    const int oImageHeight = rImageIn.height();
    int oActualTileWidth = m_oTileWidth;
    int oActualTileHeight = m_oTileHeight;

    //compute the number of tiles

    int oNumberOfTilesX = 0;
    int oNumberOfTilesY = 0;

    // Calculations for tiles in x direction (number / width / offsetX)
    {
        // Is tile inside ROI?
        if ( oActualTileWidth > oImageWidth ) // Tile width is too large
        {
            if ( m_oEnsureTile && oImageWidth > 0 )
            {
                oActualTileWidth = oImageWidth;
                oNumberOfTilesX = 2;   // 'row' and 'percentage row'
                inputROIfitsImage = false;
            }
            else
            {
                oNumberOfTilesX = 0;
            }
        }
        else
        {
            oNumberOfTilesX = 2;   // 'row' and 'percentage row'
        }

        // Calculate width for second row, put value in variable 'm_oTileJumpX'
        m_oTileJumpX = (int) (m_oInnerRow * oActualTileWidth / 100.0);
        if (m_oTileJumpX <= 0)
        {
            wmLog(eWarning, "Invalid internal value for %% value of TileWidth (%d), set to 1 \n", m_oTileJumpX);
            m_oTileJumpX = 1;
        }
    }

    // Calculation of tiles in y direction (number and height)
    {
        const int boxGapY = m_oTileJumpY - m_oTileHeight;

        if ( oActualTileHeight > oImageHeight )    // box height is too large for one tile
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
        else if ( 2 * oActualTileHeight + boxGapY > oImageHeight )   // 2 boxes doesn't fit in image height
        {
            oNumberOfTilesY = 1;
        }
        else
        {
            oNumberOfTilesY = 1 + ( (oImageHeight - oActualTileHeight) / ( (double) m_oTileJumpY) );
        }
    }

    const int oNumberOfAllTiles = oNumberOfTilesX * oNumberOfTilesY;  // Total number of tiles

    if (oNumberOfAllTiles == 0)    // no calcualtions possible => quickly close and leave
    {
        m_badInput = true;
        assert(m_oSurfaceInfo.getTileContainer().empty());
        oOutInfo.getData().push_back(m_oSurfaceInfo);
        oOutInfo.getRank().push_back(0);
    }
    else
    {
        m_hasPainting = true;

        // Calc space for tiles in x
        int oTileOffsetX = (int) ((oImageWidth - oActualTileWidth) / 2.0);  // Offset for empty space left and right => tiles are in center

        // Calc space for tiles in y
        int oTilesSpaceY = m_oTileJumpY * (oNumberOfTilesY - 1) + oActualTileHeight;  // here is oNumberOfTilesY > 0! Otherwise product above is zero
        int oTileOffsetY = (int) ((oImageHeight - oTilesSpaceY) / 2.0);  // Offset for empty space left and right => tiles are in center

        // Tile container
        TileContainer tileContainer( oNumberOfTilesX, oNumberOfTilesY,
                                     oActualTileWidth, oActualTileHeight,
                                     oTileOffsetX, oTileOffsetY,
                                     m_oTileJumpX, m_oTileJumpY,   // 'm_oTileJumpX' is width of 'percentage row'
                                     true  /* 'TwoRows' */ );

        SurfaceCalculator2Rows::computeTiles(m_oSurfaceInfo, tileContainer, rImageIn, m_lastImageTmp, m_lastTitleImageTmp);

        m_oSurfaceInfo.setTileContainer(tileContainer);

        oOutInfo.getData().push_back(m_oSurfaceInfo);
        oOutInfo.getRank().push_back(255);

    }

    double dRank = (m_badInput) ? 0.0 : 1.0;

    const GeoSurfaceInfoarray oGeoInfo = GeoSurfaceInfoarray(oContext, oOutInfo, oAnalysisResult, dRank);

    preSignalAction();
    m_oPipeOutSurfaceInfo.signal(oGeoInfo);   // invoke linked filter(s)
} // proceed



void SurfaceCalculator2Rows::paint()
{
    if (!m_hasPainting)
    {
        return;
    }

    m_hasPainting = false;

    if (    m_oVerbosity < eLow
         || m_oSpTrafo.isNull()
       )
    {
        return;
    }

    try
    {
        const Trafo   & rTrafo(*m_oSpTrafo);
        OverlayCanvas & rOverlayCanvas = canvas<OverlayCanvas>(m_oCounter);

        // m_lastImageTmp is continuously overwritten, so it corresponds to the last tile
        SurfaceCalculator2Rows::paintTiles(rOverlayCanvas, m_oSurfaceInfo.getTileContainer(), rTrafo, m_lastImageTmp, m_lastTitleImageTmp, m_oVerbosity >= eHigh);
    }
    catch (...)
    {
        // Paint has problems => only logging
        wmLog(eWarning, "Exception during painting in SurfaceCalculator2Rows\n");
    }

} // paint



} // namespace filter
} // namespace precitec
