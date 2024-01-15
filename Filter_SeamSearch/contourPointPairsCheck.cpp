/*!
*  @copyright    Precitec Vision GmbH & Co. KG
*  @author       Urs Gisiger (GUR)
*  @date         2022
*  @file         contourPointPairsCheck.cpp
*  @brief        Fliplib filter 'ContourPointPairsCheck' in component 'Filter_SeamSearch'. Checks left/right contour points for special conditions.
*/


#include "contourPointPairsCheck.h"

#include "system/types.h"               ///< typedefs
#include "common/defines.h"             ///< debug assert integrity assumptions
#include "module/moduleLogger.h"

#include "overlay/overlayPrimitive.h"   ///< paint overlay
#include "filter/algoArray.h"           ///< Intarray algo

#include <algorithm>                    ///< replace
#include <functional>                   ///< equal
#include <fliplib/TypeToDataTypeImpl.h>


using namespace fliplib;

namespace precitec
{
    using namespace image;
    using namespace interface;
    using namespace geo2d;

namespace filter
{

const std::string ContourPointPairsCheck::m_oFilterName    = std::string("ContourPointPairsCheck");
const std::string ContourPointPairsCheck::m_oPipeOutName1  = std::string("SeamPosLeft");
const std::string ContourPointPairsCheck::m_oPipeOutName2  = std::string("SeamPosRight");
const std::string ContourPointPairsCheck::m_oPipeOutName3  = std::string("SeamPosOK");


ContourPointPairsCheck::ContourPointPairsCheck() :
TransformFilter                     ( ContourPointPairsCheck::m_oFilterName, Poco::UUID{"e6fcb9e4-e82c-4e37-898b-99af040b3317"} ),
    m_pPipeInContourL               (nullptr),
    m_pPipeInContourR               (nullptr),
    m_pPipeInImgSize                (nullptr),
    m_oPipeOutSeamPosL              ( this, m_oPipeOutName1 ),
    m_oPipeOutSeamPosR              ( this, m_oPipeOutName2 ),
    m_oPipeOutSeamPosOK             ( this, m_oPipeOutName3 ),
    m_oSeamPosL                     (1),     // one output point, for left X position of (found) "good SeamPos"
    m_oSeamPosR                     (1),     // one output point, for right X position of (found) "good SeamPos"
    m_oNumberBigGradient            (3),     // min 3 found point pairs with big gradient
    m_oMaxStandardDeviationWidth    (3.0),
    m_oMaxDistancePosition          (3),
    m_oSeamPosOK                    (false)  // true, if found SeamPos is good
{
    // set parameter default values

    parameters_.add("MinNumberBigGradient", Parameter::TYPE_int, m_oNumberBigGradient);
    parameters_.add("MaxStandardDevWidth",  Parameter::TYPE_double, m_oMaxStandardDeviationWidth);
    parameters_.add("MaxDistancePosition",  Parameter::TYPE_int, m_oMaxDistancePosition);

    setInPipeConnectors ({ {Poco::UUID("c641caa4-597c-4e54-a519-c264678000b9"), m_pPipeInContourL, "ContourLeft",  1, "contour_left"},
                           {Poco::UUID("d3167b8a-b6a2-401f-8327-1d28d85843cb"), m_pPipeInContourR, "ContourRight", 1, "contour_right"},
                           {Poco::UUID("2daddb38-8cb0-4c04-bad2-311de1576a92"), m_pPipeInImgSize,  "ImgSize",      1, ""}
                        });
    setOutPipeConnectors({ {Poco::UUID("a52de818-aca6-4dde-98fe-b34bfd8aa4de"), &m_oPipeOutSeamPosL,  m_oPipeOutName1, 0, ""},
                           {Poco::UUID("817821e6-a00b-452a-87b4-055a3547390b"), &m_oPipeOutSeamPosR,  m_oPipeOutName2, 0, ""},
                           {Poco::UUID("eead3d53-53c7-4f10-ac96-844419f772e9"), &m_oPipeOutSeamPosOK, m_oPipeOutName3, 0, ""}
                        });
    setVariantID(Poco::UUID("2fd0336b-922d-4163-b9d5-e5cb6b6a2a44"));
} // ContourPointPairsCheck


ContourPointPairsCheck::~ContourPointPairsCheck() = default;


void ContourPointPairsCheck::setParameter()
{
    TransformFilter::setParameter();
    m_oNumberBigGradient         = parameters_.getParameter("MinNumberBigGradient").convert<int>();
    m_oMaxStandardDeviationWidth = parameters_.getParameter("MaxStandardDevWidth").convert<double>();
    m_oMaxDistancePosition       = parameters_.getParameter("MaxDistancePosition").convert<int>();

    // Parameter assertion. Should be pre-checked by UI / MMI / GUI.
    poco_assert_dbg(m_oNumberBigGradient >= 1);
    poco_assert_dbg(m_oMaxStandardDeviationWidth >= 0);
    poco_assert_dbg(m_oMaxDistancePosition >= 0);
} // setParameter


void ContourPointPairsCheck::paint()
{
    if (m_oVerbosity <= eNone)
    {
        return;
    } // if

    // paint contour points

    const GeoDoublearray  &rGeoContourLIn ( m_pPipeInContourL->read(m_oCounter) );
    const GeoDoublearray  &rGeoContourRIn ( m_pPipeInContourR->read(m_oCounter) );
    const unsigned int    oNProfiles      ( rGeoContourLIn.ref().getData().size() );
    const int             oCrossRadius    ( 4 );
    const int             oDeltaY         ( oNProfiles != 0 ? m_oImageSize.height / oNProfiles : 0 );
    int                   oY              ( oDeltaY / 2 );

    if (rGeoContourLIn.context().trafo().isNull())
    {
        return;
    }

    const Trafo&          rTrafo          ( *rGeoContourLIn.context().trafo() );
    OverlayCanvas&        rCanvas         ( canvas<OverlayCanvas>(m_oCounter) );
    OverlayLayer&         rLayerPosition  ( rCanvas.getLayerPosition());
    OverlayLayer&         rLayerLine      ( rCanvas.getLayerLine());

    const auto&           rContourLIn     ( rGeoContourLIn.ref().getData() );
    const auto&           rContourRIn     ( rGeoContourRIn.ref().getData() );
    const auto&           rContourLInRank ( rGeoContourLIn.ref().getRank() );
    const auto&           rContourRInRank ( rGeoContourRIn.ref().getRank() );
    const auto&           rSeamPosLOut    ( m_oSeamPosL.getData() );
    const auto&           rSeamPosROut    ( m_oSeamPosR.getData() );

    // Print a red cross (size 6) at top of grey ROI for found positions
    if (m_oSeamPosOK)
    {
        const Point oPositionLOut ( int( rSeamPosLOut[0]), 0 );
        rLayerPosition.add( new  OverlayCross(rTrafo(oPositionLOut), 8, Color::Red() ) );
        const Point oPositionROut ( int( rSeamPosROut[0]), 0 );
        rLayerPosition.add( new  OverlayCross(rTrafo(oPositionROut), 8, Color::Red() ) );
    }

    for (unsigned int sliceN = 0; sliceN < oNProfiles; ++sliceN)  // loop over N profiles
    {
        const Point oPositionLIn ( int( rContourLIn[sliceN]), oY );

        if (rContourLInRank[sliceN] == eRankMax)
        {
            // Is a "good SeamPos" candidate !

            // Print a cross at the gradient's position
            rLayerPosition.add( new  OverlayCross(rTrafo(oPositionLIn), oCrossRadius, Color::Yellow() ) );

            if ( (m_oVerbosity >= eMedium) && m_oSeamPosOK )
            {
                // Draw a line to the "good SeamPos" position (horizontally)
                const Point oPositionLOut( int( rSeamPosLOut[0] ), oY );
                rLayerLine.add( new OverlayLine(rTrafo(oPositionLIn), rTrafo(oPositionLOut), Color::Yellow() ) );
            }
        }

        const Point oPositionRIn ( int( rContourRIn[sliceN]), oY );

        if (rContourRInRank[sliceN] == eRankMax)
        {
            // Is a "good SeamPos" candidate !

            // Print a cross at the gradient's position
            rLayerPosition.add( new  OverlayCross(rTrafo(oPositionRIn), oCrossRadius, Color::Magenta() ) );

            if ( (m_oVerbosity >= eMedium) && m_oSeamPosOK )
            {
                // Draw a line to the "good SeamPos" position (horizontally)
                const Point oPositionROut( int( rSeamPosROut[0] ), oY );
                rLayerLine.add( new OverlayLine(rTrafo(oPositionRIn), rTrafo(oPositionROut), Color::Magenta() ) );
            }
        }

        oY += oDeltaY;
    } // for

} // paint


bool ContourPointPairsCheck::subscribe(fliplib::BasePipe & p_rPipe, int p_oGroup)
{
    if (p_rPipe.tag() == "contour_left")
        m_pPipeInContourL  = dynamic_cast< scalar_pipe_t* >(&p_rPipe);
    else if (p_rPipe.tag() == "contour_right")
        m_pPipeInContourR  = dynamic_cast< scalar_pipe_t* >(&p_rPipe);
    else
        m_pPipeInImgSize = dynamic_cast< scalar_pipe_t * >(&p_rPipe);

    return BaseFilter::subscribe( p_rPipe, p_oGroup );
} // subscribe


void ContourPointPairsCheck::proceedGroup(const void* sender, PipeGroupEventArgs & e)
{
    poco_assert_dbg(m_pPipeInContourL != nullptr); // to be asserted by graph editor
    poco_assert_dbg(m_pPipeInContourR != nullptr); // to be asserted by graph editor
    poco_assert_dbg(m_pPipeInImgSize  != nullptr); // to be asserted by graph editor

    // get data from frame

    const GeoDoublearray rContourLIn = m_pPipeInContourL->read(m_oCounter);
    const GeoDoublearray rContourRIn = m_pPipeInContourR->read(m_oCounter);
    const auto &         rImgSizeIn  = m_pPipeInImgSize->read(m_oCounter).ref().getData();

    reinitialize(); // (re)initialization of output structure

    // input validity check

    if ( inputIsInvalid(rContourLIn) || inputIsInvalid(rContourRIn) || rContourLIn.ref().size() != rContourRIn.ref().size() )
    {
        const GeoDoublearray oGeoSeamPosLeftOut  (rContourLIn.context(), m_oSeamPosL, rContourLIn.analysisResult(), 0.0);  // bad rank
        const GeoDoublearray oGeoSeamPosRightOut (rContourRIn.context(), m_oSeamPosR, rContourRIn.analysisResult(), 0.0);  // bad rank
        // put parameter zero into an array of length 1 with max rank
        const GeoDoublearray oGeoSeamPosOKOut    (rContourRIn.context(), Doublearray(1, 0, eRankMax), rContourRIn.analysisResult(), 0.0 );  // bad rank

        preSignalAction();
        m_oPipeOutSeamPosL.signal( oGeoSeamPosLeftOut );
        m_oPipeOutSeamPosR.signal( oGeoSeamPosRightOut );
        m_oPipeOutSeamPosOK.signal( oGeoSeamPosOKOut );

        return;
    }

    m_oImageSize.width  = int( rImgSizeIn[0] );
    m_oImageSize.height = int( rImgSizeIn[1] );

    // signal processing
    calcConditionsForSeamPos ( rContourLIn.ref(), rContourRIn.ref(),
                               m_oNumberBigGradient,
                               m_oMaxStandardDeviationWidth,
                               m_oMaxDistancePosition,
                               m_oSeamPosL, m_oSeamPosR,
                               m_oSeamPosOK);

    double oNewRank = eRankMin;  // Default is bad rank
    if (m_oSeamPosOK)
    {
        // SeamPos is good  =>  set rank for SeamPos to max
        oNewRank = eRankMax;
    }
    const GeoDoublearray oGeoSeamPosLeftOut  (rContourLIn.context(), m_oSeamPosL, rContourLIn.analysisResult(), oNewRank);
    const GeoDoublearray oGeoSeamPosRightOut (rContourRIn.context(), m_oSeamPosR, rContourRIn.analysisResult(), oNewRank);
    // put parameter m_oSeamPosOK into an array of length 1 with max rank
    const GeoDoublearray oGeoSeamPosOKOut    (rContourRIn.context(), Doublearray(1, m_oSeamPosOK, eRankMax), rContourRIn.analysisResult(), eRankMax );

    preSignalAction();
    m_oPipeOutSeamPosL.signal( oGeoSeamPosLeftOut );
    m_oPipeOutSeamPosR.signal( oGeoSeamPosRightOut );
    m_oPipeOutSeamPosOK.signal( oGeoSeamPosOKOut );
} // proceed


void ContourPointPairsCheck::reinitialize ()
{
    m_oSeamPosL.reinitialize();
    m_oSeamPosR.reinitialize();
} // reinitialize


void ContourPointPairsCheck::calcConditionsForSeamPos  ( const Doublearray  &p_rContourLIn,
                                                         const Doublearray  &p_rContourRIn,
                                                         int                p_oNumberBigGradient,
                                                         double             p_oMaxStandardDeviationWidth,
                                                         int                p_oMaxDistancePosition,
                                                         Doublearray        &p_oSeamPosL,
                                                         Doublearray        &p_oSeamPosR,
                                                         bool               &p_oSeamPosOK
                                                       )
{
    // equal size for left and right input contour asserted - see above
    poco_assert_dbg( p_rContourLIn.size() == p_rContourRIn.size() );

    const unsigned int  oNumberContourPoints = p_rContourLIn.size();
    int                 oNumberOfBigGrad     = 0;
    int                 oMeanPointDistances  = 0;
    int                 oMeanCenterPosition  = 0;
    int                 oMeanXLeft           = 0;
    int                 oMeanXRight          = 0;

    int                 oActualWidth;
    double              oSummWidth           = 0;
    double              oSummWidthSquare     = 0;

    for (unsigned int oContourPairN = 0; oContourPairN < oNumberContourPoints; oContourPairN++)
    {
        const double oContourPosLIn  = p_rContourLIn.getData()[oContourPairN];
        const int    oContourRankLIn = p_rContourLIn.getRank()[oContourPairN];

        const double oContourPosRIn  = p_rContourRIn.getData()[oContourPairN];
        const int    oContourRankRIn = p_rContourRIn.getRank()[oContourPairN];

        if (    (oContourRankLIn == eRankMax)
             && (oContourRankRIn == eRankMax)
           )
        {
            // This contour pair is used because of big gradients
            oNumberOfBigGrad++;

            // Sum the point distances for "Standard deviation" calculation
            if (oContourPosLIn <= oContourPosRIn)
            {
                oActualWidth = oContourPosRIn - oContourPosLIn;
                oSummWidth += oActualWidth;
            }

            // Sum the point's center position for "Max. distance positions" calculation
            oMeanCenterPosition += (oContourPosRIn + oContourPosLIn) / 2;
        }
    }

    // Check number of found "big" gradients. If number is too small, no "good SeamPos" decision possible!
    if (oNumberOfBigGrad < p_oNumberBigGradient)
    {
        p_oSeamPosOK = false;

        return;
    }

    // Zero condition already checked above!
    oMeanCenterPosition /= oNumberOfBigGrad;

    // Recalculate diff. to mean values
    oNumberOfBigGrad   = 0;
    oSummWidth         = 0;

    for (unsigned int oContourPairN = 0; oContourPairN < oNumberContourPoints; oContourPairN++)
    {
        const double oContourPosLIn  = p_rContourLIn.getData()[oContourPairN];
        const int    oContourRankLIn = p_rContourLIn.getRank()[oContourPairN];

        const double oContourPosRIn  = p_rContourRIn.getData()[oContourPairN];
        const int    oContourRankRIn = p_rContourRIn.getRank()[oContourPairN];

        if (    // Has this contour pair big gradients ?
                (    (oContourRankLIn == eRankMax)
                  && (oContourRankRIn == eRankMax)
                )
             && // Is center pos. of this contour pair "near" the mean value ?
                (    fabs( ((oContourPosRIn + oContourPosLIn) / 2) - oMeanCenterPosition )
                  <= p_oMaxDistancePosition
                )
           )
        {
            // This contour pair is "good SeamPos" candidate

            // New values for "Standard deviation" calculation
            if (oContourPosLIn <= oContourPosRIn)
            {
                oActualWidth = oContourPosRIn - oContourPosLIn;
                oSummWidth += oActualWidth;
                oSummWidthSquare += oActualWidth * oActualWidth;
            }

            // Values for special left/right SeamPos X positions
            oMeanXLeft += oContourPosLIn;
            oMeanXRight += oContourPosRIn;

            oNumberOfBigGrad++;
        }
    }

    // Check number of found "good SeamPos" candidates.
    if (oNumberOfBigGrad < p_oNumberBigGradient)
    {
        // No, not enough candidates!
        p_oSeamPosOK = false;
    }
    else
    {
        // Check now the "Standard deviation" of contour distance
        oMeanPointDistances = oSummWidth / oNumberOfBigGrad;    // Zero condition already checked above!

        double oStandardDevWidth = sqrt (   (   oMeanPointDistances * oMeanPointDistances * oNumberOfBigGrad
                                              - 2 * oMeanPointDistances * oSummWidth
                                              + oSummWidthSquare
                                            )
                                          / oNumberOfBigGrad
                                        );

        if (oStandardDevWidth <= p_oMaxStandardDeviationWidth)
        {
            p_oSeamPosOK = true;

            // Calculate and set the left/right X positions of the SeamPos
            p_oSeamPosL.getData()[0] = oMeanXLeft / oNumberOfBigGrad;
            p_oSeamPosL.getRank()[0] = eRankMax;
            p_oSeamPosR.getData()[0] = oMeanXRight / oNumberOfBigGrad;
            p_oSeamPosR.getRank()[0] = eRankMax;
        }
        else
        {
            p_oSeamPosOK = false;
        }
    }

    return;
} // calcConditionsForSeamPos


} // namespace filter
} // namespace precitec
