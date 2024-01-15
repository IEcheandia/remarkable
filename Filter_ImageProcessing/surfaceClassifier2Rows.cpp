/**
*   @file
*   @copyright  Precitec Vision GmbH & Co. KG
*   @author     GUR
*   @date       2022
*   @brief      Filter 'SurfaceClassifier2Rows', copied from filter 'SurfaceClassifier'.
*               Gets a SurfaceInfo structure with two (overlying) rows of tiles and checks the given limits.
*/

// WM includes

#include "surfaceClassifier2Rows.h"

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
namespace precitec
{
using namespace geo2d;
using namespace interface;
using namespace image;

namespace filter
{

const std::string SurfaceClassifier2Rows::m_oFilterName("SurfaceClassifier2Rows");      ///< Filter name.
const std::string SurfaceClassifier2Rows::m_oPipeOutSizeConnected1Name("SizeConnected1"); ///< Pipe: Scalar out-pipe.
const std::string SurfaceClassifier2Rows::m_oPipeOutSizeConnected2Name("SizeConnected2"); ///< Pipe: Scalar out-pipe.


SurfaceClassifier2Rows::SurfaceClassifier2Rows()
    : TransformFilter(m_oFilterName, Poco::UUID{"22b8231a-d824-46f8-b03e-22516d719e83"})
    , m_pPipeInSurfaceInfo(nullptr)
    , m_oPipeOutSizeConnected1(this, m_oPipeOutSizeConnected1Name)
    , m_oPipeOutSizeConnected2(this, m_oPipeOutSizeConnected2Name)

    , m_oDisplay(0)

    , m_oMinMean(0)
    , m_oMaxMean(255)
    , m_oMinRelInt(0)
    , m_oMaxRelInt(1000)
    , m_oMinVariation(0)
    , m_oMaxVariation(1000)
    , m_oMinMinMaxDistance(0)
    , m_oMaxMinMaxDistance(255)
    , m_oMinSurface(0)
    , m_oMaxSurface(100)
    , m_oMinSurfaceX(0)
    , m_oMaxSurfaceX(100)
    , m_oMinSurfaceY(0)
    , m_oMaxSurfaceY(100)
    , m_oMinTexture(0)
    , m_oMaxTexture(100)
    , m_oMinStructure(0)
    , m_oMaxStructure(100)

    , m_oMinMean2(0)
    , m_oMaxMean2(255)
    , m_oMinRelInt2(0)
    , m_oMaxRelInt2(1000)
    , m_oMinVariation2(0)
    , m_oMaxVariation2(1000)
    , m_oMinMinMaxDistance2(0)
    , m_oMaxMinMaxDistance2(255)
    , m_oMinSurface2(0)
    , m_oMaxSurface2(100)
    , m_oMinSurfaceX2(0)
    , m_oMaxSurfaceX2(100)
    , m_oMinSurfaceY2(0)
    , m_oMaxSurfaceY2(100)
    , m_oMinTexture2(0)
    , m_oMaxTexture2(100)
    , m_oMinStructure2(0)
    , m_oMaxStructure2(100)
{
    // Set default values for the parameters of the filter
    parameters_.add("Display", Parameter::TYPE_int, m_oDisplay);

    parameters_.add("MinMean", Parameter::TYPE_double, m_oMinMean);
    parameters_.add("MaxMean", Parameter::TYPE_double, m_oMaxMean);
    parameters_.add("MinRelInt", Parameter::TYPE_double, m_oMinRelInt);
    parameters_.add("MaxRelInt", Parameter::TYPE_double, m_oMaxRelInt);
    parameters_.add("MinVariation", Parameter::TYPE_double, m_oMinVariation);
    parameters_.add("MaxVariation", Parameter::TYPE_double, m_oMaxVariation);
    parameters_.add("MinMinMaxDistance", Parameter::TYPE_double, m_oMinMinMaxDistance);
    parameters_.add("MaxMinMaxDistance", Parameter::TYPE_double, m_oMaxMinMaxDistance);
    parameters_.add("MinSurface", Parameter::TYPE_double, m_oMinSurface);
    parameters_.add("MaxSurface", Parameter::TYPE_double, m_oMaxSurface);
    parameters_.add("MinSurfaceX", Parameter::TYPE_double, m_oMinSurfaceX);
    parameters_.add("MaxSurfaceX", Parameter::TYPE_double, m_oMaxSurfaceX);
    parameters_.add("MinSurfaceY", Parameter::TYPE_double, m_oMinSurfaceY);
    parameters_.add("MaxSurfaceY", Parameter::TYPE_double, m_oMaxSurfaceY);
    parameters_.add("MinTexture", Parameter::TYPE_double, m_oMinTexture);
    parameters_.add("MaxTexture", Parameter::TYPE_double, m_oMaxTexture);
    parameters_.add("MinStructure", Parameter::TYPE_double, m_oMinStructure);
    parameters_.add("MaxStructure", Parameter::TYPE_double, m_oMaxStructure);

    parameters_.add("MinMean2", Parameter::TYPE_double, m_oMinMean2);
    parameters_.add("MaxMean2", Parameter::TYPE_double, m_oMaxMean2);
    parameters_.add("MinRelInt2", Parameter::TYPE_double, m_oMinRelInt2);
    parameters_.add("MaxRelInt2", Parameter::TYPE_double, m_oMaxRelInt2);
    parameters_.add("MinVariation2", Parameter::TYPE_double, m_oMinVariation2);
    parameters_.add("MaxVariation2", Parameter::TYPE_double, m_oMaxVariation2);
    parameters_.add("MinMinMaxDistance2", Parameter::TYPE_double, m_oMinMinMaxDistance2);
    parameters_.add("MaxMinMaxDistance2", Parameter::TYPE_double, m_oMaxMinMaxDistance2);
    parameters_.add("MinSurface2", Parameter::TYPE_double, m_oMinSurface2);
    parameters_.add("MaxSurface2", Parameter::TYPE_double, m_oMaxSurface2);
    parameters_.add("MinSurfaceX2", Parameter::TYPE_double, m_oMinSurfaceX2);
    parameters_.add("MaxSurfaceX2", Parameter::TYPE_double, m_oMaxSurfaceX2);
    parameters_.add("MinSurfaceY2", Parameter::TYPE_double, m_oMinSurfaceY2);
    parameters_.add("MaxSurfaceY2", Parameter::TYPE_double, m_oMaxSurfaceY2);
    parameters_.add("MinTexture2", Parameter::TYPE_double, m_oMinTexture2);
    parameters_.add("MaxTexture2", Parameter::TYPE_double, m_oMaxTexture2);
    parameters_.add("MinStructure2", Parameter::TYPE_double, m_oMinStructure2);
    parameters_.add("MaxStructure2", Parameter::TYPE_double, m_oMaxStructure2);

    setInPipeConnectors({
        {Poco::UUID("a18a554c-af65-43ff-9d9a-eda1a41ad4a5"), m_pPipeInSurfaceInfo, "SurfaceInfo", 0, ""}});
    setOutPipeConnectors({
        {Poco::UUID("4abebd9b-8924-46ed-b9b9-b8e68f692ec0"), &m_oPipeOutSizeConnected1, m_oPipeOutSizeConnected1Name, 0, ""},
        {Poco::UUID("78cbde74-84db-41cc-ada9-3959f74758f5"), &m_oPipeOutSizeConnected2, m_oPipeOutSizeConnected2Name, 0, ""}});
    setVariantID(Poco::UUID("78a9d7b7-7c1c-445c-931d-05d68db468aa"));
}


void SurfaceClassifier2Rows::setParameter()
{
    using namespace std::placeholders;

    TransformFilter::setParameter();

    m_oDisplay            = parameters_.getParameter("Display");

    m_oMinMean            = parameters_.getParameter("MinMean");
    m_oMaxMean            = parameters_.getParameter("MaxMean");
    m_oMinRelInt          = parameters_.getParameter("MinRelInt");
    m_oMaxRelInt          = parameters_.getParameter("MaxRelInt");
    m_oMinVariation       = parameters_.getParameter("MinVariation");
    m_oMaxVariation       = parameters_.getParameter("MaxVariation");
    m_oMinMinMaxDistance  = parameters_.getParameter("MinMinMaxDistance");
    m_oMaxMinMaxDistance  = parameters_.getParameter("MaxMinMaxDistance");
    m_oMinSurface         = parameters_.getParameter("MinSurface");
    m_oMaxSurface         = parameters_.getParameter("MaxSurface");
    m_oMinSurfaceX        = parameters_.getParameter("MinSurfaceX");
    m_oMaxSurfaceX        = parameters_.getParameter("MaxSurfaceX");
    m_oMinSurfaceY        = parameters_.getParameter("MinSurfaceY");
    m_oMaxSurfaceY        = parameters_.getParameter("MaxSurfaceY");
    m_oMinTexture         = parameters_.getParameter("MinTexture");
    m_oMaxTexture         = parameters_.getParameter("MaxTexture");
    m_oMinStructure       = parameters_.getParameter("MinStructure");
    m_oMaxStructure       = parameters_.getParameter("MaxStructure");

    m_oMinMean2           = parameters_.getParameter("MinMean2");
    m_oMaxMean2           = parameters_.getParameter("MaxMean2");
    m_oMinRelInt2         = parameters_.getParameter("MinRelInt2");
    m_oMaxRelInt2         = parameters_.getParameter("MaxRelInt2");
    m_oMinVariation2      = parameters_.getParameter("MinVariation2");
    m_oMaxVariation2      = parameters_.getParameter("MaxVariation2");
    m_oMinMinMaxDistance2 = parameters_.getParameter("MinMinMaxDistance2");
    m_oMaxMinMaxDistance2 = parameters_.getParameter("MaxMinMaxDistance2");
    m_oMinSurface2        = parameters_.getParameter("MinSurface2");
    m_oMaxSurface2        = parameters_.getParameter("MaxSurface2");
    m_oMinSurfaceX2       = parameters_.getParameter("MinSurfaceX2");
    m_oMaxSurfaceX2       = parameters_.getParameter("MaxSurfaceX2");
    m_oMinSurfaceY2       = parameters_.getParameter("MinSurfaceY2");
    m_oMaxSurfaceY2       = parameters_.getParameter("MaxSurfaceY2");
    m_oMinTexture2        = parameters_.getParameter("MinTexture2");
    m_oMaxTexture2        = parameters_.getParameter("MaxTexture2");
    m_oMinStructure2      = parameters_.getParameter("MinStructure2");
    m_oMaxStructure2      = parameters_.getParameter("MaxStructure2");
} // setParameter


bool SurfaceClassifier2Rows::subscribe(BasePipe &p_rPipe, int p_oGroup)
{
    // has only one InPipe => 'SurfaceInfo' is checked
    m_pPipeInSurfaceInfo = dynamic_cast<const fliplib::SynchronePipe< interface::GeoSurfaceInfoarray >*>(&p_rPipe);

    return BaseFilter::subscribe(p_rPipe, p_oGroup);
}


void SurfaceClassifier2Rows::proceed(const void* p_pSender, PipeEventArgs &p_rEvent)
{
    poco_assert_dbg(m_pPipeInSurfaceInfo != nullptr);  // to be asserted by graph editor

    const GeoSurfaceInfoarray &rGeoSurfaceInfoIn(m_pPipeInSurfaceInfo->read(m_oCounter));
    geo2d::Doublearray        oOutSizeConnected1;
    geo2d::Doublearray        oOutSizeConnected2;

    m_allInfoLines.clear();
    std::vector<SurfaceInfoLine2Rows> infoLinesPerSurface;

    m_oSpTrafo = rGeoSurfaceInfoIn.context().trafo();
    m_hasPainting = false;

    unsigned int oSizeOfArray = rGeoSurfaceInfoIn.ref().size();

    if ( (oSizeOfArray <= 0) || (rGeoSurfaceInfoIn.isValid() == false) )  // Problems with incoming data?
    {
        oOutSizeConnected1.getData().push_back(0);
        oOutSizeConnected1.getRank().push_back(0);

        oOutSizeConnected2.getData().push_back(0);
        oOutSizeConnected2.getRank().push_back(0);

        const auto oSizeConnectedOut1 = GeoDoublearray(rGeoSurfaceInfoIn.context(), oOutSizeConnected1, rGeoSurfaceInfoIn.analysisResult(), 0.0);
        const auto oSizeConnectedOut2 = GeoDoublearray(rGeoSurfaceInfoIn.context(), oOutSizeConnected2, rGeoSurfaceInfoIn.analysisResult(), 0.0);

        preSignalAction();
        m_oPipeOutSizeConnected1.signal(oSizeConnectedOut1);
        m_oPipeOutSizeConnected2.signal(oSizeConnectedOut2);

        return;
    }

    m_oInInfo = std::get<eData>(rGeoSurfaceInfoIn.ref()[0]);

    // First row is the regular, second row is the inside 'percentage' row
    TileContainer tileContainer = m_oInInfo.getTileContainer();

    if  (tileContainer.m_sizeX != 2)
    {
        wmLog(eWarning, "Filter 'SurfaceClassifier2Rows': has %d rows (should be 2) !!\n", tileContainer.m_sizeX);
    }

    if (tileContainer.m_sizeY * tileContainer.m_sizeX > 0)
    {
        m_hasPainting = true;
    }

    image::Color col = Color::Black();
    int value;

    for (int j = 0; j < tileContainer.m_sizeY; j++)
    {
        for (int i = 0; i < tileContainer.m_sizeX; i++)
        {
            // If i = 0: it's the first row = the big tile
            // If i = 1: it's the second row = the inside 'percentage' row = the smaller tile
            // and there should be no more rows!

            SingleTile tile = tileContainer.getSingleTile(i, j);
            infoLinesPerSurface.clear();

            infoLinesPerSurface.push_back(SurfaceInfoLine2Rows(eSurfacePTileNo, tile.m_number, Color::Black()));

            col = Color::Black();
            value = 0;
            if (tile.m_isMeanValid)  // Mean value was calculated
            {
                value = (int) (0.5 + tile.m_MeanValue);
                if (m_oInInfo.usesMean)
                {
                    col = Color::Green();
                    if (    ( (i == 0) && (tile.m_MeanValue < m_oMinMean || tile.m_MeanValue > m_oMaxMean) )
                         || ( (i == 1) && (tile.m_MeanValue < m_oMinMean2 || tile.m_MeanValue > m_oMaxMean2) )
                       )
                    {
                        tile.setMeanDefect();
                        tile.setDefect();
                        col = Color::Red();
                    }
                }
            }
            infoLinesPerSurface.push_back(SurfaceInfoLine2Rows(eSurfacePMean, value, col));

            if (tile.m_isRelIntensityValid)  // Relative (percentage) value was calculated
            {
                col = Color::Black();
                value = 0;
                if (m_oInInfo.usesRelBrightness)
                {
                    value = (int) (0.5 + tile.m_relIntensity);
                    col = Color::Green();
                    if (    ( (i == 0) && (tile.m_relIntensity < m_oMinRelInt || tile.m_relIntensity > m_oMaxRelInt) )
                         || ( (i == 1) && (tile.m_relIntensity < m_oMinRelInt2 || tile.m_relIntensity > m_oMaxRelInt2) )
                       )
                    {
                        tile.setRelIntDefect();
                        tile.setDefect();
                        col = Color::Red();
                    }
                }
            }
            infoLinesPerSurface.push_back(SurfaceInfoLine2Rows(eSurfacePRelBrightness, value, col));

            col = Color::Black();
            value = 0;
            if (m_oInInfo.usesVariation)
            {
                value = (int) (0.5 + tile.m_Variation);
                col = Color::Green();
                if (    ( (i == 0) && (tile.m_Variation < m_oMinVariation || tile.m_Variation > m_oMaxVariation) )
                     || ( (i == 1) && (tile.m_Variation < m_oMinVariation2 || tile.m_Variation > m_oMaxVariation2) )
                   )
                {
                    tile.setVariationDefect();
                    tile.setDefect();
                    col = Color::Red();
                }
            }
            infoLinesPerSurface.push_back(SurfaceInfoLine2Rows(eSurfacePVariation, value, col));

            col = Color::Black();
            value = 0;
            if (m_oInInfo.usesMinMaxDistance)
            {
                value = (int) (0.5 + tile.m_MinMaxDistance);
                col = Color::Green();
                if (    ( (i == 0) && (tile.m_MinMaxDistance < m_oMinMinMaxDistance || tile.m_MinMaxDistance > m_oMaxMinMaxDistance) )
                     || ( (i == 1) && (tile.m_MinMaxDistance < m_oMinMinMaxDistance2 || tile.m_MinMaxDistance > m_oMaxMinMaxDistance2) )
                   )
                {
                    tile.setMinMaxDistanceDefect();
                    tile.setDefect();
                    col = Color::Red();
                }
            }
            infoLinesPerSurface.push_back(SurfaceInfoLine2Rows(eSurfacePMinMaxDist, value, col));

            col = Color::Black();
            value = 0;
            if (m_oInInfo.usesSurface)
            {
                value = (int) (0.5 + tile.m_Surface);
                col = Color::Green();
                if (    ( (i == 0) && (tile.m_Surface < m_oMinSurface || tile.m_Surface > m_oMaxSurface) )
                     || ( (i == 1) && (tile.m_Surface < m_oMinSurface2 || tile.m_Surface > m_oMaxSurface2) )
                   )
                {
                    tile.setSurfaceDefect();
                    tile.setDefect();
                    col = Color::Red();
                }
            }
            infoLinesPerSurface.push_back(SurfaceInfoLine2Rows(eSurfacePSurface, value, col));

            col = Color::Black();
            value = 0;
            if (m_oInInfo.usesSurfaceX)
            {
                value = (int) (0.5 + tile.m_SurfaceX);
                col = Color::Green();
                if (    ( (i == 0) && (tile.m_SurfaceX < m_oMinSurfaceX || tile.m_SurfaceX > m_oMaxSurfaceX) )
                     || ( (i == 1) && (tile.m_SurfaceX < m_oMinSurfaceX2 || tile.m_SurfaceX > m_oMaxSurfaceX2) )
                   )
                {
                    tile.setSurfaceXDefect();
                    tile.setDefect();
                    col = Color::Red();
                }
            }
            infoLinesPerSurface.push_back(SurfaceInfoLine2Rows(eSurfacePSurfaceX, value, col));

            col = Color::Black();
            value = 0;
            if (m_oInInfo.usesSurfaceY)
            {
                value = (int) (0.5 + tile.m_SurfaceY);
                col = Color::Green();
                if (    ( (i == 0) && (tile.m_SurfaceY < m_oMinSurfaceY || tile.m_SurfaceY > m_oMaxSurfaceY) )
                     || ( (i == 1) && (tile.m_SurfaceY < m_oMinSurfaceY2 || tile.m_SurfaceY > m_oMaxSurfaceY2) )
                   )
                {
                    tile.setSurfaceYDefect();
                    tile.setDefect();
                    col = Color::Red();
                }
            }
            infoLinesPerSurface.push_back(SurfaceInfoLine2Rows(eSurfacePSurfaceY, value, col));

            col = Color::Black();
            value = 0;
            if (m_oInInfo.usesTexture)
            {
                value = (int) (0.5 + tile.m_Texture);
                col = Color::Green();
                if (    ( (i == 0) && (tile.m_Texture < m_oMinTexture || tile.m_Texture > m_oMaxTexture) )
                     || ( (i == 1) && (tile.m_Texture < m_oMinTexture2 || tile.m_Texture > m_oMaxTexture2) )
                   )
                {
                    tile.setTextureDefect();
                    tile.setDefect();
                    col = Color::Red();
                }
            }
            infoLinesPerSurface.push_back(SurfaceInfoLine2Rows(eSurfacePTexture, value, col));

            col = Color::Black();
            value = 0;
            if (m_oInInfo.usesStructure)
            {
                value = (int) (0.5 + tile.m_Structure);
                col = Color::Green();
                if (    ( (i == 0) && (tile.m_Structure < m_oMinStructure || tile.m_Structure > m_oMaxStructure) )
                     || ( (i == 1) && (tile.m_Structure < m_oMinStructure2 || tile.m_Structure > m_oMaxStructure2) )
                   )
                {
                    tile.setStructureDefect();
                    tile.setDefect();
                    col = Color::Red();
                }
            }
            infoLinesPerSurface.push_back(SurfaceInfoLine2Rows(eSurfacePStructure, value, col));

            tileContainer.putSingleTile(i, j, tile);

            m_allInfoLines.push_back(infoLinesPerSurface);
        }
    }

    // 'Tag' collections of defects
    tagBlobs(tileContainer);

    // Search the biggest collection of defects and delete all 'blob numbers' except the
    // biggest one in each row.

    int blobHeight1 = 0;  // Size of biggest defect/blob in first row (big row)
    int blobHeight2 = 0;  // Size of biggest defect/blob in second row (percentage row)

    findBiggestBlobInRows(tileContainer, blobHeight1, blobHeight2);

    m_tileContainer = tileContainer;  // Copy the last tile container for the 'paint' routine

    oOutSizeConnected1.getData().push_back(blobHeight1);
    oOutSizeConnected1.getRank().push_back(255);

    oOutSizeConnected2.getData().push_back(blobHeight2);
    oOutSizeConnected2.getRank().push_back(255);

    // update sampling factors in context
    const auto oSizeConnectedOut1 = GeoDoublearray(rGeoSurfaceInfoIn.context(), oOutSizeConnected1, rGeoSurfaceInfoIn.analysisResult(), 1.0);
    const auto oSizeConnectedOut2 = GeoDoublearray(rGeoSurfaceInfoIn.context(), oOutSizeConnected2, rGeoSurfaceInfoIn.analysisResult(), 1.0);

    preSignalAction();
    m_oPipeOutSizeConnected1.signal(oSizeConnectedOut1);
    m_oPipeOutSizeConnected2.signal(oSizeConnectedOut2);

} // proceed


void SurfaceClassifier2Rows::paint()
{
    if (!m_hasPainting)
    {
        return;
    }

    m_hasPainting = false;

    try
    {
        if (m_oVerbosity < eLow || m_oSpTrafo.isNull())
        {
            return;
        }

        const Trafo   &rTrafo(*m_oSpTrafo);

        OverlayCanvas &rOverlayCanvas = canvas<OverlayCanvas>(m_oCounter);
        OverlayLayer  &rLayerLine     = rOverlayCanvas.getLayerLine();
        OverlayLayer  &rLayerText     = rOverlayCanvas.getLayerText();

        OverlayLayer  &rLayerInfoBox(rOverlayCanvas.getLayerInfoBox());

        for (int j = 0; j < m_tileContainer.m_sizeY; j++)
        {
            for (int i = 0; i < m_tileContainer.m_sizeX; i++)
            {
                // If i = 0: it's the first row = the big tile
                // If i = 1: it's the second row = the inside 'percentage' row = the smaller tile
                // and there should be no more rows!

                const SingleTile &tile = m_tileContainer.getSingleTile(i, j);
                const Rect       oTileRoi(tile.m_startX, tile.m_startY, tile.m_width, tile.m_height);
                const Rect       oTileRoi_1(tile.m_startX + 1, tile.m_startY + 1, tile.m_width, tile.m_height);

                const Rect       oTileRoi2(tile.m_startX, tile.m_startY + 11, tile.m_width, tile.m_height);
                const Rect       oTileRoi2_1(tile.m_startX + 1, tile.m_startY + 12, tile.m_width, tile.m_height);

                const Font       oFont(12);
                Color            oColor = Color::Green();

                if (tile.m_isDefect)
                {
                    oColor = Color::Yellow();
                }

                if (tile.m_blobNumber != 0)
                {
                    oColor = Color::Red();
                }

                rLayerLine.add<OverlayRectangle>(rTrafo(oTileRoi), oColor);

                int displayValue = 0;
                bool isValid = false;

                auto setValueColor = [&displayValue, &isValid, &oColor] (bool valid, double value, bool defect)
                {
                    if (valid)
                    {
                        displayValue = (int) (0.5 + value);
                        isValid = true;
                        oColor = Color::Green();
                        if (defect)
                        {
                            oColor = Color::Red();
                        }
                    }
                };

                switch (m_oDisplay)
                {
                    case 1:
                        setValueColor(tile.m_isMeanValid, tile.m_MeanValue, tile.m_isMeanDefect);
                        break;

                    case 2:
                        setValueColor(tile.m_isRelIntensityValid, tile.m_relIntensity, tile.m_isRelIntDefect);
                        break;

                    case 3:
                        setValueColor(tile.m_isVariationValid, tile.m_Variation, tile.m_isVariationDefect);
                        break;

                    case 4:
                        setValueColor(tile.m_isMinMaxDistanceValid, tile.m_MinMaxDistance, tile.m_isMinMaxDistanceDefect);
                        break;

                    case 5:
                        setValueColor(tile.m_isSurfaceValid, tile.m_Surface, tile.m_isSurfaceDefect);
                        break;

                    case 6:
                        setValueColor(tile.m_isSurfaceXValid, tile.m_SurfaceX, tile.m_isSurfaceXDefect);
                        break;

                    case 7:
                        setValueColor(tile.m_isSurfaceYValid, tile.m_SurfaceY, tile.m_isSurfaceYDefect);
                        break;

                    case 8:
                        setValueColor(tile.m_isTextureValid, tile.m_Texture, tile.m_isTextureDefect);
                        break;

                    case 9:
                        setValueColor(tile.m_isStructureValid, tile.m_Structure, tile.m_isStructureDefect);
                        break;

                    default:
                        displayValue = tile.m_number;
                        isValid = true;
                        oColor = Color::Green();
                        if (tile.m_isDefect)
                        {
                            oColor = Color::Red();
                        }
                }  // case

                if (isValid)
                {
                    // oTileRoi    draws the colored rectangle
                    // oTileRoi_1  draws a black "shadow" rectangle to the colored one
                    rLayerText.add<OverlayText>(std::to_string(displayValue), oFont, rTrafo(oTileRoi_1), Color::Black());
                    rLayerText.add<OverlayText>(std::to_string(displayValue), oFont, rTrafo(oTileRoi), oColor);
                }

                int size = m_allInfoLines.size();

                const auto &infoLinesPerSurface = (tile.m_number - 1 < size)
                                                  ? m_allInfoLines[tile.m_number - 1]
                                                  : m_allInfoLines[size - 1];

                const auto oBoundingBoxStart = Point(tile.m_startX, tile.m_startY);
                const auto oBoundingBoxEnd   = Point(tile.m_startX + tile.m_width, tile.m_startY + tile.m_height);
                const auto oBoundingBox      = Rect(oBoundingBoxStart, oBoundingBoxEnd);

                std::vector<OverlayText> oFeatureLines = std::vector<OverlayText>(infoLinesPerSurface.size() + 1);

                oFeatureLines[0] = OverlayText(id().toString() + ":FILTERGUID:0", Font(12, true, false, "Courier New"), rTrafo(Rect(10, 10, 100, 200)), Color::Black(), 0);

                for (int i = 0; i < (int) infoLinesPerSurface.size(); i++)
                {
                    oFeatureLines[i + 1] = OverlayText(infoLinesPerSurface[i].getLine(), Font(12, true, false, "Courier New"), rTrafo(Rect(10, 10, 100, 200)), infoLinesPerSurface[i]._color, i + 1);
                }

                rLayerInfoBox.add<OverlayInfoBox>(image::eSurface, tile.m_number - 1, std::move(oFeatureLines), rTrafo(oBoundingBox));

            } // for i = 0...SizeX
        }
    }
    catch (...)
    {
        // 'paint' was not successful  =>  only logging
        wmLog(eWarning, "Filter 'surfaceClassifier2Rows': exception in 'paint'!\n");
    }
} // paint


// Helper Functions

void SurfaceClassifier2Rows::tagBlobs(TileContainer &tileContainer)
{
    // Checks the prepared and precalculated tiles. The 'm_blobNumber' is at the begin zero for all tiles. As soon as a defect tile is found,
    // the tile's 'm_blobNumber' is set to the next 'blob candidate number', starting with 1 for the first defect. All defect tiles that are
    // connecting to the actual defect one gets the same 'm_blobNumber'.
    // Voest: the two rows are checked separate!
    // Voest: if there is a 'good' tile in between 2 defect tiles, it will automatically be changed into a 'defect' tile!

    SingleTile otherTile;

    for (int i = 0; i < tileContainer.m_sizeX; i++)
    {
        // If i = 0: it's the first row = the big tile
        // If i = 1: it's the second row = the inside 'percentage' row = the smaller tile
        // and there should be no more rows!

        int curBlobNumber = 1;

        for (int j = 0; j < tileContainer.m_sizeY; j++)
        {

            SingleTile currentTile = tileContainer.getSingleTile(i, j);

            // Check this tile if it's defect, the one above a 'good' and the one above the 'good' a 'defect'.
            // Then the 'good' tile in between this 2 defect tiles must be changed to 'defect'!
            if (currentTile.m_isDefect && (j - 2 >= 0))  // There are 2 tiles above
            {
                otherTile = tileContainer.getSingleTile(i, j - 2);

                if (otherTile.m_isDefect)
                {
                    // The tile 2 places above is 'defect'!!
                    int tempBlobNumber = otherTile.m_blobNumber;

                    otherTile = tileContainer.getSingleTile(i, j - 1);

                    if ( ! otherTile.m_isDefect)
                    {
                        // The tile above the actual is a 'good'  =>  it's in between 2 defects  =>  change it to 'defect'
                        otherTile.m_isDefect = true;
                        otherTile.m_blobNumber = tempBlobNumber;
                        tileContainer.putSingleTile(i, j - 1, otherTile);
                    }

                }
            }

            // Check the neighbouring tiles of the current tile
            if ((currentTile.m_isDefect) && (currentTile.m_blobNumber == 0))  // Defect tile without a blob number
            {
                // Check tile above
                if (j - 1 >= 0)  // There is a tile above
                {
                    otherTile = tileContainer.getSingleTile(i, j - 1);

                    if (otherTile.m_isDefect)  // The tile above is defect, must be tagged
                    {
                        currentTile.m_blobNumber = otherTile.m_blobNumber;
                    }
                }

                // All neighbours done. Check if still no number for actual tile.
                if (currentTile.m_blobNumber == 0)
                {
                    // This is a 'new' defect tile = new blob!
                    currentTile.m_blobNumber = curBlobNumber;
                    // Set counter for next 'new' defect = not connected to the actual defect
                    curBlobNumber++;
                }
            }  // Defect tile without a blob number

            tileContainer.putSingleTile(i, j, currentTile);
        }  // for j
    }  // for i
}


void SurfaceClassifier2Rows::findBiggestBlobInRows(TileContainer &tileContainer, int &blobHeight1, int &blobHeight2)
{
    // Check each of the 2 rows separate
    for (int row = 0; row < tileContainer.m_sizeX; row++)
    {
        // Gets the number of different defects/blobs
        const int maxNumber = getMaxBlobNumber(tileContainer, row);

        std::vector<int> size(maxNumber + 1);

        // Init the counters for each defect/blob
        for (int i = 0; i < (maxNumber + 1); i++)
        {
            size[i] = 0;
        }

        // Count the number of tiles that belong to each defect/blob
        for (int j = 0; j < tileContainer.m_sizeY; j++)
        {
            size[tileContainer.getSingleTile(row, j).m_blobNumber]++;
        }

        int maxSize = 0;
        int maxIndex = 0;

        // Searches the blob number with the biggest amount of tiles.
        // Start with '1' because '0' is not a valid blob number, but initialization.
        for (int i = 1; i < maxNumber + 1; i++)
        {
            if (size[i] > maxSize)
            {
                maxSize = size[i];
                maxIndex = i;
            }
        }

        // Sets all blob numbers of the tiles in the actual row to zero except the biggest one.
        // So it eliminates all blobs in this row except one.
        for (int j = 0; j < tileContainer.m_sizeY; j++)
        {
            SingleTile currentTile = tileContainer.getSingleTile(row, j);

            if (currentTile.m_blobNumber != maxIndex)
            {
                currentTile.m_blobNumber = 0;
                tileContainer.putSingleTile(row, j, currentTile);
            }
        }

        // Save size of biggest blob in row
        if (row == 0)
        {
            blobHeight1 = maxSize;
        }
        else
        {
            blobHeight2 = maxSize;
        }
    }
}


int SurfaceClassifier2Rows::getMaxBlobNumber(TileContainer &tileContainer, const int row)
{
    // Checks all blob numbers and returns the biggest one for the given row

    int maxNumber = 0;

    for (int j = 0; j < tileContainer.m_sizeY; j++)
    {
        const SingleTile &tile = tileContainer.getSingleTile(row, j);
        if (tile.m_blobNumber > maxNumber)
        {
            maxNumber = tile.m_blobNumber;
        }
     }

    return maxNumber;
}


////////////////////////////////////////////////
// Class InfoLine2Rows
// Holds one line of Overlay Textbox display
////////////////////////////////////////////////

SurfaceInfoLine2Rows::SurfaceInfoLine2Rows()
    : _number(eSurfacePNone)
    , _value(0)
    , _color(Color::Black())
{
}

SurfaceInfoLine2Rows::SurfaceInfoLine2Rows(SurfaceParameterDisplay eValue, int value, Color color)
    : _number(eValue)
    , _value(value)
    , _color(color)
{
}

std::string SurfaceInfoLine2Rows::getLine() const
{
    switch (_number)
    {
        case eSurfacePNone:
            return "Unit.None:Unit.None:0";
            break;
        case eSurfacePTileNo:
            return "TileNumber:Unit.None:" + std::to_string(_value);
            break;
        case eSurfacePMean:
            return "Mean:Unit.None:" + std::to_string(_value);
            break;
        case eSurfacePRelBrightness:
            return "RelBrightness:Unit.None:" + std::to_string(_value);
            break;
        case eSurfacePVariation:
            return "Variation:Unit.None:" + std::to_string(_value);
            break;
        case eSurfacePMinMaxDist:
            return "MinMaxDist:Unit.None:" + std::to_string(_value);
            break;
        case eSurfacePSurface:
            return "SurfaceXY:Unit.None:" + std::to_string(_value);
            break;
        case eSurfacePSurfaceX:
            return "SurfaceX:Unit.None:" + std::to_string(_value);
            break;
        case eSurfacePSurfaceY:
            return "SurfaceY:Unit.None:" + std::to_string(_value);
            break;
        case eSurfacePTexture:
            return "Texture:Unit.None:" + std::to_string(_value);
            break;
        case eSurfacePStructure:
            return "Strukture:Unit.None:" + std::to_string(_value);
            break;
    }

    return "Unit.None:Unit.None:0";
}


//////////////////////////
// Class 'Overlay2Rows'
//////////////////////////

SurfaceOverlay2Rows::SurfaceOverlay2Rows()
{
    reset();
}

void SurfaceOverlay2Rows::reset()
{
    _pointContainer.clear();
    _lineContainer.clear();
    _rectangleContainer.clear();
}

const std::vector<SurfacePoint2Rows> &SurfaceOverlay2Rows::getPointContainer() const
{
    return _pointContainer;
}

const std::vector<SurfaceLine2Rows> &SurfaceOverlay2Rows::getLineContainer() const
{
    return _lineContainer;
}

const std::vector<SurfaceRectangle2Rows> &SurfaceOverlay2Rows::getRectangleContainer() const
{
    return _rectangleContainer;
}

void SurfaceOverlay2Rows::addPoint(int x, int y, Color color)
{
    _pointContainer.emplace_back(x, y, color);
}

void SurfaceOverlay2Rows::addLine(int x1, int y1, int x2, int y2, Color color)
{
    _lineContainer.emplace_back(x1, y1, x2, y2, color);
}

void SurfaceOverlay2Rows::addRectangle(int x, int y, int width, int height, Color color)
{
    _rectangleContainer.emplace_back(x, y, width, height, color);
}


//////////////////////////
// Class 'Point2Rows'
//////////////////////////

SurfacePoint2Rows::SurfacePoint2Rows()
    : x(0)
    , y(0)
    , color(Color::Black())
{
}

SurfacePoint2Rows::SurfacePoint2Rows(int x, int y, Color color)
    : x(x)
    , y(y)
    , color(color)
{
}


//////////////////////////
// Class 'Line2Rows'
//////////////////////////

SurfaceLine2Rows::SurfaceLine2Rows()
    : x1(0)
    , y1(0)
    , x2(0)
    , y2(0)
    , color(Color::Black())
{
}

SurfaceLine2Rows::SurfaceLine2Rows(int x1, int y1, int x2, int y2, Color color)
    : x1(x1)
    , y1(y1)
    , x2(x2)
    , y2(y2)
    , color(color)
{
}


//////////////////////////
// Class 'Rectangle2Rows'
//////////////////////////

SurfaceRectangle2Rows::SurfaceRectangle2Rows()
    : x(0)
    , y(0)
    , width(0)
    , height(0)
    , color(Color::Black())
{
}

SurfaceRectangle2Rows::SurfaceRectangle2Rows(int x, int y, int width, int height, Color color)
    : x(x)
    , y(y)
    , width(width)
    , height(height)
    , color(color)
{
}


} // namespace filter
} // namespace precitec
