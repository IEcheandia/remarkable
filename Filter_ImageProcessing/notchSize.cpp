#include "notchSize.h"

#include "filter/algoArray.h"
#include <overlay/overlayLayer.h>
#include <overlay/overlayCanvas.h>
#include <overlay/overlayPrimitive.h>
#include "util/calibDataSingleton.h"
#include <fliplib/TypeToDataTypeImpl.h>

namespace precitec
{
namespace filter
{

namespace start_end_detection
{
    using namespace precitec::image;
    using precitec::geo2d::StartEndInfo;
    using precitec::geo2d::Point;

void NotchSizeCalculator::reset()
{
    mInitialized = false;
    mComputed = false;
    m_errorMessage.clear();
}


bool NotchSizeCalculator::init(const BImage & rImage, int liNaht, int reNaht,
    int LastStripeOnTubeIndex, const StripePositioning & rStripePositioning,
    BackgroundPosition backgroundPosition
     )
{
    mInitialized = false;
    mBackgroundPosition = backgroundPosition;
    mLiNaht = liNaht;
    mReNaht = reNaht;
    assert(false && "Not implemented");
    int MittelwertLi = 0;
    int MittelwertRe = 0;
    double yAchsenAbschnittLi = 0.0;
    double yAchsenAbschnittRe = 0.0;
 /*
 //ID1 iLastStripeOnTube_ - 2
 //ID2 iLastStripeOnTube_ + 1
    EdgeFitResult leftEdgeFitResult = searchEdge(Side.left, backgroundOnTop, liNaht, reNaht, iDistImgBorderX, pInput);
    if !(leftEdgeFitResult.valid)
    {
    return;
    }
    // Jetzt Streifen erneut testen, wann der Mittelwert verlassen wird
    double yAchsenAbschnittLi = fitLineEdge(leftEdgeFitResult);
    mState = State::foundleftEdge;

    // Dann die rechte Seite = rechts von der Naht
    // -------------------------------------------

    EdgeFitResult rightEdgeFitResult = searchEdge(Side.right, backgroundOnTop, liNaht, reNaht, iDistImgBorderX, pInput);
    if !(rightEdgeFitResult.valid)
    {
        return;
    }
    // Jetzt Streifen erneut testen, wann der Mittelwert verlassen wird

    double yAchsenAbschnittRe = fitLineEdge(rightEdgeFitResult);

    // Sind beide Kanten gefunden worden?
    // ----------------------------------

    if ( (yAchsenAbschnittLi + yAchsenAbschnittRe) >= 10000	)
    {
        // Eine oder beide Kanten nicht gefunden!
        if (pInput->isDebug_)
            printf("Min. eine Kante nicht gefunden! (%s)\n", backgroundOnTop ? "VU" : "VO");
        return false;
    }
    mState = State::foundRightEdge;
    if (!foundEdge())
        return ;
    MittelwertLi = leftEdgeFitResult.Mittelwert;
    MittelwertRe = rightEdgeFitResult.Mittelwert;
*/
    mLeftEdge.m = rStripePositioning.viewParameters().m_stripe_inclination;
    mRightEdge.m = rStripePositioning.viewParameters().m_stripe_inclination;
    mLeftEdge.q = yAchsenAbschnittLi;
    mRightEdge.q = yAchsenAbschnittRe;
    mThresholdIntensityNotchInWindow = (MittelwertLi + MittelwertRe) * (NOTCH_WINDOW_H * NOTCH_WINDOW_W)/2;

    mInitialized = initializeStart(rImage);
    return mInitialized;
}


bool NotchSizeCalculator::init(const BImage & rImage, int liNaht, int reNaht, int threshTube,
    geo2d::StartEndInfo::FittedLine lineEdgeLeft, geo2d::StartEndInfo::FittedLine lineEdgeRight,
    BackgroundPosition backgroundPosition)
{
    mInitialized = false;
    mBackgroundPosition = backgroundPosition;
    mLiNaht = liNaht;
    mReNaht = reNaht;
    mThresholdIntensityNotchInWindow = threshTube * (NOTCH_WINDOW_H * NOTCH_WINDOW_W);
    if ( lineEdgeLeft.m != lineEdgeRight.m )
    {
        m_errorMessage = "Non-parallel left and right edge not supported";
        return false;
    }
    mLeftEdge.m = lineEdgeLeft.m;
    mLeftEdge.q = lineEdgeLeft.q;
    mRightEdge.m = lineEdgeRight.m;
    mRightEdge.q = lineEdgeRight.q;
    mInitialized = initializeStart(rImage);
    return mInitialized;

}

bool NotchSizeCalculator::initializeStart(const image::BImage & rImage)
{
    mNotchPoints.clear();
    mNotchParabola.Reset();
    mComputed = false;

    assert(mLeftEdge.m == mRightEdge.m);
    mMeanEdge.m = mLeftEdge.m;
    mMeanEdge.q = (mLeftEdge.q + mRightEdge.q) / 2.0;

    auto yLeft = mMeanEdge.getY(mLiNaht);
    auto yRight = mMeanEdge.getY(mReNaht);

    bool backgroundOnTop = mBackgroundPosition == BackgroundPosition::Top;
    mYStart = backgroundOnTop ? std::min(yLeft, yRight) : std::max(yLeft, yRight);

    int minValidKerbenStartY = backgroundOnTop ? 5 : 51;
    int maxValidKerbenStartY =  rImage.height() - int( backgroundOnTop ?  50 : 6);

    //at least 5 pixel from the bg border or 50 from the material border
    for ( auto & y : {mYStart, yLeft, yRight} )
    {
        if ( (y < minValidKerbenStartY) || (y > maxValidKerbenStartY) )
        {
            //printf("Keine Kerbenvermessung, zu dicht am Rand (%d Pixel)! (VU)\n", KerbeYStart);
            m_errorMessage = "Edge too close at image border (mean edge: " + std::to_string(y)
                + " accepted range: " + std::to_string(minValidKerbenStartY) + " " + std::to_string(maxValidKerbenStartY) +
                ")";
            return false;
        }
    }

    if ( (mReNaht - mLiNaht) < 8 )
    {
        // printf("Keine Kerbenvermessung, Naht zu schmal = %d Pixel\n", reNaht - liNaht);
        m_errorMessage = "Seam too narrow (" + std::to_string(mLiNaht) + " , " + std::to_string(mReNaht) + ")";
        return false;
    }
    return true;
}

bool NotchSizeCalculator::calc(const image::BImage & rImage)
{
    switch ( mBackgroundPosition )
    {
        case BackgroundPosition::Top:
            return calc<BackgroundPosition::Top>(rImage);
        case BackgroundPosition::Bottom:
            return calc<BackgroundPosition::Bottom>(rImage);
    }
    assert(false && "not reachable");
    return false;
}



template<BackgroundPosition tBackgroundPosition>
bool NotchSizeCalculator::calc(const BImage & rImage)
{
    assert(tBackgroundPosition == mBackgroundPosition);

    auto fInnermost = [] (double y1, double y2)
    {
        return (tBackgroundPosition == BackgroundPosition::Top) ? std::max(y1, y2) : std::min(y1, y2);
    };
    auto fOutermost = [] (double y1, double y2)
    {
        return (tBackgroundPosition == BackgroundPosition::Top) ? std::min(y1, y2) : std::max(y1, y2);
    };
    auto fIsInnermost = [] (double y1, double yRef)
    {
        return (tBackgroundPosition == BackgroundPosition::Top) ? y1 > yRef : y1 < yRef;
    };


    mComputed = false;
    if ( !mInitialized )
    {
        m_errorMessage = "Calc method called without initialization";
        return false;
    }

    searchNotchPoints<tBackgroundPosition>(mNotchPoints, rImage);
    if ( mNotchPoints.size() == 0 )
    {
        m_errorMessage = "No points found (x range " + std::to_string(mLiNaht) + " " + std::to_string(mReNaht)
            + ", threshold " + std::to_string(mThresholdIntensityNotchInWindow / (NOTCH_WINDOW_H*NOTCH_WINDOW_W)) + ")";
        return false;
    }
    mFittedNotch = fitNotchParabola(mNotchPoints, mNotchParabola);
    auto oFitXBounds = getFitXBounds();
    if ( oFitXBounds.first == -1 || oFitXBounds.second ==-1 )
    {
        m_errorMessage = "No points found for parabola fitting";
        return false;
    }
    int & x_left = oFitXBounds.first;
    int & x_right = oFitXBounds.second;

    switch ( mReferencePosition )
    {
        case ReferencePosition::BetweenEdges:
            mYReference = mMeanEdge.getY((x_left + x_right) / 2.0);
            break;
        case ReferencePosition::InternalEdge:
            mYReference = fInnermost(mLeftEdge.getY(mLiNaht), mRightEdge.getY(mReNaht));
            break;
        case ReferencePosition::ExternalEdge:
            mYReference = fOutermost(mLeftEdge.getY(mLiNaht), mRightEdge.getY(mReNaht));
            break;
    }

    assert(mYReference >= 0 && mYReference < rImage.height());

    //find innermost y
    mIndentation.y = tBackgroundPosition == BackgroundPosition::Top ? 0.0 : 10000.0;

    for ( int x = x_left; x <= x_right; x++ )
    {
        double FktWert = mFittedNotch.a * x *x + mFittedNotch.b * x + mFittedNotch.c;
        if ( fIsInnermost(FktWert, mIndentation.y) )
        {
            mIndentation.x = x;
            mIndentation.y = FktWert;
            assert(FktWert >= 0 && FktWert < rImage.height());
        }
    }
    mComputed = true;
    return true;
}

template<BackgroundPosition tBackgroundPosition>
void NotchSizeCalculator::searchNotchPoints(std::vector<geo2d::Point> & rPoints, const image::BImage & rImage) const
{
    assert(mBackgroundPosition == tBackgroundPosition);
    assert(mReNaht - mLiNaht > NOTCH_WINDOW_W && "input should have been checked in initializeStart");

    rPoints.clear();
    rPoints.reserve(mReNaht - mLiNaht - NOTCH_WINDOW_W);

    int lastY = tBackgroundPosition == BackgroundPosition::Top ? (rImage.height() - HEIGHT_SEARCH_NOTCH) : HEIGHT_SEARCH_NOTCH;

    // Helligkeits-Bestimmung innerhalb der Naht, am Rohr-Anfang (backgroundOnTop) / Ende (!backgroundOnTop)
    for ( int x = mLiNaht + 2; x <= mReNaht - 2; x++ )
    {
        // backgroundOnTop: Von oben nach unten: // !backgroundOnTop: Von unten nach oben
        // wo ist Uebergang Hintergrund -> Rohr ?
        for ( int y = static_cast<int>(mYStart+0.5);
            tBackgroundPosition == BackgroundPosition::Top ? y < lastY : y > lastY;
            tBackgroundPosition == BackgroundPosition::Top ? y++ : y-- )
        {
            int nextY = tBackgroundPosition == BackgroundPosition::Top ? y + 1 : y - 1;
            auto pCurrentRow = rImage.rowBegin(y);
            auto pNextRow = rImage.rowBegin(nextY);
            int gwTotal = pCurrentRow[x - 2]  + pNextRow[x - 2]
                        + pCurrentRow[x - 1]  + pNextRow[x - 1]
                        + pCurrentRow[x]      + pNextRow[x]
                        + pCurrentRow[x + 1]  + pNextRow[x + 1]
                        + pCurrentRow[x + 2]  + pNextRow[x + 2];

            //assert(gwTotal = calcSum(x-2, backgroundOnTop ? y :y y-1, 5, 2));

            if ( gwTotal > mThresholdIntensityNotchInWindow )
            {
                // Uebergang gefunden! found the start of material in this column
                rPoints.push_back({x, y});
                break;
            }
        } // for y
    }  // for x
}

NotchSizeCalculator::FittedParabola NotchSizeCalculator::fitNotchParabola(const std::vector<geo2d::Point> & rPoints, precitec::math::ParabolaEquation & rParabola)
{
    rParabola.Reset();
    assert(rPoints.size() > 0);
    // Einschub: Punkte nochmal bearbeiten, dann erst Parabel berechnen

    int links = 0;
    int rechts = (int)rPoints.size() - 1;

    int liWert, reWert;
    int swing = 0;

    int diffLinks = 0;
    int diffRechts = 0;

    //choose 70 points in the middle , avoid those lying on mMeanEdge
    while ( (rechts - links) > 71 )
    {
        if ( swing > 7 )
        {
            //  +7  "straight" right points, don't get stuck here and advance left index
            swing = 0;
            links++;
            diffLinks++;
            continue;
        }
        else if ( swing < -7 )
        {
            //  +7  "straight" left points, don't get stuck here and advance right index
            swing = 0;
            rechts--;
            diffRechts++;
            continue;
        }

        //difference between fittedY and actual Y
        liWert = abs((int) (mMeanEdge.getY(rPoints[links].x) - rPoints[links].y + 0.5));
        reWert = abs((int) (mMeanEdge.getY(rPoints[rechts].x) - rPoints[rechts].y + 0.5));

        if ( liWert < reWert )
        {
            swing--;
            links++; //advance left index
            diffLinks++;
        }
        else
        {
            swing++;
            rechts--; //advance right index
            diffRechts++;
        }
    }

    assert(links == diffLinks && rechts == int(rPoints.size()) - 1 - diffRechts);


    FittedParabola result;
    result.index_left = links;
    result.index_right = rechts;

    for ( int i = result.index_left; i <= result.index_right; i++ )
    {
        rParabola.AddPoint(rPoints[i].x, rPoints[i].y);
    }

    rParabola.CalcABC(result.a, result.b, result.c);
    return result;

}



std::pair< double, bool > NotchSizeCalculator::getNotchSize() const
{
    if ( !mInitialized || !mComputed )
    {
        return{0.0, false};
    }

    double yDifference = mBackgroundPosition == BackgroundPosition::Top ? mIndentation.y - mYReference
                                                                        : mYReference - mIndentation.y;
    if ( yDifference < 0 )
    {
        yDifference = 0.0;
    }
    return{yDifference, true};
}


std::pair< int, int > NotchSizeCalculator::getFitXBounds() const
{
    int  numPoints = static_cast<int>( mNotchPoints.size());
    if ( !mInitialized || numPoints == 0 )
    {
        return{-1,-1};
    }
    if ( mFittedNotch.index_left < 0 || mFittedNotch.index_left >= numPoints
            || mFittedNotch.index_right < 0 || mFittedNotch.index_right >= numPoints )
    {
        return {-1, -1,};
    }
    return{mNotchPoints[mFittedNotch.index_left].x, mNotchPoints[mFittedNotch.index_right].x};
}


double NotchSizeCalculator::getYReference() const
{
    return mYReference;
}


geo2d::DPoint NotchSizeCalculator::getIndentedPoint() const
{
    return mIndentation;
}


const std::string& NotchSizeCalculator::viewErrorMessage() const
{
    return m_errorMessage;
}

}
}
}


namespace precitec
{
namespace filter
{

using namespace fliplib;
using start_end_detection::NotchSizeCalculator;
using start_end_detection::BackgroundPosition;
using start_end_detection::ImageValidRange;
using geo2d::StartEndInfo;

/*static*/ const std::string NotchSize::m_oFilterName = "NotchSize";

NotchSize::NotchSize()
:TransformFilter(m_oFilterName, Poco::UUID{"54c9455e-9039-4666-a5ba-f4b776c2be28"}),
m_pPipeInImageFrame(nullptr),
m_pPipeInStartEndInfo(nullptr),
m_pPipeInXSeamLeft(nullptr),
m_pPipeInXSeamRight(nullptr),
m_oPipeOutNotchSizeStart(this, "NotchSizeStart"),
m_oPipeOutNotchSizeEnd(this, "NotchSizeEnd"),
m_oThresholdSeam(50)
{
    m_oNotchSizeCalculator.mReferencePosition = NotchSizeCalculator::ReferencePosition::InternalEdge;
    parameters_.add("Reference", Parameter::TYPE_int, int(m_oNotchSizeCalculator.mReferencePosition));
    parameters_.add("ThresholdSeam", Parameter::TYPE_int, m_oThresholdSeam);

    setInPipeConnectors({{Poco::UUID("2C3DB0B4-CFBE-472A-8587-611F56A62095"), m_pPipeInImageFrame, "image", 1, "image"},
    {Poco::UUID("1339E843-5C37-4DFE-9B6C-6707E2E2A244"), m_pPipeInStartEndInfo, "startendinfo", 1, "startendinfo"},
    {Poco::UUID("0F20EE2B-88E5-42B8-9037-4857640392CE"), m_pPipeInXSeamLeft, "xseamleft", 1, "xseamleft"},
    {Poco::UUID("7D2DEF03-C3FE-481E-9CD1-23642A89712F"), m_pPipeInXSeamRight, "xseamright", 1, "xseamright"}});
    setOutPipeConnectors({{Poco::UUID("05BC50A3-E30E-4A2E-A1E1-6E3616D48B66"), &m_oPipeOutNotchSizeStart, "NotchSizeStart", 0, "NotchSizeStart"},
    {Poco::UUID("CABFB697-B051-4241-BF6C-8A35D40FD15F"), &m_oPipeOutNotchSizeEnd, "NotchSizeEnd", 0, "NotchSizeEnd"}});
    setVariantID(Poco::UUID("8392d81d-5fea-4377-ae19-2551eef8b219"));
}

void NotchSize::setParameter()
{
    TransformFilter::setParameter();
    int oReference = parameters_.getParameter("Reference");
    m_oNotchSizeCalculator.mReferencePosition = static_cast<NotchSizeCalculator::ReferencePosition>(oReference);
    m_oThresholdSeam = parameters_.getParameter("ThresholdSeam");
}

bool NotchSize::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup)
{
    bool found = false;
    if ( p_rPipe.tag() == "image" )
    {
        m_pPipeInImageFrame = dynamic_cast<image_pipe_t*>(&p_rPipe);
        found = true;
    }
    if ( p_rPipe.tag() == "startendinfo" )
    {
        m_pPipeInStartEndInfo = dynamic_cast<startendinfo_pipe_t*>(&p_rPipe);
        found = true;
    }
    if ( p_rPipe.tag() == "xseamleft" )
    {
        m_pPipeInXSeamLeft = dynamic_cast<double_pipe_t*>(&p_rPipe);
        found = true;
    }
    if ( p_rPipe.tag() == "xseamright" )
    {
        m_pPipeInXSeamRight = dynamic_cast<double_pipe_t*>(&p_rPipe);
        found = true;
    }
    if ( !found )
    {
        assert(false && "Error in pipes definition");
        return false;
    }
    return BaseFilter::subscribe(p_rPipe, p_oGroup);
}

void NotchSize::proceedGroup(const void* sender, fliplib::PipeGroupEventArgs& e)
{
    poco_assert_dbg(m_pPipeInImageFrame != nullptr);
    poco_assert_dbg(m_pPipeInStartEndInfo != nullptr);
    poco_assert_dbg(m_pPipeInXSeamLeft != nullptr);
    poco_assert_dbg(m_pPipeInXSeamRight != nullptr);

    m_oNotchSizeCalculator.reset();
    m_hasEdge = false;

    const auto & rFrameIn(m_pPipeInImageFrame->read(m_oCounter));
    const auto & rImage(rFrameIn.data());
    const auto & rStartEndInfoIn(m_pPipeInStartEndInfo->read(m_oCounter));
    const auto & rXSeamLeftIn(m_pPipeInXSeamLeft->read(m_oCounter));
    const auto & rXSeamRightIn(m_pPipeInXSeamRight->read(m_oCounter));

    auto oAnalysisResult = std::min({rFrameIn.analysisResult(),
                                    rStartEndInfoIn.analysisResult(),
                                    rXSeamLeftIn.analysisResult(),
                                    rXSeamRightIn.analysisResult()});

    interface::GeoDoublearray oGeoNotchSizeOutStart = {
        rFrameIn.context(),
        geo2d::Doublearray{1, 0, eRankMin},
        oAnalysisResult,
        0.0};
    interface::GeoDoublearray oGeoNotchSizeOutEnd = {
        rFrameIn.context(),
        geo2d::Doublearray{1, 0, eRankMin},
        oAnalysisResult,
        0.0};

    if ( !rImage.isValid()
        || rStartEndInfoIn.ref().size() == 0
        || rXSeamLeftIn.ref().size() == 0 || inputIsInvalid(rXSeamLeftIn)
        || rXSeamRightIn.ref().size() == 0 || inputIsInvalid(rXSeamRightIn)
        )
    {
        m_oSpTrafo = nullptr;
        preSignalAction();
        m_oPipeOutNotchSizeStart.signal(oGeoNotchSizeOutStart);
        m_oPipeOutNotchSizeEnd.signal(oGeoNotchSizeOutEnd);
        return;
    }

    auto & rStartEndInfo = rStartEndInfoIn.ref().getData().front();
    auto & rContext = rFrameIn.context();
    m_oSpTrafo = rContext.trafo();

    //computation only if a valid full edge was found
    m_hasEdge = rStartEndInfo.m_oImageState == StartEndInfo::ImageState::FullEdgeVisible
        && (rStartEndInfo.m_oImageStateEvaluation == StartEndInfo::ImageStateEvaluation::StartEdge
        || rStartEndInfo.m_oImageStateEvaluation == StartEndInfo::ImageStateEvaluation::EndEdge);

    if (m_hasEdge)
    {
        assert(rStartEndInfo.m_oImageState == StartEndInfo::ImageState::FullEdgeVisible);
        assert(rStartEndInfo.isTopDark != rStartEndInfo.isBottomDark);

        StartEndInfo::FittedLine lineLeft(rStartEndInfo.leftEdge);
        StartEndInfo::FittedLine lineRight(rStartEndInfo.rightEdge);

        //adjust trafo, bring everything to current image coordinates
        auto oStartEndTrafo = rStartEndInfoIn.context().trafo();
        int offset_x = m_oSpTrafo->dx() - oStartEndTrafo->dx();
        int offset_y = m_oSpTrafo->dy() - oStartEndTrafo->dy();
        if ( offset_x != 0 || offset_y != 0)
        {
            lineLeft.q += (lineLeft.m * offset_x - offset_y);
            lineRight.q += (lineRight.m * offset_x - offset_y);
        }

        assert(lineLeft.getY(512 - m_oSpTrafo->dx()) + m_oSpTrafo->dy() ==  rStartEndInfo.leftEdge.getY(512 - oStartEndTrafo->dx()) + oStartEndTrafo->dy());
        assert(lineRight.getY(512 - m_oSpTrafo->dx()) + m_oSpTrafo->dy() == rStartEndInfo.rightEdge.getY(512 - oStartEndTrafo->dx()) + oStartEndTrafo->dy());


        BackgroundPosition oBackgroundPosition = rStartEndInfo.isTopDark ? BackgroundPosition::Top : BackgroundPosition::Bottom;


        m_oNotchSizeCalculator.m_distImgBorderX = 10;

        auto fGetX = [this] (const interface::GeoDoublearray & rGeo)
        {
            return static_cast<int>(std::round(rGeo.ref().getData().front())) + rGeo.context().trafo()->dx() - m_oSpTrafo->dx();
        };

        int xSeamLeft = fGetX(rXSeamLeftIn);
        int xSeamRight = fGetX(rXSeamRightIn);

        bool initialized = m_oNotchSizeCalculator.init(rImage, xSeamLeft, xSeamRight, m_oThresholdSeam,
            lineLeft, lineRight, oBackgroundPosition);
        if ( !initialized )
        {
            wmLog(eInfo, "NotchSize: %s \n", m_oNotchSizeCalculator.viewErrorMessage().c_str());
        }
        bool found = m_oNotchSizeCalculator.calc(rImage);
        if ( found )
        {
            auto oNotchSize = m_oNotchSizeCalculator.getNotchSize();
            assert(oNotchSize.second);
            auto & rNotchSize_pixel = oNotchSize.first;
            auto Yreference = m_oNotchSizeCalculator.getYReference();
            auto indentedPoint = m_oNotchSizeCalculator.getIndentedPoint();

            auto & rCalib = system::CalibDataSingleton::getCalibrationCoords(math::SensorId::eSensorId0);

            geo2d::Point sensorOffset = {m_oSpTrafo->dx() + rContext.HW_ROI_x0, m_oSpTrafo->dy() + rContext.HW_ROI_y0};
            double pixel_to_mm = rCalib.pixel_to_mm_OnHorizontalPlane(100, (int) (indentedPoint.x + sensorOffset.x), (int)(Yreference + sensorOffset.y));

            double oNotchSize_mm = rNotchSize_pixel * pixel_to_mm;

            // fill the correct output pipe (start or end) with the notch size
            if (rStartEndInfo.m_oImageStateEvaluation == StartEndInfo::ImageStateEvaluation::StartEdge)
            {
                oGeoNotchSizeOutStart = interface::GeoDoublearray{
                    rFrameIn.context(),
                        geo2d::Doublearray{1, oNotchSize_mm, eRankMax},
                    oAnalysisResult,
                    1.0};
            }
            else
            {
                assert(rStartEndInfo.m_oImageStateEvaluation == StartEndInfo::ImageStateEvaluation::EndEdge);
                oGeoNotchSizeOutEnd = interface::GeoDoublearray{
                    rFrameIn.context(),
                        geo2d::Doublearray{1, oNotchSize_mm, eRankMax},
                    oAnalysisResult,
                    1.0};
            }
        }
        else
        {
            wmLog(eDebug, "No notch found: %s \n", m_oNotchSizeCalculator.viewErrorMessage().c_str());
        }

    }
    preSignalAction();

    // it's important the other output pipe has a value = 0, so that in the graph it's possible to just the two pipes to get "any" NotchSize
    assert( oGeoNotchSizeOutStart.ref().getRank().front() != eRankMax  || oGeoNotchSizeOutEnd.ref().getRank().front() != eRankMax);
    assert( oGeoNotchSizeOutStart.ref().getData().front() == 0.0  || oGeoNotchSizeOutEnd.ref().getData().front() == 0.0);

    m_oPipeOutNotchSizeStart.signal(oGeoNotchSizeOutStart);
    m_oPipeOutNotchSizeEnd.signal(oGeoNotchSizeOutEnd);

}

void NotchSize::paint()
{
    using namespace image;

    if ( m_oVerbosity < VerbosityType::eMedium || m_oSpTrafo.isNull() )
    {
        return;
    }
    auto &rCanvas(canvas<image::OverlayCanvas>(m_oCounter));
    using namespace precitec::image;
    using namespace precitec::filter::start_end_detection;
    using precitec::geo2d::StartEndInfo;
    using precitec::geo2d::Point;


    const auto & rTrafo(*m_oSpTrafo);
    auto dx = rTrafo.dx();
    auto dy = rTrafo.dy();

    OverlayLayer & rLayerContour(rCanvas.getLayerContour());
    OverlayLayer & rLayerPosition(rCanvas.getLayerPosition());
    OverlayLayer & rLayerLine(rCanvas.getLayerLine());

    if ( !m_oNotchSizeCalculator.isInitialized() && m_oVerbosity >= VerbosityType::eHigh )
    {
        // draw the input position to show why they are invalid (if possible)
        if ( !m_hasEdge )
        {
            return;
        }
        {
            auto leftX = m_oNotchSizeCalculator.mLiNaht;
            auto leftY = m_oNotchSizeCalculator.mLeftEdge.getY(leftX);
            auto borderLeftY = m_oNotchSizeCalculator.mLeftEdge.getY(0);
            auto oOverlayLeft = rTrafo(geo2d::Point{int(std::round(leftX)), int(std::round(leftY))});
            rLayerPosition.add<OverlayLine>(rTrafo(geo2d::Point{0, int(borderLeftY)}), oOverlayLeft, Color::Red());
            rLayerPosition.add<OverlayCross>(oOverlayLeft, 20, Color::Red());
        }

        {
            auto rightX = m_oNotchSizeCalculator.mReNaht;
            auto rightY = m_oNotchSizeCalculator.mRightEdge.getY(rightX);
            auto borderRightY = m_oNotchSizeCalculator.mRightEdge.getY(1024);
            auto oOverlayRight = rTrafo(geo2d::Point{int(std::round(rightX)), int(std::round(rightY))});
            rLayerPosition.add<OverlayLine>(rTrafo(geo2d::Point{1024, int(borderRightY)}), oOverlayRight, Color::Red());
            rLayerPosition.add<OverlayCross>(oOverlayRight, 20, Color::Red());
        }

        return;
    }

    auto oNotchresult = m_oNotchSizeCalculator.getNotchSize();
    if ( !oNotchresult.second )
    {
        return;
    }
    auto & rNotchSize_pixel = oNotchresult.first;

    int y_reference = static_cast<int> (std::round(m_oNotchSizeCalculator.mYReference));

    const auto & rFittedNotch = m_oNotchSizeCalculator.mFittedNotch;
    const auto & rNotchPoints = m_oNotchSizeCalculator.mNotchPoints;

    //show line used for notch computation

    rLayerLine.add<OverlayLine>(m_oNotchSizeCalculator.mLiNaht + dx, y_reference + dy, m_oNotchSizeCalculator.mReNaht + dx, y_reference + dy, Color::Orange());

    int xFitLeft = rNotchPoints[rFittedNotch.index_left].x;
    int xFitRight = rNotchPoints[rFittedNotch.index_right].x;
    auto colorNotch = rNotchSize_pixel > 0 ? Color::Orange() : Color::Red();
    rLayerLine.add<OverlayLine>(xFitLeft + dx, y_reference, m_oNotchSizeCalculator.mIndentation.x + dx, m_oNotchSizeCalculator.mIndentation.y + dy,
        colorNotch);
    rLayerLine.add<OverlayLine>(xFitRight + dx, y_reference, m_oNotchSizeCalculator.mIndentation.x + dx, m_oNotchSizeCalculator.mIndentation.y + dy,
        colorNotch);


    if ( m_oVerbosity < VerbosityType::eHigh)
    {
        return;
    }

    Color ColorActualPoints = Color::Magenta();
    Color ColorFit = Color::Red();
    ColorFit.alpha = 128;


    auto fLinePointToOverlay = [&](int x, const StartEndInfo::FittedLine & rLine)
    {
        int y = static_cast<int>(std::round(rLine.getY(x)));
        return Point{x + dx, y + dy};
    };

    auto fParabolaPointToOverlay = [&](int x)
    {
        int y = static_cast<int>(std::round(rFittedNotch.a * x * x + rFittedNotch.b * x + rFittedNotch.c));
        return geo2d::Point{x + dx, y + dy};
    };

    int index = 0;
    for ( ; index < rFittedNotch.index_left; index++ )
    {
        auto & point = rNotchPoints[index];
        rLayerPosition.add<OverlayPoint>(point.x + dx, point.y + dy, ColorActualPoints);
    }

    //points actually used for fitting
    for ( ; index <= rFittedNotch.index_right; index++ )
    {
        auto & point = rNotchPoints[index];
        rLayerPosition.add<OverlayPoint>(point.x + dx, point.y + dy, ColorActualPoints);
        auto overlayParabolaPoint = fParabolaPointToOverlay(point.x);
        rLayerContour.add<OverlayLine>(point.x + dx, point.y + dy, overlayParabolaPoint.x, overlayParabolaPoint.y, ColorFit);
    }

    for ( ; index < (int)( rNotchPoints.size()); index++ )
    {
        auto & point = rNotchPoints[index];
        rLayerPosition.add<OverlayPoint>(point.x + dx, point.y + dy, ColorActualPoints);
    }


    //show the line used to fit the edge
    auto overlayMeanEdgeStart = fLinePointToOverlay(0, m_oNotchSizeCalculator.mMeanEdge);
    auto overlayMeanEdgeLeft = fLinePointToOverlay(m_oNotchSizeCalculator.mLiNaht, m_oNotchSizeCalculator.mMeanEdge);
    auto overlayMeanEdgeRight = fLinePointToOverlay(m_oNotchSizeCalculator.mReNaht, m_oNotchSizeCalculator.mMeanEdge);
    auto overlayMeanEdgeEnd = fLinePointToOverlay(m_oNotchSizeCalculator.mReNaht + m_oNotchSizeCalculator.mLiNaht, m_oNotchSizeCalculator.mMeanEdge);

    rLayerContour.add<OverlayLine>(overlayMeanEdgeStart.x, overlayMeanEdgeStart.y, overlayMeanEdgeLeft.x, overlayMeanEdgeLeft.y, ColorFit);
    rLayerContour.add<OverlayLine>(overlayMeanEdgeRight.x, overlayMeanEdgeRight.y, overlayMeanEdgeEnd.x, overlayMeanEdgeEnd.y, ColorFit);

    //show parabola
    auto parabolaOverlayPoint = fParabolaPointToOverlay(m_oNotchSizeCalculator.mLiNaht);
    for ( int x = m_oNotchSizeCalculator.mLiNaht; x <= m_oNotchSizeCalculator.mReNaht; ++x )
    {
        auto overlayPoint = fParabolaPointToOverlay(x);
        rLayerContour.add<OverlayLine>(parabolaOverlayPoint.x, parabolaOverlayPoint.y, overlayPoint.x, overlayPoint.y, ColorFit);
        parabolaOverlayPoint = overlayPoint;
    }

}

} //end namespaces
}
