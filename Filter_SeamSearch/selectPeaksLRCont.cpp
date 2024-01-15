/*!
 *  @copyright      Precitec Vision GmbH & Co. KG
 *  @author         Urs Gisiger (GUR)
 *  @date           2022
 *  @file
 *  @brief          Fliplib filter 'SelectPeaksLRCont' in component 'Filter_SeamSearch'.
 *                  Copied from filter 'SelectPeaksCont'. Changed inpipe 'CenterPosition' to 'ExpLeftPosition'
 *                  and added inpipe 'ExpRightPosition'. Calculates right and left seam position, using from
 *                  the before image the found left seam position as 'ExpLeftPosition' and the found right seam
 *                  position as 'ExpRightPosition'.
 */


#include "selectPeaksLRCont.h"

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


const std::string SelectPeaksLRCont::m_oFilterName   = std::string("SelectPeaksLRCont");
const std::string SelectPeaksLRCont::m_oPipeOutName1 = std::string("ContourLeft");
const std::string SelectPeaksLRCont::m_oPipeOutName2 = std::string("ContourRight");


SelectPeaksLRCont::SelectPeaksLRCont() :
TransformFilter                ( m_oFilterName, Poco::UUID{"9948907f-f1d8-4e6e-8daa-6b55524f9399"} ),
    m_pPipeInGradientLeft      ( nullptr ),
    m_pPipeInGradientRight     ( nullptr ),
    m_pPipeInMaxFLenght        ( nullptr ),
    m_pPipeInImgSize           ( nullptr ),
    m_pPipeInDefaultSeamWidth  ( nullptr ),
    m_pPipeInExpLeftSeamPos    ( nullptr ),
    m_pPipeInExpRightSeamPos   ( nullptr ),
    m_pPipeInStartPos          ( nullptr ),
    m_oPipeOutContourLeft      ( this, m_oPipeOutName1 ),
    m_oPipeOutContourRight     ( this, m_oPipeOutName2 ),
    m_oMaxFilterLenght         ( 20 ),
    m_oDefaultSeamWidth        ( 220 ),
    m_oOldSeamPosLeftROI       ( 0 ),
    m_oOldSeamPosRightROI      ( 0 ),
    m_oStartPos                ( 0 ),
    m_oDisplayStripe           ( 0 ),
    m_oThresholdLeft           ( 4 ),
    m_oThresholdRight          ( 4 ),
    m_oMaxDeltaXPos            ( 10 )
{
    // Set default values for the parameters of the filter
    parameters_.add("DisplayStripe",    Parameter::TYPE_int, m_oDisplayStripe);
    parameters_.add("ThresholdLeft",    Parameter::TYPE_int, m_oThresholdLeft);
    parameters_.add("ThresholdRight",   Parameter::TYPE_int, m_oThresholdRight);
    parameters_.add("MaxDeltaXPos",     Parameter::TYPE_int, m_oMaxDeltaXPos);

    setInPipeConnectors({
        {Poco::UUID("5e1d9878-9b85-4c42-bc67-9203ba3e8d96"), m_pPipeInGradientLeft, "GradientLeft", 1, "gradient_left"},
        {Poco::UUID("fb8234c0-cfcd-4c59-bc72-a468c07314d2"), m_pPipeInGradientRight, "GradientRight", 1, "gradient_right"},
        {Poco::UUID("6c6f8971-5e06-4975-868d-c0e1a7d65676"), m_pPipeInMaxFLenght, "MaxFilterLength", 1, "max_filter_length"},
        {Poco::UUID("0bc58ad3-4b7a-48b2-8a90-ec596eb08e67"), m_pPipeInImgSize, "ImageSize", 1, "image_size"},
        {Poco::UUID("a5ed0ffc-7123-41f1-addf-d88284cbfeb6"), m_pPipeInDefaultSeamWidth, "DefaultSeamWidth", 1, "default_seamWidth"},
        {Poco::UUID("2cc91eaf-e569-4923-9c3c-62a1c54d4ac2"), m_pPipeInExpLeftSeamPos, "ExpLeftPosition", 1, "seam_pos_left"},
        {Poco::UUID("3da8c2e0-d745-463c-bc75-e6c61f4ec03f"), m_pPipeInExpRightSeamPos, "ExpRightPosition", 1, "seam_pos_right"},
        {Poco::UUID("5ebc9cd7-50bf-4382-94e7-f223e3546c22"), m_pPipeInStartPos, "StartPosition", 1, "startpos"}});
    setOutPipeConnectors({
        {Poco::UUID("0f148093-0324-4643-8589-d03f61adff95"), &m_oPipeOutContourLeft, m_oPipeOutName1, 0, ""},
        {Poco::UUID("a64811d0-466e-4ec0-bb75-448338dd77e9"), &m_oPipeOutContourRight, m_oPipeOutName2, 0, ""}});
    setVariantID(Poco::UUID("a9041006-4c35-440e-902e-5fb795994ebf"));
} // SelectPeaksLRCont


void SelectPeaksLRCont::setParameter()
{
    TransformFilter::setParameter();
    m_oDisplayStripe    = parameters_.getParameter("DisplayStripe").convert<int>();
    m_oThresholdLeft    = parameters_.getParameter("ThresholdLeft").convert<int>();
    m_oThresholdRight   = parameters_.getParameter("ThresholdRight").convert<int>();
    m_oMaxDeltaXPos     = parameters_.getParameter("MaxDeltaXPos").convert<int>();

    // Parameter assertion. Should be pre-checked by UI / MMI / GUI.
    poco_assert_dbg(m_oDefaultSeamWidth > 0);
} // setParameter


void SelectPeaksLRCont::paint()
{
    if (m_oVerbosity <= eNone || m_oContourLeftOut.size() == 0)
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

        const Color oColorL ( Color::Yellow() );
        const Color oColorR ( Color::Magenta() );

        const Point oPositionLeft ( int(m_oContourLeftOut.getData()[sliceN]), oY );
        rLayerPosition.add<OverlayCross> ( rTrafo(oPositionLeft), oCrossRadius, oColorL );

        const Point oPositionRight( int(m_oContourRightOut.getData()[sliceN]), oY );
        rLayerPosition.add<OverlayCross> ( rTrafo(oPositionRight), oCrossRadius, oColorR );

        if (m_oVerbosity >= eMedium)
        {
            std::stringstream oTmpL;
            oTmpL << m_oContourLeftOut.getRank()[sliceN];
            rLayerText.add<OverlayText>( oTmpL.str(), image::Font(), rTrafo(Rect(oPositionLeft.x, oPositionLeft.y, 30, 16)), oColorL );

            std::stringstream oTmpR;
            oTmpR << m_oContourRightOut.getRank()[sliceN];
            rLayerText.add<OverlayText>( oTmpR.str(), image::Font(), rTrafo(Rect(oPositionRight.x, oPositionRight.y, 30, 16)), oColorR );
        }

        oY += oDeltaY;
    }
} // paint


bool SelectPeaksLRCont::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup)
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
    else if (p_rPipe.tag() == "seam_pos_left")
    {
        m_pPipeInExpLeftSeamPos = dynamic_cast< scalar_pipe_t* >(&p_rPipe);
    }
    else if (p_rPipe.tag() == "seam_pos_right")
    {
        m_pPipeInExpRightSeamPos = dynamic_cast< scalar_pipe_t* >(&p_rPipe);
    }
    else if (p_rPipe.tag() == "startpos")
    {
        m_pPipeInStartPos = dynamic_cast< scalar_pipe_t* >(&p_rPipe);
    }

    return BaseFilter::subscribe( p_rPipe, p_oGroup );
} // subscribe


void SelectPeaksLRCont::proceedGroup(const void* sender, PipeGroupEventArgs& e) {

    poco_assert_dbg(m_pPipeInGradientLeft != nullptr); // to be asserted by graph editor
    poco_assert_dbg(m_pPipeInGradientRight != nullptr); // to be asserted by graph editor
    poco_assert_dbg(m_pPipeInMaxFLenght != nullptr); // to be asserted by graph editor
    poco_assert_dbg(m_pPipeInImgSize != nullptr); // to be asserted by graph editor
    poco_assert_dbg(m_pPipeInDefaultSeamWidth != nullptr); // to be asserted by graph editor
    poco_assert_dbg(m_pPipeInExpLeftSeamPos != nullptr); // to be asserted by graph editor
    poco_assert_dbg(m_pPipeInExpRightSeamPos != nullptr); // to be asserted by graph editor
    poco_assert_dbg(m_pPipeInStartPos != nullptr); // to be asserted by graph editor

    // input validity check

    poco_assert_dbg( ! m_pPipeInGradientLeft->read(m_oCounter).ref().empty() );
    poco_assert_dbg( ! m_pPipeInGradientRight->read(m_oCounter).ref().empty() );
    poco_assert_dbg( ! m_pPipeInMaxFLenght->read(m_oCounter).ref().getData().empty() );
    poco_assert_dbg( ! m_pPipeInImgSize->read(m_oCounter).ref().getData().empty() );
    poco_assert_dbg( ! m_pPipeInDefaultSeamWidth->read(m_oCounter).ref().getData().empty() );
    poco_assert_dbg( ! m_pPipeInExpLeftSeamPos->read(m_oCounter).ref().getData().empty() );
    poco_assert_dbg( ! m_pPipeInExpRightSeamPos->read(m_oCounter).ref().getData().empty() );
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
            wmLog(eWarning, "Filter 'selectPeaksLRCont': Default seam width is too big.\n");
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
        m_oOldSeamPosLeftROI  = 0;
        m_oOldSeamPosRightROI = 0;
    }
    else
    {
        if (    inputIsInvalid( m_pPipeInExpLeftSeamPos->read(m_oCounter) )
             || inputIsInvalid( m_pPipeInExpLeftSeamPos->read(m_oCounter) )
           )
        {
            // No valid data yet
            m_oOldSeamPosLeftROI  = 0;
            m_oOldSeamPosRightROI = 0;
        }
        else
        {
            // X pos inside grey ROI
            m_oOldSeamPosLeftROI  = int( m_pPipeInExpLeftSeamPos->read(m_oCounter).ref().getData().front() );
            m_oOldSeamPosRightROI = int( m_pPipeInExpRightSeamPos->read(m_oCounter).ref().getData().front() );
        }
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

    // calculate the lines/stripes global x/y coordinate
    const int oXRoiOffset  = rGradientLeftIn.context().trafo()->dx();
    const int oYRoiOffset  = rGradientLeftIn.context().trafo()->dy();

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
            m_oOldSeamPosLeft     = m_oOldSeamPosLeftROI  + oXRoiOffset;
            m_oOldSeamPosRight    = m_oOldSeamPosRightROI + oXRoiOffset;
        }
        else
        {
            m_oOldSeamPosLeft     = m_oStartPos - 0.5 * m_oDefaultSeamWidth;
            m_oOldSeamPosRight    = m_oStartPos + 0.5 * m_oDefaultSeamWidth;
            m_oOldSeamPosLeftROI  = m_oOldSeamPosLeft - oXRoiOffset;
            m_oOldSeamPosRightROI = m_oOldSeamPosRight - oXRoiOffset;
        }
    }
    else   // Not the first image of the seam
    {
        if (    (m_oOldSeamPosLeftROI  == 0)
             || (m_oOldSeamPosRightROI == 0)
           )
        {
            // No valid 'old seam pos'

            const GeoDoublearray oGeoSeamPosLeftOut  ( rGradientLeftIn.context(),  m_oContourLeftOut,  rGradientLeftIn.analysisResult(),  0.0 ); // bad rank
            const GeoDoublearray oGeoSeamPosRightOut ( rGradientRightIn.context(), m_oContourRightOut, rGradientRightIn.analysisResult(), 0.0 ); // bad rank
            preSignalAction();
            m_oPipeOutContourLeft.signal( oGeoSeamPosLeftOut );
            m_oPipeOutContourRight.signal( oGeoSeamPosRightOut );

            return; // RETURN
        }

        m_oOldSeamPosLeft  = m_oOldSeamPosLeftROI  + oXRoiOffset;
        m_oOldSeamPosRight = m_oOldSeamPosRightROI + oXRoiOffset;
    }

    // signal processing
    calcSelectPeaksLRCont(
        rGradientLeftIn.ref(),
        rGradientRightIn.ref(),
        oXRoiOffset,
        oYRoiOffset,
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


void SelectPeaksLRCont::reinitialize(
    const VecDoublearray & p_rGradientLeftIn,
    const VecDoublearray & p_rGradientRightIn )
{
    m_oContourLeftOut.assign ( p_rGradientLeftIn.size(),  0, eRankMin );   // (re)initialize output based on input dimension
    m_oContourRightOut.assign( p_rGradientRightIn.size(), 0, eRankMin );   // (re)initialize output based on input dimension
} // reinitialize


// actual signal processing
void SelectPeaksLRCont::calcSelectPeaksLRCont(
    const VecDoublearray    &p_rGradientLeftIn,
    const VecDoublearray    &p_rGradientRightIn,
    const int               p_oXRoiOffset,
    const int               p_oYRoiOffset,
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
        auto        oItGradR        = p_rGradientRightIn[profileN].getData().begin()    + oMaxFilterLengthMinusOne;

        const auto  oItStartGradL   = p_rGradientLeftIn[profileN].getData().begin()     + oMaxFilterLengthMinusOne;
        const auto  oItStartGradR   = p_rGradientRightIn[profileN].getData().begin()    + oMaxFilterLengthMinusOne;
        const auto  oItEndGradL     = p_rGradientLeftIn[profileN].getData().end()       - oMaxFilterLengthMinusOne;

        vec_double_cit_t    oItAboveTholdL;
        vec_double_cit_t    oItAboveTholdR;
        const int           oMaxInit    = 0;    // (re)initialisation value (small: 0 or std::limits::numeric_min)
        double              oMaxL       = oMaxInit;
        double              oMaxR       = oMaxInit;
        bool                oIsNewMaxL  = false;
        bool                oIsNewMaxR  = false;

        m_oPeaksL.clear();
        m_oPeaksR.clear();

        // Variables for graphical display output

        OverlayCanvas & rOverlayCanvas  ( canvas<OverlayCanvas>(m_oCounter) );
        OverlayLayer &  rLayerPosition  ( rOverlayCanvas.getLayerPosition() );
        int xCurrentROI = -1;
        int xLMaxROI = -1;
        int xRMaxROI = -1;

        // same lenght for left and right asserted, therefore only one loop
        for (; oItGradL != oItEndGradL; ++oItGradL, ++oItGradR)
        {
            // Search only for maxima over the threshold!!

            xCurrentROI++;

            // new left maximum if curr value bigger than threshold and bigger than old max and bigger than prev value (rising curve)
            if (    *oItGradL >= p_oThresholdLeft
                 && *oItGradL >  oMaxL
                 && *oItGradL >  *(oItGradL - 1)
               )
            {
                oItAboveTholdL  = oItGradL;
                oMaxL           = *oItGradL;
                xLMaxROI        = xCurrentROI;
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
                xRMaxROI        = xCurrentROI;
                oIsNewMaxR      = true;
            } // if

            // save maxima if next value smaller (falling curve)

            if (oIsNewMaxL && *oItGradL < oMaxL)
            {
                if  (    ( m_oVerbosity >= eHigh )
                      && ( (unsigned int)m_oDisplayStripe == (profileN + 1) )
                    )
                {
                    // Paint a vertical marking line on upper border of ROI
                    rLayerPosition.add<OverlayLine>( Point(xCurrentROI + p_oXRoiOffset + oMaxFilterLengthMinusOne, 0     + p_oYRoiOffset),
                                                     Point(xCurrentROI + p_oXRoiOffset + oMaxFilterLengthMinusOne, oMaxL + p_oYRoiOffset),
                                                     Color::Yellow());

                    if  (m_oVerbosity >= eMax)
                    {
                        wmLog(eDebug, "Str %d: LMax %f at xpos %d in ROI \n", profileN + 1, oMaxL, xLMaxROI );
                    }
                }

                // new max found and current value smaller (falling curve)
                m_oPeaksL.push_back(oItGradL - 1);   // save last max
                oMaxL       = oMaxInit;
                oIsNewMaxL  = false;
            } // if

            if (oIsNewMaxR && *oItGradR < oMaxR)
            {
                if  (    ( m_oVerbosity >= eHigh )
                      && ( (unsigned int)m_oDisplayStripe == (profileN + 1) )
                    )
                {
                    // Paint a vertical marking line on upper border of ROI
                    rLayerPosition.add<OverlayLine>( Point(xCurrentROI + p_oXRoiOffset + oMaxFilterLengthMinusOne, 0     + p_oYRoiOffset),
                                                     Point(xCurrentROI + p_oXRoiOffset + oMaxFilterLengthMinusOne, oMaxR + p_oYRoiOffset),
                                                     Color::Blue());

                    if  (m_oVerbosity >= eMax)
                    {
                        wmLog(eDebug, "Str %d: RMax %f at xpos %d in ROI \n", profileN + 1, oMaxR, xRMaxROI );
                    }
                }

                // new max found and current value smaller (falling curve)
                m_oPeaksR.push_back( oItGradR - 1 );   // save last max
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

        // Counter for left peaks
        int PeakL = 0;

        // loop over left peaks
        for (oItPeaksL = m_oPeaksL.begin(); oItPeaksL != oItPeaksLEnd; ++oItPeaksL)
        {
            PeakL++;

            // Counter for right peaks
            int PeakR = 0;

            // loop over right peaks
            for (oItPeaksR = m_oPeaksR.begin(); oItPeaksR != oItPeaksREnd; ++oItPeaksR)
            {
                PeakR++;

                // Check left side peak with left 'expected' position
                const int oPeakExpDistanceLeft = std::abs( m_oOldSeamPosLeftROI - std::distance(oItStartGradL, *oItPeaksL) );

                // Check right side peak with right 'expected' position
                const int oPeakExpDistanceRight = std::abs( m_oOldSeamPosRightROI - std::distance(oItStartGradR, *oItPeaksR) );

                // Check distance between right and left side peak with 'expected' seam width
                const int oPeakDistance = std::distance(oItStartGradR, *oItPeaksR) - std::distance(oItStartGradL, *oItPeaksL);
                const int oDiffWidth    = abs(oPeakDistance - p_oDefaultSeamWidth);

                // Sum the differences for a total sum for this pair.
                // Use size of peaks as additional part for total sum.
                const double dWeightFactor = 0.3;

                // Calc temp sum for left peak
                int dTempSum = oPeakExpDistanceLeft - ( dWeightFactor * (**oItPeaksL) );
                int oTotalDiffSum = dTempSum;

                // Calc temp sum for right peak
                dTempSum = oPeakExpDistanceRight - ( dWeightFactor * (**oItPeaksR) );
                oTotalDiffSum += dTempSum;

                // Calc temp sum for width
                dTempSum = oDiffWidth;
                oTotalDiffSum += dTempSum;

                if  (    (m_oVerbosity >= eMax)
                      && ( (unsigned int)m_oDisplayStripe == (profileN + 1) )
                    )
                {
                    wmLog(eDebug, "CheckStr %d: L/R %d %d dPeak %d = R %d - L %d \n",
                        profileN + 1, PeakL, PeakR, oPeakDistance, std::distance(oItStartGradR, *oItPeaksR), std::distance(oItStartGradL, *oItPeaksL) );
                }

                if ( oTotalDiffSum < oMinDiff )
                {
                    oMinDiff = oTotalDiffSum;
                    oItPeaksBestL = *oItPeaksL;
                    oItPeaksBestR = *oItPeaksR;

                    if  (    (m_oVerbosity >= eMax)
                        && ( (unsigned int)m_oDisplayStripe == (profileN + 1) )
                        )
                    {
                        wmLog(eDebug, "CheckStr %d: new min %d for L %d R %d \n", profileN + 1, oMinDiff, PeakL, PeakR );
                    }
                }
            } // for
        } // for

        // get indices

        int oSeamPosLeft  = 0;
        int oSeamPosRight = 0;

        if ( ! m_oPeaksL.empty() && ! m_oPeaksR.empty() )
        {
            oSeamPosLeft  = std::distance(oItStartGradL, oItPeaksBestL) + oMaxFilterLengthMinusOne;
            oSeamPosRight = std::distance(oItStartGradR, oItPeaksBestR) + oMaxFilterLengthMinusOne;
        }

        if  (    (m_oVerbosity >= eMax)
                && ( (unsigned int)m_oDisplayStripe == (profileN + 1) )
            )
        {
            wmLog(eInfo, "CheckStr %d: Result peaks posL %d posR %d \n", profileN + 1, oSeamPosLeft, oSeamPosRight );
        }

        // Compare found left/right position with 'old' position

        if ( abs(oSeamPosLeft - m_oOldSeamPosLeftROI) > m_oMaxDeltaXPos )
        {
            // Difference too big, use 'old' position +/- max allowed distance
            if (oSeamPosLeft > m_oOldSeamPosLeftROI)
            {
                oSeamPosLeft = m_oOldSeamPosLeftROI + m_oMaxDeltaXPos;
            }
            else
            {
                oSeamPosLeft = m_oOldSeamPosLeftROI - m_oMaxDeltaXPos;
            }
        }

        if ( abs(oSeamPosRight - m_oOldSeamPosRightROI) > m_oMaxDeltaXPos )
        {
            // Difference too big, use 'old' position +/- max allowed distance
            if (oSeamPosRight > m_oOldSeamPosRightROI)
            {
                oSeamPosRight = m_oOldSeamPosRightROI + m_oMaxDeltaXPos;
            }
            else
            {
                oSeamPosRight = m_oOldSeamPosRightROI - m_oMaxDeltaXPos;
            }
        }

        // set result

        rContourLeftOutData[profileN]  = oSeamPosLeft;
        rContourRightOutData[profileN] = oSeamPosRight;

    } // for profileN

    // calculate rank
    calcRank(p_rContourLeftOut, p_rContourRightOut, oProfileSize, p_oMaxFilterLenght);

} // calcSelectPeaksLRCont


} // namespace filter
} // namespace precitec
