/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		LB
 * 	@date		2018
 * 	@brief		This filter visualizes a position and a rank using overlay primitives (extension of PosDisplayWithRank).
 */
#include "PosDisplayWithRank.h"
#include "posDisplay.h" // paintCrosshairMedium, paintCrosshairLarge
// WM includes
#include <filter/algoArray.h>
#include <module/moduleLogger.h>
#include <overlay/overlayPrimitive.h>
#include <overlay/overlayPrimitive.h>
#include <overlay/layerType.h>
#include <fliplib/TypeToDataTypeImpl.h>

// std lib
#include <string>
#define _USE_MATH_DEFINES
#include <math.h>

namespace precitec {
using namespace geo2d;
using namespace interface;
using namespace image;
namespace filter {

const std::string PosDisplayWithRank::m_oFilterName("PosDisplayWithRank");
const std::string PosDisplayWithRank::m_oParamStyle("Style");
const std::string PosDisplayWithRank::m_oParamColorRed("RedOK");
const std::string PosDisplayWithRank::m_oParamColorGreen("GreenOK");
const std::string PosDisplayWithRank::m_oParamColorBlue("BlueOK");
const std::string PosDisplayWithRank::m_oParamColorAlpha("AlphaOK");
const std::string PosDisplayWithRank::m_oParamBadColorRed("RedKO");
const std::string PosDisplayWithRank::m_oParamBadColorGreen("GreenKO");
const std::string PosDisplayWithRank::m_oParamBadColorBlue("BlueKO");
const std::string PosDisplayWithRank::m_oParamBadColorAlpha("AlphaKO");
const std::string PosDisplayWithRank::m_oParamRankThreshold("RankThreshold");


PosDisplayWithRank::PosDisplayWithRank() :
TransformFilter(PosDisplayWithRank::m_oFilterName, Poco::UUID{"521c7b38-fecb-4346-9d35-a5f10b39c04e"}),
m_pPipeInDataX(nullptr),
m_pPipeInDataY(nullptr),
m_oStyle(0),
m_oColorOK(Color::Green()),
m_oColorKO(Color::Red()),
m_oRankThreshold(ValueRankType::eRankMax), //decide color based on element rank (0-255)
m_oHandleDifferentContext(false) // by default false,to behave as posDisplay
{
    parameters_.add(PosDisplayWithRank::m_oParamStyle, fliplib::Parameter::TYPE_int, static_cast<unsigned int>(m_oStyle));
    parameters_.add(PosDisplayWithRank::m_oParamColorRed, fliplib::Parameter::TYPE_uint, static_cast<unsigned int>(m_oColorOK.red));
    parameters_.add(PosDisplayWithRank::m_oParamColorGreen, fliplib::Parameter::TYPE_uint, static_cast<unsigned int>(m_oColorOK.green));
    parameters_.add(PosDisplayWithRank::m_oParamColorBlue, fliplib::Parameter::TYPE_uint, static_cast<unsigned int>(m_oColorOK.blue));
    parameters_.add(PosDisplayWithRank::m_oParamColorAlpha, fliplib::Parameter::TYPE_uint, static_cast<unsigned int>(m_oColorOK.alpha));
    parameters_.add(PosDisplayWithRank::m_oParamBadColorRed, fliplib::Parameter::TYPE_uint, static_cast<unsigned int>(m_oColorKO.red));
    parameters_.add(PosDisplayWithRank::m_oParamBadColorGreen, fliplib::Parameter::TYPE_uint, static_cast<unsigned int>(m_oColorKO.green));
    parameters_.add(PosDisplayWithRank::m_oParamBadColorBlue, fliplib::Parameter::TYPE_uint, static_cast<unsigned int>(m_oColorKO.blue));
    parameters_.add(PosDisplayWithRank::m_oParamBadColorAlpha, fliplib::Parameter::TYPE_uint, static_cast<unsigned int>(m_oColorKO.alpha));
    parameters_.add(PosDisplayWithRank::m_oParamRankThreshold, fliplib::Parameter::TYPE_uint, static_cast<unsigned int>(m_oRankThreshold));

    setInPipeConnectors({{Poco::UUID("bf30d1f9-20c9-4a5d-9a84-cf84fb5c1487"), m_pPipeInDataX, "PositionX", 1, "pos_x"},
    {Poco::UUID("584c671f-1795-4bd9-bbfe-ea78562f4beb"), m_pPipeInDataY, "PositionY", 1, "pos_y"}});
    setVariantID(Poco::UUID("f08a5f23-ce9e-4926-811a-8b0df35ef5bc"));
} // CTor.


/*virtual*/ PosDisplayWithRank::~PosDisplayWithRank()
{

} // DTor.

void PosDisplayWithRank::setParameter()
{
    TransformFilter::setParameter();

    m_oStyle = parameters_.getParameter(PosDisplayWithRank::m_oParamStyle).convert<unsigned int>();
    m_oColorOK.red = parameters_.getParameter(PosDisplayWithRank::m_oParamColorRed).convert<byte>();
    m_oColorOK.green = parameters_.getParameter(PosDisplayWithRank::m_oParamColorGreen).convert<byte>();
    m_oColorOK.blue = parameters_.getParameter(PosDisplayWithRank::m_oParamColorBlue).convert<byte>();
    m_oColorOK.alpha = parameters_.getParameter(PosDisplayWithRank::m_oParamColorAlpha).convert<byte>();
    m_oColorKO.red = parameters_.getParameter(PosDisplayWithRank::m_oParamBadColorRed).convert<byte>();
    m_oColorKO.green = parameters_.getParameter(PosDisplayWithRank::m_oParamBadColorGreen).convert<byte>();
    m_oColorKO.blue = parameters_.getParameter(PosDisplayWithRank::m_oParamBadColorBlue).convert<byte>();
    m_oColorKO.alpha = parameters_.getParameter(PosDisplayWithRank::m_oParamBadColorAlpha).convert<byte>();
    m_oRankThreshold = parameters_.getParameter(PosDisplayWithRank::m_oParamRankThreshold).convert<byte>();
} // SetParameter



bool PosDisplayWithRank::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup)
{
    if ( p_rPipe.tag() == "pos_x" )
        m_pPipeInDataX = dynamic_cast< fliplib::SynchronePipe < interface::GeoDoublearray > * >(&p_rPipe);
    if ( p_rPipe.tag() == "pos_y" )
        m_pPipeInDataY = dynamic_cast< fliplib::SynchronePipe < interface::GeoDoublearray > * >(&p_rPipe);

    return BaseFilter::subscribe(p_rPipe, p_oGroup);

} // subscribe



void PosDisplayWithRank::paint()
{
    if ( m_oVerbosity == eNone || m_pTrafo.isNull() )  // filter should not paint anything on verbosity eNone
    {
        return;
    } // if

    const Trafo &rTrafo(*m_pTrafo);
    OverlayCanvas &rCanvas(canvas<OverlayCanvas>(m_oCounter));
    OverlayLayer &rLayer(rCanvas.getLayerPosition());

    switch ( m_oStyle )
    {
        // cross small
        case 0:
            rLayer.add<OverlayCross>(rTrafo(m_oPoint.ref()), 25, m_oColor);
            break;

        // cross medium
        case 1:
            rLayer.add<OverlayCross>(rTrafo(m_oPoint.ref()), 75, m_oColor);
            break;

        // cross large
        case 2:
            rLayer.add<OverlayCross>(rTrafo(m_oPoint.ref()), 350, m_oColor);
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

} // paint



void PosDisplayWithRank::proceedGroup(const void* p_pSender, fliplib::PipeGroupEventArgs& p_rEvent)
{
    poco_assert_dbg(m_pPipeInDataX != nullptr); // to be asserted by graph editor
    poco_assert_dbg(m_pPipeInDataY != nullptr); // to be asserted by graph editor

    if ( m_oVerbosity == eNone )  // filter should not store any coordinate on verbosity eNone.
    {
        preSignalAction();
        return;
    } // if

    const GeoDoublearray &rGeoPosXIn(m_pPipeInDataX->read(m_oCounter));
    const GeoDoublearray &rGeoPosYIn(m_pPipeInDataY->read(m_oCounter));

    const Doublearray &rPosXIn(rGeoPosXIn.ref());
    const Doublearray &rPosYIn(rGeoPosYIn.ref());

    if ( rPosXIn.getData().empty() || rPosYIn.getData().empty() )
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
        wmLog(eDebug, "Filter '%s': Received %u Y values. Y values. Can only process first element, rest will be discarded.\n", m_oFilterName.c_str(), rPosYIn.size());
    }

    if ( m_oHandleDifferentContext && (rGeoPosXIn.context() != rGeoPosYIn.context()) ) // contexts expected to be equal
    {
        std::ostringstream oMsg;
        oMsg << m_oFilterName << ": Different contexts for x and y value: '" << rGeoPosXIn.context() << "', '" << rGeoPosYIn.context() << "'\n";
        wmLog(eWarning, oMsg.str());
    }

    m_pTrafo = rGeoPosXIn.context().trafo();

    const Point oPosOut(doubleToInt(rPosXIn).getData().front(), doubleToInt(rPosYIn).getData().front());

    //compute global rank for the output pipe
    const double oRank((rGeoPosXIn.rank() + rGeoPosYIn.rank()) / 2.);
    m_oPoint = GeoPoint(rGeoPosXIn.context(), oPosOut, rGeoPosXIn.analysisResult(), oRank);

    //for the color, it makes more sense to look at the "element" rank [0,255], which can be set by the arithmetic filter
    m_oColor = std::min(rPosXIn.getRank().front(), rPosYIn.getRank().front() ) >= m_oRankThreshold ? m_oColorOK : m_oColorKO;
    if (inputIsInvalid(rPosXIn) || inputIsInvalid(rPosYIn))
    {
        m_oColor = m_oColorKO;
    }

    preSignalAction();
} // proceedGroup


} // namespace filter
} // namespace precitec
