/*!
 *  @copyright      Precitec Vision GmbH & Co. KG
 *  @author         Urs Gisiger (GUR)
 *  @date           2022
 *  @file
 *  @brief          Fliplib filter 'SelectPeaksCont' in component 'Filter_SeamSearch'.
 *                  Copied from filter 'SelectPeaks', added inpipes 'StartPosition' and 'CenterPosition'.
 *                  Calculates right and left seam position, using the 'CenterPosition' from the before
 *                  image as "expected" seam center position for the actual image.
 */


#include "selectPeaksCont.h"

#include "seamSearch.h"                         ///< input check, rank calculation

#include "system/types.h"                       ///< typedefs
#include "common/defines.h"                     ///< debug assert integrity assumptions
#include "module/moduleLogger.h"

#include "overlay/overlayPrimitive.h"           ///< paint overlay
#include "image/image.h"                        ///< BImage

#include "filter/algoArray.h"                   ///< Doublearray algo

#include <limits>                               ///< min int
#include <fliplib/TypeToDataTypeImpl.h>

using namespace fliplib;
namespace precitec {
    using namespace image;
    using namespace interface;
    using namespace geo2d;
namespace filter {


const std::string SelectPeaksCont::m_oFilterName   = std::string("SelectPeaksCont");
const std::string SelectPeaksCont::m_oPipeOutName1 = std::string("ContourLeft");
const std::string SelectPeaksCont::m_oPipeOutName2 = std::string("ContourRight");


SelectPeaksCont::SelectPeaksCont() :
TransformFilter                ( m_oFilterName, Poco::UUID{"f99f3915-4ec6-41ce-b4ec-26b488686975"} ),
    m_pPipeInGradientLeft      ( nullptr ),
    m_pPipeInGradientRight     ( nullptr ),
    m_pPipeInMaxFLenght        ( nullptr ),
    m_pPipeInImgSize           ( nullptr ),
    m_pPipeInDefaultSeamWidth  ( nullptr ),
    m_pPipeInCenterPos         ( nullptr ),
    m_pPipeInStartPos          ( nullptr ),
    m_oPipeOutContourLeft      ( this, m_oPipeOutName1 ),
    m_oPipeOutContourRight     ( this, m_oPipeOutName2 ),
    m_oMaxFilterLenght         ( 20 ),
    m_oDefaultSeamWidth        ( 220 ),
    m_oCenterPosROI            ( 0 ),
    m_oStartPos                ( 0 ),
    m_oThresholdLeft           ( 4 ),
    m_oThresholdRight          ( 4 )
{
    // Set default values for the parameters of the filter
    parameters_.add("ThresholdLeft",    Parameter::TYPE_int, m_oThresholdLeft);
    parameters_.add("ThresholdRight",   Parameter::TYPE_int, m_oThresholdRight);

    setInPipeConnectors({
        {Poco::UUID("d6cd9339-5500-43a9-843c-1db04f063dbb"), m_pPipeInGradientLeft, "GradientLeft", 1, "gradient_left"},
        {Poco::UUID("4286a6fd-3ff9-48a1-b21d-3b3784211560"), m_pPipeInGradientRight, "GradientRight", 1, "gradient_right"},
        {Poco::UUID("3f4fcec4-89a2-4c59-8f63-b332357b8a2e"), m_pPipeInMaxFLenght, "MaxFilterLength", 1, "max_filter_length"},
        {Poco::UUID("b4a8fcec-785c-493f-b2d7-56824de18c82"), m_pPipeInImgSize, "ImageSize", 1, "image_size"},
        {Poco::UUID("9e2c253e-5441-4e32-82bc-d8f2b8592ae3"), m_pPipeInDefaultSeamWidth, "DefaultSeamWidth", 1, "default_seamWidth"},
        {Poco::UUID("93597b36-66a6-4ad7-8fe9-c6f59c8b0d90"), m_pPipeInCenterPos, "CenterPosition", 1, "centerpos"},
        {Poco::UUID("d02bce52-cd06-40fd-8bd4-2978589ea2e2"), m_pPipeInStartPos, "StartPosition", 1, "startpos"}});
    setOutPipeConnectors({
        {Poco::UUID("bdb65bb2-b277-4775-9db7-26ee1febe0a7"), &m_oPipeOutContourLeft, m_oPipeOutName1, 0, ""},
        {Poco::UUID("b3ccd664-e742-40df-9b9f-1bc54fb3069b"), &m_oPipeOutContourRight, m_oPipeOutName2, 0, ""}});
    setVariantID(Poco::UUID("aaf6407a-37f0-43b0-8de6-f973ac6b81a1"));
} // SeamposFromPeaksCont


void SelectPeaksCont::setParameter()
{
    TransformFilter::setParameter();
    m_oThresholdLeft    = parameters_.getParameter("ThresholdLeft").convert<int>();
    m_oThresholdRight   = parameters_.getParameter("ThresholdRight").convert<int>();

    // Parameter assertion. Should be pre-checked by UI / MMI / GUI.
    poco_assert_dbg(m_oDefaultSeamWidth > 0);
} // setParameter


void SelectPeaksCont::paint()
{
    if  (m_oVerbosity <= eNone || m_oContourLeftOut.size() == 0)
    {
        return;
    } // if

    // paint contour points

    const GeoVecDoublearray &rGradientLeftIn = m_pPipeInGradientLeft->read(m_oCounter);
    poco_assert_dbg( ! rGradientLeftIn.ref().empty() );

    const int           oCrossRadius    ( 4 );
    const unsigned int  oNProfiles      ( rGradientLeftIn.ref().size() );
    const int           oDeltaY         ( m_oImageSize.height / oNProfiles );
    int                 oY              ( oDeltaY / 2 );

    auto oSpTrafo = rGradientLeftIn.context().trafo();
    if (oSpTrafo.isNull())
    {
        return;
    }

    const Trafo &   rTrafo          ( *oSpTrafo );
    OverlayCanvas & rOverlayCanvas  ( canvas<OverlayCanvas>(m_oCounter) );
    OverlayLayer &  rLayerPosition  ( rOverlayCanvas.getLayerPosition() );
    OverlayLayer &  rLayerText      ( rOverlayCanvas.getLayerText() );

    for (unsigned int sliceN = 0; sliceN < oNProfiles; ++sliceN)
    {
        // loop over N profiles
        const Point         oPositionLeft   ( int(m_oContourLeftOut.getData()[sliceN]), oY );
        const Color         oColorL         ( Color::Yellow() );
        const Color         oColorR         ( Color::Magenta() );
        std::stringstream   oTmpL;

        oTmpL << m_oContourLeftOut.getRank()[sliceN];
        rLayerPosition.add<OverlayCross>( rTrafo(oPositionLeft), oCrossRadius, oColorL );
        rLayerText.add<OverlayText>( oTmpL.str(), image::Font(), rTrafo(Rect(oPositionLeft.x, oPositionLeft.y, 30, 16)), oColorL );

        const Point         oPositionRight( int(m_oContourRightOut.getData()[sliceN]), oY );
        std::stringstream   oTmpR;

        oTmpR << m_oContourRightOut.getRank()[sliceN];
        rLayerPosition.add<OverlayCross>( rTrafo(oPositionRight), oCrossRadius, oColorR );
        rLayerText.add<OverlayText>( oTmpR.str(), image::Font(), rTrafo(Rect(oPositionRight.x, oPositionRight.y, 30, 16)), oColorR );

        oY += oDeltaY;
    }
} // paint


bool SelectPeaksCont::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup)
{
    if (p_rPipe.tag() == "gradient_left")
    {
        m_pPipeInGradientLeft  = dynamic_cast< line_pipe_t* >(&p_rPipe);
    }
    else if (p_rPipe.tag() == "gradient_right")
    {
        m_pPipeInGradientRight  = dynamic_cast< line_pipe_t* >(&p_rPipe);
    }
    else if (p_rPipe.tag() == "max_filter_length")
    {
        m_pPipeInMaxFLenght = dynamic_cast< scalar_pipe_t* >(&p_rPipe);
    }
    else if (p_rPipe.tag() == "image_size")
    {
        m_pPipeInImgSize = dynamic_cast< scalar_pipe_t* >(&p_rPipe);
    }
    else if (p_rPipe.tag() == "default_seamWidth")
    {
        m_pPipeInDefaultSeamWidth = dynamic_cast< scalar_pipe_t* >(&p_rPipe);
    }
    else if (p_rPipe.tag() == "centerpos")
    {
        m_pPipeInCenterPos = dynamic_cast< scalar_pipe_t* >(&p_rPipe);
    }
    else if (p_rPipe.tag() == "startpos")
    {
        m_pPipeInStartPos = dynamic_cast< scalar_pipe_t* >(&p_rPipe);
    }

    return BaseFilter::subscribe( p_rPipe, p_oGroup );
} // subscribe


void SelectPeaksCont::proceedGroup(const void* sender, PipeGroupEventArgs& e)
{
    poco_assert_dbg(m_pPipeInGradientLeft != nullptr);      // to be asserted by graph editor
    poco_assert_dbg(m_pPipeInGradientRight != nullptr);     // to be asserted by graph editor
    poco_assert_dbg(m_pPipeInMaxFLenght != nullptr);        // to be asserted by graph editor
    poco_assert_dbg(m_pPipeInImgSize != nullptr);           // to be asserted by graph editor
    poco_assert_dbg(m_pPipeInDefaultSeamWidth != nullptr);  // to be asserted by graph editor
    poco_assert_dbg(m_pPipeInCenterPos != nullptr);         // to be asserted by graph editor
    poco_assert_dbg(m_pPipeInStartPos != nullptr);          // to be asserted by graph editor

    // input validity check

    poco_assert_dbg( ! m_pPipeInGradientLeft->read(m_oCounter).ref().empty() );
    poco_assert_dbg( ! m_pPipeInGradientRight->read(m_oCounter).ref().empty() );
    poco_assert_dbg( ! m_pPipeInMaxFLenght->read(m_oCounter).ref().getData().empty() );
    poco_assert_dbg( ! m_pPipeInImgSize->read(m_oCounter).ref().getData().empty() );
    poco_assert_dbg( ! m_pPipeInDefaultSeamWidth->read(m_oCounter).ref().getData().empty() );
    poco_assert_dbg( ! m_pPipeInCenterPos->read(m_oCounter).ref().getData().empty() );
    poco_assert_dbg( ! m_pPipeInStartPos->read(m_oCounter).ref().getData().empty() );

    // get data from frame

    const GeoVecDoublearray rGradientLeftIn     = m_pPipeInGradientLeft->read(m_oCounter);
    const GeoVecDoublearray rGradientRightIn    = m_pPipeInGradientRight->read(m_oCounter);
    const GeoDoublearray    rGeoImgSizeIn       = m_pPipeInImgSize->read(m_oCounter);
    const auto &            rImgSizeIn          = rGeoImgSizeIn.ref().getData();

    const bool oDefaultSeamWTooHigh	( [&, this]()->bool
    {
        if ( static_cast<unsigned int>(m_oDefaultSeamWidth) >= rGradientLeftIn.ref().front().size() )
        {
            wmLog(eWarning, "Filter 'selectPeaksCont': Default seam width is too high.\n");
            return true;
        }
        return false;
    } () );

    if (    inputIsInvalid( rGradientLeftIn )
         || inputIsInvalid( rGradientRightIn )
         || inputIsInvalid( rGeoImgSizeIn )
         || rGradientLeftIn.ref().size() != rGradientRightIn.ref().size()
         || oDefaultSeamWTooHigh
       )
    {
        const GeoDoublearray oGeoSeamPosLeftOut  ( rGradientLeftIn.context(),  m_oContourLeftOut,  rGradientLeftIn.analysisResult(),  0.0 ); // bad rank
        const GeoDoublearray oGeoSeamPosRightOut ( rGradientRightIn.context(), m_oContourRightOut, rGradientRightIn.analysisResult(), 0.0 ); // bad rank
        preSignalAction();
        m_oPipeOutContourLeft.signal( oGeoSeamPosLeftOut );
        m_oPipeOutContourRight.signal( oGeoSeamPosRightOut );

        return; // RETURN
    }

    // 'ImageSize' = size of grey ROI
    m_oImageSize.width  = int( rImgSizeIn[0] );
    m_oImageSize.height = int( rImgSizeIn[1] );
    m_oMaxFilterLenght  = int( m_pPipeInMaxFLenght->read(m_oCounter).ref().getData().front() );
    m_oDefaultSeamWidth = int( m_pPipeInDefaultSeamWidth->read(m_oCounter).ref().getData().front() );

    if ( inputIsInvalid( m_pPipeInImgSize->read(m_oCounter) ) )
    {
        // No valid data yet
        m_oCenterPosROI = 0;
    }
    else
    {
        // X pos inside grey ROI
        m_oCenterPosROI = int( m_pPipeInCenterPos->read(m_oCounter).ref().getData().front() );
    }
    if ( inputIsInvalid( m_pPipeInStartPos->read(m_oCounter) ) )
    {
        // No valid data yet
        m_oStartPos = 0;
    }
    else
    {
        // X pos inside whole image
        m_oStartPos = int( m_pPipeInStartPos->read(m_oCounter).ref().getData().front() );
    }

    // (re)initialization of output structure
    reinitialize( rGradientLeftIn.ref(), rGradientRightIn.ref() );

    // calculate the lines/stripes global x coordinate
    const int oLinesOffset  = rGradientLeftIn.context().trafo()->dx();

    // Check seam position status
    // Is it the first image of the seam?
    if (rGradientLeftIn.context().imageNumber() == 0)
    {
        // It's the first image  =>  check 'StartPos' and set 'expected' left/right seam position
        if (m_oStartPos == 0)
        {
            // Value is not set  =>  set dummy values for old seam positions in center of image
            m_oOldSeamPosLeftROI  = 0.5 * m_oImageSize.width - 0.5 * m_oDefaultSeamWidth;
            m_oOldSeamPosRightROI = 0.5 * m_oImageSize.width + 0.5 * m_oDefaultSeamWidth;
            m_oOldSeamPosLeft     = m_oOldSeamPosLeftROI  + oLinesOffset;
            m_oOldSeamPosRight    = m_oOldSeamPosRightROI + oLinesOffset;
            m_oCenterPosROI       = 0.5 * (m_oOldSeamPosLeftROI + m_oOldSeamPosRightROI);
        }
        else
        {
            m_oOldSeamPosLeft     = m_oStartPos - 0.5 * m_oDefaultSeamWidth;
            m_oOldSeamPosRight    = m_oStartPos + 0.5 * m_oDefaultSeamWidth;
            m_oOldSeamPosLeftROI  = m_oOldSeamPosLeft - oLinesOffset;
            m_oOldSeamPosRightROI = m_oOldSeamPosRight - oLinesOffset;
            m_oCenterPosROI       = m_oStartPos - oLinesOffset;
        }
    }
    else   // Not the first image of the seam
    {
        if (m_oCenterPosROI == 0)
        {
            // No valid 'center pos'  =>  set center of image
            m_oCenterPosROI = 0.5 * m_oImageSize.width;
        }

        m_oOldSeamPosLeftROI  = m_oCenterPosROI - 0.5 * m_oDefaultSeamWidth;
        m_oOldSeamPosRightROI = m_oCenterPosROI + 0.5 * m_oDefaultSeamWidth;
        m_oOldSeamPosLeft     = m_oOldSeamPosLeftROI  + oLinesOffset;
        m_oOldSeamPosRight    = m_oOldSeamPosRightROI + oLinesOffset;
    }

    // signal processing
    calcSelectPeaksCont(
        rGradientLeftIn.ref(),
        rGradientRightIn.ref(),
        m_oMaxFilterLenght,
        m_oDefaultSeamWidth,
        m_oThresholdLeft,
        m_oThresholdRight,
        m_oContourLeftOut,
        m_oContourRightOut
    );

    enforceIntegrity (m_oContourLeftOut, m_oContourRightOut, m_oImageSize.width, m_oDefaultSeamWidth); // validate integrity

    const double oNewRankLeft   = (rGradientLeftIn.rank()  + 1.0) * 0.5;   // full rank
    const double oNewRankRight  = (rGradientRightIn.rank() + 1.0) * 0.5;   // full rank
    const GeoDoublearray oGeoSeamPosLeftOut  ( rGradientLeftIn.context(),  m_oContourLeftOut,  rGradientLeftIn.analysisResult(),  oNewRankLeft );
    const GeoDoublearray oGeoSeamPosRightOut ( rGradientRightIn.context(), m_oContourRightOut, rGradientRightIn.analysisResult(), oNewRankRight );

    preSignalAction();
    m_oPipeOutContourLeft.signal( oGeoSeamPosLeftOut );
    m_oPipeOutContourRight.signal( oGeoSeamPosRightOut );

} // proceed


void SelectPeaksCont::reinitialize(
    const VecDoublearray & p_rGradientLeftIn,
    const VecDoublearray & p_rGradientRightIn )
{
    m_oContourLeftOut.assign ( p_rGradientLeftIn.size(),  0, eRankMin );   // (re)initialize output based on input dimension
    m_oContourRightOut.assign( p_rGradientRightIn.size(), 0, eRankMin );   // (re)initialize output based on input dimension
} // reinitialize


// actual signal processing
void SelectPeaksCont::calcSelectPeaksCont(
    const VecDoublearray    &p_rGradientLeftIn,
    const VecDoublearray    &p_rGradientRightIn,
    int                     p_oMaxFilterLenght,
    int                     p_oDefaultSeamWidth,
    int                     p_oThresholdLeft,
    int                     p_oThresholdRight,
    Doublearray             &p_rContourLeftOut,
    Doublearray             &p_rContourRightOut )
{

    // Parameter assertion. Should be pre-checked by UI / MMI / GUI.

    poco_assert_dbg( p_rGradientLeftIn.size() ==  p_rGradientRightIn.size() );
    poco_assert_dbg( ! p_rGradientLeftIn.empty() );
    poco_assert_dbg( p_oMaxFilterLenght > 0);

    const unsigned int oNProfiles               = p_rGradientLeftIn.size();
    const unsigned int oProfileSize             = p_rGradientLeftIn.front().getData().size();
    const unsigned int oMaxFilterLengthMinusOne = p_oMaxFilterLenght - 1;

    // Parameter assertion. Should be pre-checked by UI / MMI / GUI.
    poco_assert_dbg( static_cast<unsigned int>(p_oMaxFilterLenght) <= oProfileSize );

    // get references to data

    auto &rContourLeftOutData  = p_rContourLeftOut.getData();
    auto &rContourRightOutData = p_rContourRightOut.getData();

    for (unsigned int profileN = 0; profileN < oNProfiles; ++profileN)
    {
        // loop over N profiles

        // range without boundary values
        auto        oItGradL        = p_rGradientLeftIn[profileN].getData().begin()     + oMaxFilterLengthMinusOne;
        auto        oItGradR        = p_rGradientRightIn[profileN].getData().rbegin()   + oMaxFilterLengthMinusOne;   // Reverse !

        const auto  oItStartGradL   = p_rGradientLeftIn[profileN].getData().begin()     + oMaxFilterLengthMinusOne;
        const auto  oItStartGradR   = p_rGradientRightIn[profileN].getData().begin()    + oMaxFilterLengthMinusOne;
        const auto  oItEndGradL     = p_rGradientLeftIn[profileN].getData().end()       - oMaxFilterLengthMinusOne;

        vec_double_cit_t    oItAboveTholdL;
        vec_double_crit_t   oItAboveTholdR;   // Reverse !
        const int           oMaxInit    = 0;    // (re)initialisation value (small: 0 or std::limits::numeric_min)
        double              oMaxL       = oMaxInit;
        double              oMaxR       = oMaxInit;
        bool                oIsNewMaxL  = false;
        bool                oIsNewMaxR  = false;

        m_oPeaksL.clear();
        m_oPeaksR.clear();

        // same lenght for left and right asserted, therefore only one loop
        for (; oItGradL != oItEndGradL; ++oItGradL, ++oItGradR)
        {
            // Search only for maxima over the threshold!!

            // new left maximum if curr value bigger than threshold and bigger than old max and bigger than prev value (rising curve)
            if (    *oItGradL >= p_oThresholdLeft
                 && *oItGradL >  oMaxL
                 && *oItGradL >  *(oItGradL - 1)
               )
            {
                oItAboveTholdL  = oItGradL;
                oMaxL           = *oItGradL;
                oIsNewMaxL      = true;
            } // if

            // new right maximum if curr value bigger than threshold and bigger than old max and bigger than prev value (rising curve)
            if (    *oItGradR >= p_oThresholdRight
                 && *oItGradR >  oMaxR
                 && *oItGradR >  *(oItGradR - 1)
               )
            {
                oItAboveTholdR  = oItGradR;
                oMaxR           = *oItGradR;
                oIsNewMaxR      = true;
            } // if

            // save maxima if next value smaller (falling curve)

            if (oIsNewMaxL && *oItGradL < oMaxL)
            {
                // new max found and current value smaller (falling curve)
                m_oPeaksL.push_back(oItGradL - 1);   // save last max
                oMaxL       = oMaxInit;
                oIsNewMaxL  = false;
            } // if

            if (oIsNewMaxR && *oItGradR < oMaxR)
            {
                // new max found and current value smaller (falling curve)
                m_oPeaksR.push_back( oItGradR.base()/*- 1 is implicit*/ );   // get std iterator // save last max
                oMaxR       = oMaxInit;
                oIsNewMaxR  = false;
            } // if
        } // for

        auto oItPeaksL = m_oPeaksL.begin();
        auto oItPeaksR = m_oPeaksR.begin();

        vec_double_cit_t oItPeaksBestL;
        vec_double_cit_t oItPeaksBestR;

        const std::vector<vec_double_cit_t>::const_iterator oItPeaksLEnd = m_oPeaksL.end();
        const std::vector<vec_double_cit_t>::const_iterator oItPeaksREnd = m_oPeaksR.end();

        int oMinDiff = std::numeric_limits<int>::max();

        // Check the found left and right gradient maxima, which pair matches best

        // loop over left peaks
        for (oItPeaksL = m_oPeaksL.begin(); oItPeaksL != oItPeaksLEnd; ++oItPeaksL)
        {
            // loop over right peaks
            for (oItPeaksR = m_oPeaksR.begin(); oItPeaksR != oItPeaksREnd; ++oItPeaksR)
            {
                // Check left side peak with left 'expected' position
                const int oPeakExpDistanceLeft = m_oOldSeamPosLeftROI - std::distance(oItStartGradL, *oItPeaksL);

                // Check right side peak with right 'expected' position
                const int oPeakExpDistanceRight = m_oOldSeamPosRightROI - std::distance(oItStartGradR, *oItPeaksR);

                // Check distance between right and left side peak with 'expected' seam width
                const int oPeakDistance = std::distance(oItStartGradR, *oItPeaksR) - std::distance(oItStartGradL, *oItPeaksL);
                const int oDiffWidth    = oPeakDistance - p_oDefaultSeamWidth;

                // Sum the squared differences for a total sum for this pair.
                // The positions are double-weighted.
                const int oTotalSquareDiff =   2 * oPeakExpDistanceLeft * oPeakExpDistanceLeft
                                             + 2 * oPeakExpDistanceRight * oPeakExpDistanceRight
                                             + oDiffWidth * oDiffWidth;

                if ( oTotalSquareDiff < oMinDiff )
                {
                    oMinDiff = oTotalSquareDiff;
                    oItPeaksBestL = *oItPeaksL;
                    oItPeaksBestR = *oItPeaksR;
                }
            } // for
        } // for

        // get indices

        int oSeamPosLeft  = 0;
        int oSeamPosRight = 0;

        // Max. allowed difference of new found position to old position
        const int oMaxDeltaPosX = 10;

        if ( ! m_oPeaksL.empty() && ! m_oPeaksR.empty() )
        {
            oSeamPosLeft  = std::distance(oItStartGradL, oItPeaksBestL) + oMaxFilterLengthMinusOne;
            oSeamPosRight = std::distance(oItStartGradR, oItPeaksBestR) + oMaxFilterLengthMinusOne;
        }

        // Compare found left/right position with 'old' position

        if ( abs(oSeamPosLeft - m_oOldSeamPosLeftROI) > oMaxDeltaPosX )
        {
            // Difference too big, use 'old' position
            oSeamPosLeft = m_oOldSeamPosLeftROI;
        }

        if ( abs(oSeamPosRight - m_oOldSeamPosRightROI) > oMaxDeltaPosX )
        {
            // Difference too big, use 'old' position
            oSeamPosRight = m_oOldSeamPosRightROI;
        }

        // set result

        rContourLeftOutData[profileN]  = oSeamPosLeft;
        rContourRightOutData[profileN] = oSeamPosRight;

    } // for profileN

    // calculate rank
    calcRank(p_rContourLeftOut, p_rContourRightOut, oProfileSize, p_oMaxFilterLenght);

} // calcSelectPeaksCont


} // namespace filter
} // namespace precitec
