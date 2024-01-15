/**
* 	@file
* 	@copyright	Precitec Vision GmbH & Co. KG
* 	@author		OS
* 	@date		2017
* 	@brief		Filter 'SurfaceCalculator'. Divides an image into tiles. The tiles may overlap. For each tile some surface texture features are calculated.
*/

// WM includes

#include "surfaceCalculator.h"

#include "fliplib/Parameter.h"
#include "overlay/overlayPrimitive.h"
#include "image/image.h"
#include "module/moduleLogger.h"
#include "filter/algoImage.h"
#include "filter/algoStl.h"
#include "system/typeTraits.h"
#include <fliplib/TypeToDataTypeImpl.h>

// stdlib includes

#include <algorithm>

using namespace fliplib;
namespace precitec {
	using namespace geo2d;
	using namespace interface;
	using namespace image;
	namespace filter
{

const std::string SurfaceCalculator::m_oFilterName("SurfaceCalculator");		///< Filter name
const std::string SurfaceCalculator::m_oPipeOutSurfaceInfoName("SurfaceInfo");	///< out-pipe for surface information

/*static*/ void SurfaceCalculator::initializeFilterParameterContainer(SurfaceInfo & surfaceInfo, fliplib::ParameterContainer & parameters_)
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

/*static*/ void SurfaceCalculator::updateFromParameterContainer(SurfaceInfo & surfaceInfo, fliplib::ParameterContainer & parameters_)
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


SurfaceCalculator::SurfaceCalculator()
	:
	TransformFilter(m_oFilterName, Poco::UUID{"77f911f4-e5bc-4b2a-b38f-e9431bfec907"}),
	m_pPipeInImageFrame(nullptr),
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
    initializeFilterParameterContainer(m_oSurfaceInfo, parameters_);
	parameters_.add("TileWidth", Parameter::TYPE_int, m_oTileWidth);
	parameters_.add("TileJumpX", Parameter::TYPE_int, m_oTileJumpX);
	parameters_.add("TileHeight", Parameter::TYPE_int, m_oTileHeight);
	parameters_.add("TileJumpY", Parameter::TYPE_int, m_oTileJumpY);
    parameters_.add("EnsureTile", Parameter::TYPE_bool, m_oEnsureTile);

    setInPipeConnectors({{Poco::UUID("9DFFEE32-6C5F-4F65-9612-FE20EBBCC8A5"), m_pPipeInImageFrame, "ImageFrame", 0, ""}});
    setOutPipeConnectors({{Poco::UUID("FEC3CF12-BBE6-4F6C-A6C1-F36984175FC8"), &m_oPipeOutSurfaceInfo, "SurfaceInfo", 0, ""}});
    setVariantID(Poco::UUID("5bb8f4b6-2616-41e9-ae1e-cbb27d0f2056"));
}

void SurfaceCalculator::setParameter()
{

	TransformFilter::setParameter();

	m_oTileWidth = parameters_.getParameter("TileWidth");
	m_oTileJumpX = parameters_.getParameter("TileJumpX");
	m_oTileHeight = parameters_.getParameter("TileHeight");
	m_oTileJumpY = parameters_.getParameter("TileJumpY");
    m_oEnsureTile = parameters_.getParameter("EnsureTile");
    updateFromParameterContainer(m_oSurfaceInfo, parameters_);

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

bool SurfaceCalculator::subscribe(BasePipe& p_rPipe, int p_oGroup)
{
	m_pPipeInImageFrame = dynamic_cast<const image_pipe_t*>(&p_rPipe);

	return BaseFilter::subscribe(p_rPipe, p_oGroup);
} // subscribe



/*static*/ void SurfaceCalculator::computeTiles(const SurfaceInfo & rSurfaceInfo,
    TileContainer & tileContainer,
    const BImage & rImageIn,
    BImage & r_lastImageTmp, std::string & r_lastTitleImageTmp)
{
    int oNumberOfTilesX = tileContainer.m_sizeX;
    int oNumberOfTilesY = tileContainer.m_sizeY;

    auto fGetTileImage = [&rImageIn] (const SingleTile & tile)
    {
        const Rect oTileRoi(tile.m_startX, tile.m_startY, tile.m_width, tile.m_height);
        BImage oTileImgIn(rImageIn, oTileRoi, true);
        assert(oTileImgIn.width() == tile.m_width && oTileImgIn.height() == tile.m_height);
        return oTileImgIn;
    };

    double meanValueSum = 0.0;

    if ( rSurfaceInfo.usesMean || rSurfaceInfo.usesRelBrightness )
    {
        for ( int j = 0; j < oNumberOfTilesY; j++ )
        {
            for ( int i = 0; i < oNumberOfTilesX; i++ )
            {
                SingleTile tile = tileContainer.getSingleTile(i, j);

                const BImage oTileImgIn = fGetTileImage(tile);

                double meanValue = calcMeanIntensity(oTileImgIn);
                meanValueSum += meanValue;

                tile.m_MeanValue = meanValue;
                tile.m_isMeanValid = rSurfaceInfo.usesMean;

                // am Ende Tile ablegen
                tileContainer.putSingleTile(i, j, tile);
            }
        }
    }

    double totalMean = meanValueSum / ((double) (oNumberOfTilesX * oNumberOfTilesY));
    if ( std::abs(totalMean) < 0.00001 ) totalMean = 1;

    for ( int j = 0; j < oNumberOfTilesY; j++ )
    {
        for ( int i = 0; i < oNumberOfTilesX; i++ )
        {
            SingleTile tile = tileContainer.getSingleTile(i, j);

            if ( rSurfaceInfo.usesRelBrightness )
            {
                tile.setRelIntensity(100.0 * (tile.m_MeanValue / totalMean));
                tile.m_isRelIntensityValid = true;
            }

            // jetzt alle anderen

            const BImage oTileImgIn = fGetTileImage(tile);

            if ( rSurfaceInfo.usesVariation )
            {
                double variation = calcVariance(oTileImgIn);

                tile.m_Variation = variation;
                tile.m_isVariationValid = true;
            }

            if ( rSurfaceInfo.usesMinMaxDistance )
            {
                double minMaxDistance = calcMinMaxDistanceDeleteHighLow(oTileImgIn, 1);

                tile.m_MinMaxDistance = minMaxDistance;
                tile.m_isMinMaxDistanceValid = true;
            }

            if ( rSurfaceInfo.usesSurface || rSurfaceInfo.usesSurfaceX )
            {
                double surfaceX = calcGradientSumX(oTileImgIn);

                tile.m_SurfaceX = surfaceX;
                tile.m_isSurfaceXValid = rSurfaceInfo.usesSurfaceX;
            }

            if ( rSurfaceInfo.usesSurface || rSurfaceInfo.usesSurfaceY )
            {
                double surfaceY = calcGradientSumY(oTileImgIn);

                tile.m_SurfaceY = surfaceY;
                tile.m_isSurfaceYValid = rSurfaceInfo.usesSurfaceY;
            }

            if ( rSurfaceInfo.usesSurface )
            {
                double surface = 0.5 * (tile.m_SurfaceX + tile.m_SurfaceY);

                tile.m_Surface = surface;
                tile.m_isSurfaceValid = true;
            }

            if ( rSurfaceInfo.usesStructure )
            {
                double structure = calcStructure(oTileImgIn, r_lastImageTmp);
                r_lastTitleImageTmp = "Structure";
                tile.m_Structure = structure;
                tile.m_isStructureValid = true;
            }

            if ( rSurfaceInfo.usesTexture )
            {
                double texture = calcTexture(oTileImgIn, r_lastImageTmp);
                r_lastTitleImageTmp = "Texture";
                tile.m_Texture = texture;
                tile.m_isTextureValid = true;
            }

            // Tile wieder zurueck in die Liste
            tileContainer.putSingleTile(i, j, tile);
        }
    }

};


/*static */ void SurfaceCalculator::paintTiles(OverlayCanvas&			rOverlayCanvas, const TileContainer & r_tileContainer, const Trafo		&rTrafo,
    const BImage & r_lastImageTmp, const std::string & r_lastTitleImageTmp, bool paintLastImage)
{

    int oNumberOfTilesX = r_tileContainer.m_sizeX;
    int oNumberOfTilesY = r_tileContainer.m_sizeY;

    OverlayLayer&			rLayerLine = rOverlayCanvas.getLayerLine();
    OverlayLayer&			rLayerImage = rOverlayCanvas.getLayerImage();

    for ( int j = 0; j < oNumberOfTilesY; j++ )
    {
        for ( int i = 0; i < oNumberOfTilesX; i++ )
        {
            const SingleTile &tile = r_tileContainer.getSingleTile(i, j);
            const Rect oTileRoi(tile.m_startX, tile.m_startY, tile.m_width, tile.m_height);
            rLayerLine.add(new OverlayRectangle(rTrafo(oTileRoi), Color::m_oAluminium2));
        }
    }

    if ( r_lastImageTmp.isValid() && paintLastImage)
    {
        SingleTile tile = r_tileContainer.getSingleTile(oNumberOfTilesX - 1, oNumberOfTilesY - 1);
        const auto	oPosition = rTrafo(Point(tile.m_startX, tile.m_startY));
        const auto	oTitle = OverlayText(r_lastTitleImageTmp, Font(), Rect(150, 18), Color::Green());
        rLayerImage.add(new OverlayImage(oPosition, r_lastImageTmp, oTitle));
    }
}

void SurfaceCalculator::proceed(const void* p_pSender, PipeEventArgs& p_rEvent)
{
	m_hasPainting = false;
	poco_assert_dbg(m_pPipeInImageFrame != nullptr);
	m_badInput = false;

	geo2d::SurfaceInfoarray oOutInfo;

	const ImageFrame&		rFrameIn(m_pPipeInImageFrame->read(m_oCounter));
	const BImage&			rImageIn(rFrameIn.data());

	m_oSpTrafo = rFrameIn.context().trafo();

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


        computeTiles(m_oSurfaceInfo, tileContainer, rImageIn, m_lastImageTmp, m_lastTitleImageTmp);

        m_oSurfaceInfo.setTileContainer(tileContainer);

        oOutInfo.getData().push_back(m_oSurfaceInfo);
		oOutInfo.getRank().push_back(255);

	}
	double dRank = (m_badInput) ? 0.0 : 1.0;

	const auto oAnalysisResult = rFrameIn.analysisResult() == AnalysisOK ? AnalysisOK : rFrameIn.analysisResult(); // replace 2nd AnalysisOK by your result type

	const GeoSurfaceInfoarray &rInfo = GeoSurfaceInfoarray(rFrameIn.context(), oOutInfo, oAnalysisResult, dRank);

	preSignalAction();
	m_oPipeOutSurfaceInfo.signal(rInfo);			// invoke linked filter(s)
} // proceed


void SurfaceCalculator::paint()
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

        //m_lastImageTmp is continuely overwritten, so it corresponds to the last tile
        paintTiles(rOverlayCanvas, m_oSurfaceInfo.getTileContainer(), rTrafo, m_lastImageTmp, m_lastTitleImageTmp, m_oVerbosity >= eHigh);


	}
	catch (...)
	{
		// Paint ging schief => nur loggen
		wmLog(eWarning, "Exception during painting in surfaceCalculator");
	}

} // paint



} // namespace filter
} // namespace precitec
