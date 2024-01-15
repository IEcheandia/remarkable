/**
 * @file
 * @copyright	Precitec Vision GmbH & Co. KG
 * @author		MM
 * @date		2021
 * @brief		This filter visualizes a position and a rank using overlay primitives (extension of PosDisplayWithRank).
 *              Analogue to PosDisplayWithRank with additional input to turn off the painting.
 */
#include "PosDisplayWithRankOptDraw.h"
#include "posDisplay.h"

#include <filter/algoArray.h>
#include <module/moduleLogger.h>
#include <overlay/overlayPrimitive.h>
#include <overlay/overlayPrimitive.h>
#include <overlay/layerType.h>
#include <fliplib/TypeToDataTypeImpl.h>


namespace precitec
{
namespace filter
{

const std::string PosDisplayWithRankOptDraw::m_oFilterName("PosDisplayWithRankOptDraw");
const std::string PosDisplayWithRankOptDraw::m_oParamStyle("Style");
const std::string PosDisplayWithRankOptDraw::m_oParamColorRed("RedOK");
const std::string PosDisplayWithRankOptDraw::m_oParamColorGreen("GreenOK");
const std::string PosDisplayWithRankOptDraw::m_oParamColorBlue("BlueOK");
const std::string PosDisplayWithRankOptDraw::m_oParamColorAlpha("AlphaOK");
const std::string PosDisplayWithRankOptDraw::m_oParamBadColorRed("RedKO");
const std::string PosDisplayWithRankOptDraw::m_oParamBadColorGreen("GreenKO");
const std::string PosDisplayWithRankOptDraw::m_oParamBadColorBlue("BlueKO");
const std::string PosDisplayWithRankOptDraw::m_oParamBadColorAlpha("AlphaKO");
const std::string PosDisplayWithRankOptDraw::m_oParamRankThreshold("RankThreshold");


PosDisplayWithRankOptDraw::PosDisplayWithRankOptDraw()
    : TransformFilter(PosDisplayWithRankOptDraw::m_oFilterName, Poco::UUID{"b7df3dfe-291d-44ea-9e16-f3e40f971f4f"})
    , m_pPipeInDataX(nullptr)
    , m_pPipeInDataY(nullptr)
    , m_pPipeInDataDraw(nullptr)
    , m_oStyle(0)
    , m_oColorOK(image::Color::Green())
    , m_oColorKO(image::Color::Red())
    , m_oRankThreshold(ValueRankType::eRankMax) //decide color based on element rank (0-255)
{
    parameters_.add(PosDisplayWithRankOptDraw::m_oParamStyle, fliplib::Parameter::TYPE_int, static_cast<unsigned int>(m_oStyle));
    parameters_.add(PosDisplayWithRankOptDraw::m_oParamColorRed, fliplib::Parameter::TYPE_uint, static_cast<unsigned int>(m_oColorOK.red));
    parameters_.add(PosDisplayWithRankOptDraw::m_oParamColorGreen, fliplib::Parameter::TYPE_uint, static_cast<unsigned int>(m_oColorOK.green));
    parameters_.add(PosDisplayWithRankOptDraw::m_oParamColorBlue, fliplib::Parameter::TYPE_uint, static_cast<unsigned int>(m_oColorOK.blue));
    parameters_.add(PosDisplayWithRankOptDraw::m_oParamColorAlpha, fliplib::Parameter::TYPE_uint, static_cast<unsigned int>(m_oColorOK.alpha));
    parameters_.add(PosDisplayWithRankOptDraw::m_oParamBadColorRed, fliplib::Parameter::TYPE_uint, static_cast<unsigned int>(m_oColorKO.red));
    parameters_.add(PosDisplayWithRankOptDraw::m_oParamBadColorGreen, fliplib::Parameter::TYPE_uint, static_cast<unsigned int>(m_oColorKO.green));
    parameters_.add(PosDisplayWithRankOptDraw::m_oParamBadColorBlue, fliplib::Parameter::TYPE_uint, static_cast<unsigned int>(m_oColorKO.blue));
    parameters_.add(PosDisplayWithRankOptDraw::m_oParamBadColorAlpha, fliplib::Parameter::TYPE_uint, static_cast<unsigned int>(m_oColorKO.alpha));
    parameters_.add(PosDisplayWithRankOptDraw::m_oParamRankThreshold, fliplib::Parameter::TYPE_uint, static_cast<unsigned int>(m_oRankThreshold));

    setInPipeConnectors({{Poco::UUID("911209b8-7f2e-4d3c-bfde-085ceff5b53c"), m_pPipeInDataX, "PositionX", 1, "pos_x"},
    {Poco::UUID("ea5fdd36-a5a4-4d6d-a788-7a45168bb2c9"), m_pPipeInDataY, "PositionY", 1, "pos_y"},
    {Poco::UUID("b2927a6d-1e5d-482b-a8e0-319e2d5c8a3d"), m_pPipeInDataDraw, "Draw", 1, "draw"}
    });
    setVariantID(Poco::UUID("10d70bf1-12f4-4d3d-a607-ff34c690f506"));
}

PosDisplayWithRankOptDraw::~PosDisplayWithRankOptDraw() = default;

void PosDisplayWithRankOptDraw::setParameter()
{
    TransformFilter::setParameter();

    m_oStyle = parameters_.getParameter(PosDisplayWithRankOptDraw::m_oParamStyle).convert<unsigned int>();
    m_oColorOK.red = parameters_.getParameter(PosDisplayWithRankOptDraw::m_oParamColorRed).convert<byte>();
    m_oColorOK.green = parameters_.getParameter(PosDisplayWithRankOptDraw::m_oParamColorGreen).convert<byte>();
    m_oColorOK.blue = parameters_.getParameter(PosDisplayWithRankOptDraw::m_oParamColorBlue).convert<byte>();
    m_oColorOK.alpha = parameters_.getParameter(PosDisplayWithRankOptDraw::m_oParamColorAlpha).convert<byte>();
    m_oColorKO.red = parameters_.getParameter(PosDisplayWithRankOptDraw::m_oParamBadColorRed).convert<byte>();
    m_oColorKO.green = parameters_.getParameter(PosDisplayWithRankOptDraw::m_oParamBadColorGreen).convert<byte>();
    m_oColorKO.blue = parameters_.getParameter(PosDisplayWithRankOptDraw::m_oParamBadColorBlue).convert<byte>();
    m_oColorKO.alpha = parameters_.getParameter(PosDisplayWithRankOptDraw::m_oParamBadColorAlpha).convert<byte>();
    m_oRankThreshold = parameters_.getParameter(PosDisplayWithRankOptDraw::m_oParamRankThreshold).convert<byte>();
}

bool PosDisplayWithRankOptDraw::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup)
{
    if ( p_rPipe.tag() == "pos_x" )
    {
        m_pPipeInDataX = dynamic_cast< fliplib::SynchronePipe < interface::GeoDoublearray > * >(&p_rPipe);
    }
    if ( p_rPipe.tag() == "pos_y" )
    {
        m_pPipeInDataY = dynamic_cast< fliplib::SynchronePipe < interface::GeoDoublearray > * >(&p_rPipe);
    }
    if ( p_rPipe.tag() == "draw" )
    {
        m_pPipeInDataDraw  = dynamic_cast< fliplib::SynchronePipe < interface::GeoDoublearray > * >(&p_rPipe);
    }

    return BaseFilter::subscribe(p_rPipe, p_oGroup);

}

void PosDisplayWithRankOptDraw::paint()
{
    if ( m_oVerbosity == eNone || m_pTrafo.isNull() )  // filter should not paint anything on verbosity eNone
    {
        return;
    }

    const interface::Trafo &rTrafo(*m_pTrafo);
    image::OverlayCanvas &rCanvas(canvas<image::OverlayCanvas>(m_oCounter));
    image::OverlayLayer &rLayer(rCanvas.getLayerPosition());

    switch ( m_oStyle )
    {
        // cross small
        case 0:
            rLayer.add<image::OverlayCross>(rTrafo(m_oPoint.ref()), 25, m_oColor);
            break;

        // cross medium
        case 1:
            rLayer.add<image::OverlayCross>(rTrafo(m_oPoint.ref()), 75, m_oColor);
            break;

        // cross large
        case 2:
            rLayer.add<image::OverlayCross>(rTrafo(m_oPoint.ref()), 350, m_oColor);
            break;

        // crosshair medium
        case 3:
            PosDisplay::paintCrosshairMedium(rLayer, m_oPoint.ref(), m_oColor, rTrafo);
            break;

        // crosshair large
        case 4:
            PosDisplay::paintCrosshairLarge(rLayer, m_oPoint.ref(), m_oColor, rTrafo);
            break;
    }
}

void PosDisplayWithRankOptDraw::proceedGroup(const void* p_pSender, fliplib::PipeGroupEventArgs& p_rEvent)
{
    poco_assert_dbg(m_pPipeInDataX != nullptr); // to be asserted by graph editor
    poco_assert_dbg(m_pPipeInDataY != nullptr); // to be asserted by graph editor

    if ( m_oVerbosity == eNone )  // filter should not store any coordinate on verbosity eNone.
    {
        preSignalAction();
        return;
    }

    const interface::GeoDoublearray &rGeoPosXIn(m_pPipeInDataX->read(m_oCounter));
    const interface::GeoDoublearray &rGeoPosYIn(m_pPipeInDataY->read(m_oCounter));
    const interface::GeoDoublearray &rGeoDrawIn(m_pPipeInDataDraw->read(m_oCounter));

    const geo2d::Doublearray &rPosXIn(rGeoPosXIn.ref());
    const geo2d::Doublearray &rPosYIn(rGeoPosYIn.ref());
    const geo2d::Doublearray &rDrawIn(rGeoDrawIn.ref());

    if ( rPosXIn.getData().empty() || rPosYIn.getData().empty() || rDrawIn.getData().empty() || rDrawIn.getData()[0] == 0 )
    {
        m_pTrafo.assign(nullptr); //disable paint
        preSignalAction();
        return;
    }

    if ( rPosXIn.size() != 1 ) // result is always one point
    {
        wmLog(eDebug, "Filter '%s': Received %u X values. Can only process first element, rest will be discarded.\n", m_oFilterName.c_str(), rPosXIn.size());
    }
    if ( rPosYIn.size() != 1 ) // result is always one point
    {
        wmLog(eDebug, "Filter '%s': Received %u Y values. Can only process first element, rest will be discarded.\n", m_oFilterName.c_str(), rPosYIn.size());
    }

    if (rGeoPosXIn.context() != rGeoPosYIn.context()) // contexts expected to be equal
    {
        std::ostringstream oMsg;
        oMsg << m_oFilterName << ": Different contexts for x and y value: '" << rGeoPosXIn.context() << "', '" << rGeoPosYIn.context() << "'\n";
        wmLog(eWarning, oMsg.str());
    }

    m_pTrafo = rGeoPosXIn.context().trafo();

    const geo2d::Point oPosOut(doubleToInt(rPosXIn).getData().front(), doubleToInt(rPosYIn).getData().front());

    //compute global rank for the output pipe
    const double oRank(0.5 * (rGeoPosXIn.rank() + rGeoPosYIn.rank()));
    m_oPoint = interface::GeoPoint(rGeoPosXIn.context(), oPosOut, rGeoPosXIn.analysisResult(), oRank);

    //for the color, it makes more sense to look at the "element" rank [0,255], which can be set by the arithmetic filter
    m_oColor = std::min(rPosXIn.getRank().front(), rPosYIn.getRank().front() ) >= m_oRankThreshold ? m_oColorOK : m_oColorKO;
    if (inputIsInvalid(rPosXIn) || inputIsInvalid(rPosYIn))
    {
        m_oColor = m_oColorKO;
    }

    preSignalAction();
}

}
}
