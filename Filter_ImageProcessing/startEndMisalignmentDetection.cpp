#include "startEndMisalignmentDetection.h"
#include <common/defines.h>
#include "module/moduleLogger.h"

//#define DEBUG_SEAM_STATE

#include "filter/armStates.h"
#include "filter/algoArray.h"
#include <overlay/overlayLayer.h>
#include <overlay/overlayCanvas.h>
#include <overlay/overlayPrimitive.h>
#include <fliplib/TypeToDataTypeImpl.h>

#include "util/calibDataSingleton.h"
#include <filter/productData.h>

//helper functions
namespace
{
using namespace precitec::filter::start_end_detection;

std::string to_string(EdgeSearch value)
{
    switch(value)
    {
        case EdgeSearch::BothDirections: return "BothDirections";
        break;
        case EdgeSearch::OnlyBackgroundOnTop: return "OnlyBackgroundOnTop";
        break;
        case EdgeSearch::OnlyBackgroundOnBottom: return "OnlyBackgroundOnBottom";
        break;
    }
    assert(false && "not reachable if all cases handled");
    return "?";
}

std::string to_string(StartEndDetectionInSeam::SeamState value)
{
    typedef StartEndDetectionInSeam::SeamState SeamState;
    switch ( value )
    {
        case SeamState::Unknown:
            return "Unknown";
        case SeamState::Invalid:
            return "Invalid";
        case SeamState::WaitingFirstStartEdge:
            return "WaitingFirstStartEdge";
        case SeamState::WaitingFirstEndEdge:
            return "WaitingFirstEndEdge";
        case SeamState::FirstStartEdgeFound:
            return "FirstStartEdgeFound";
        case SeamState::SecondStartEdgeFound:
            return "SecondStartEdgeFound";
        case SeamState::FullStartEdgeFound:
            return "FullStartEdgeFound";
        case SeamState::FullStartEdgeFound_EndMissing:
            return "FullStartEdgeFound_EndMissing";
        case SeamState::FullImageFound:
            return "FullImageFound";
        case SeamState::FullImageFound_StartMissing:
            return "FullImageFound_StartMissing";
        case SeamState::FirstEndEdgeFound:
            return "FirstEndEdgeFound";
        case SeamState::SecondEndEdgeFound:
            return "SecondEndEdgeFound";
        case SeamState::FirstEndEdgeFound_StartMissing:
            return "FirstEndEdgeFound_StartMissing";
        case SeamState::SecondEndEdgeFound_StartMissing:
            return "SecondEndEdgeFound_StartMissing";
        case SeamState::FullEndEdgeFound:
            return "FullEndEdgeFound";
        case SeamState::FullEndEdgeFound_StartMissing:
            return "FullEndEdgeFound_StartMissing";
        case SeamState::EndBackgroundImageFound:
            return "EndBackgroundImageFound";
        case SeamState::EndBackgroundImageFound_StartMissing:
            return "EndBackgroundImageFound_StartMissing";
        case SeamState::EndBackgroundImageFound_StartEndMissing:
            return "EndBackgroundImageFound_StartEndMissing";
        case SeamState::EndBackgroundImageFound_EndMissing:
            return "EndBackgroundImageFound_EndMissing";
    }
    assert(false && "not reachable if all cases handled");
    return "Unkown value";
}
} //end anonymous namespace

namespace precitec
{
namespace filter
{

using precitec::image::BImage;
namespace start_end_detection
{


bool StartEndDetectionInSeam::isSeamStateConsistent() const
{
#ifdef DEBUG_SEAM_STATE
    std::cout << "Seam State " << int(m_SeamState) << "\t" ;
    for (auto && rPos : m_EdgePositions)
    {
        std::cout << rPos.found() << "\t";
    }
    std::cout << std::endl;
#endif
    if ( mTransitionFromFullImage_mm >= 0 && mTransitionFromBackground_mm < 0 )
    {
        return m_SeamState == SeamState::Invalid;
    }
    typedef EdgePositionIndex idx;
    switch (m_SeamState)
    {
        case (SeamState::Unknown):
            return true;

        case (SeamState::Invalid):
            return true;

        case (SeamState::WaitingFirstStartEdge):
        case (SeamState::WaitingFirstEndEdge):
        case (SeamState::FullImageFound_StartMissing):
        case (SeamState::EndBackgroundImageFound_StartEndMissing):
            return !startEdgesFound() && !endEdgesFound();


        case (SeamState::FirstStartEdgeFound):
            return (m_EdgePositions[idx::eLeftStartEdge].found() != m_EdgePositions[idx::eRightStartEdge].found())
                && ! endEdgesFound();

        case (SeamState::SecondStartEdgeFound):
        case (SeamState::FullStartEdgeFound):
        case (SeamState::FullStartEdgeFound_EndMissing):
        case (SeamState::FullImageFound):
        case (SeamState::EndBackgroundImageFound_EndMissing):
            return startEdgesFound() && !endEdgesFound();

        case (SeamState::FirstEndEdgeFound):
            return startEdgesFound()
                && (m_EdgePositions[idx::eLeftEndEdge].found() != m_EdgePositions[idx::eRightEndEdge].found());

        case (SeamState::FirstEndEdgeFound_StartMissing):
            return !startEdgesFound()
                && (m_EdgePositions[idx::eLeftEndEdge].found() != m_EdgePositions[idx::eRightEndEdge].found());


        case (SeamState::SecondEndEdgeFound):
        case (SeamState::FullEndEdgeFound):
        case (SeamState::EndBackgroundImageFound):
            return  startEdgesFound() && endEdgesFound();

        case (SeamState::SecondEndEdgeFound_StartMissing):
        case (SeamState::FullEndEdgeFound_StartMissing):
        case (SeamState::EndBackgroundImageFound_StartMissing):
            return !startEdgesFound() && endEdgesFound();

    }
    assert(false);
    return false;
}

StartEndDetectionInSeam::SeamState StartEndDetectionInSeam::getNextState_FoundOnlyBackground() const
{
#ifdef DEBUG_SEAM_STATE
    std::cout << __FUNCTION__ << std::endl;
#endif

    assert(isSeamStateConsistent());

    switch (m_SeamState)
    {
        case (SeamState::WaitingFirstStartEdge):
        case (SeamState::WaitingFirstEndEdge):
        case (SeamState::FullStartEdgeFound_EndMissing):
        case (SeamState::EndBackgroundImageFound):
        case (SeamState::EndBackgroundImageFound_EndMissing):
        case (SeamState::EndBackgroundImageFound_StartEndMissing):
        case (SeamState::EndBackgroundImageFound_StartMissing):
            // state does not change
            return m_SeamState;

        case (SeamState::Unknown):
            return m_searchForEdges == SearchForEdges::end ? SeamState::WaitingFirstEndEdge : SeamState::WaitingFirstStartEdge;

        case (SeamState::SecondEndEdgeFound):
        case (SeamState::FullEndEdgeFound):
            return SeamState::EndBackgroundImageFound;

        case SeamState::FirstEndEdgeFound:
        case SeamState::FirstEndEdgeFound_StartMissing:
            return SeamState::EndBackgroundImageFound_EndMissing;


        case (SeamState::SecondEndEdgeFound_StartMissing):
        case SeamState::FullEndEdgeFound_StartMissing:
            return SeamState::EndBackgroundImageFound_StartMissing;

        case SeamState::Invalid:
        case SeamState::FirstStartEdgeFound:
        case SeamState::SecondStartEdgeFound:
        case SeamState::FullStartEdgeFound:
            // not supported case: piece so short that does not fit completely in an image
            return SeamState::Invalid;

        case SeamState::FullImageFound:
        case SeamState::FullImageFound_StartMissing:
            //special case: the end edge fell between images
            return m_SeamState;
    }
    assert(false && "Case not handled");
    return SeamState::Invalid;
}

StartEndDetectionInSeam::SeamState StartEndDetectionInSeam::getNextState_FoundOnlyMaterial() const
{
#ifdef DEBUG_SEAM_STATE
    std::cout << __FUNCTION__ << std::endl;
#endif

    assert(isSeamStateConsistent());
    switch(m_SeamState)
    {
        case SeamState::Unknown:
        case SeamState::WaitingFirstStartEdge:
        case SeamState::WaitingFirstEndEdge:
            return SeamState::FullImageFound_StartMissing;

        case SeamState::FirstStartEdgeFound:
            if (m_lastStartEndDetectionInImage.m_parameters.m_searchWidthLeft != 0 && m_lastStartEndDetectionInImage.m_parameters.m_searchWidthRight != 0)
            {
                //special case: Start Edge has fallen between images
                return SeamState::FullImageFound_StartMissing;
            }
            else
            {
                return SeamState::FullImageFound;
            }

        case SeamState::Invalid:
        case SeamState::SecondEndEdgeFound:
        case SeamState::FirstEndEdgeFound:
        case SeamState::FirstEndEdgeFound_StartMissing:
        case SeamState::SecondEndEdgeFound_StartMissing:
        case SeamState::FullEndEdgeFound:
        case SeamState::FullEndEdgeFound_StartMissing:
                // not supported case  - or the end edge was wrong
                return SeamState::Invalid;

        case SeamState::EndBackgroundImageFound:
        case SeamState::EndBackgroundImageFound_EndMissing:
        case SeamState::EndBackgroundImageFound_StartEndMissing:
        case SeamState::EndBackgroundImageFound_StartMissing:
            //ignore what comes next
            return m_SeamState;

        case SeamState::FullImageFound:
        case SeamState::SecondStartEdgeFound:
        case SeamState::FullStartEdgeFound:
        case SeamState::FullStartEdgeFound_EndMissing:
            if (m_searchForEdges == SearchForEdges::start) return SeamState::FullStartEdgeFound_EndMissing;
            return SeamState::FullImageFound;


        case SeamState::FullImageFound_StartMissing:
            return SeamState::FullImageFound_StartMissing;
    }
    assert(false && "Case not handled");
    return SeamState::Invalid;
}


std::tuple<StartEndDetectionInSeam::SeamState,StartEndDetectionInSeam::EdgePositionIndex>  StartEndDetectionInSeam::getNextState_FoundOneEdge(EdgeSide p_EdgeSide) const
{
#ifdef DEBUG_SEAM_STATE
    std::cout << __FUNCTION__ << std::endl;
#endif

    assert(isSeamStateConsistent());
    typedef EdgePositionIndex idx;

#ifndef NDEBUG
    bool ignoreEdge = false;
#endif

    auto indexStart = p_EdgeSide == EdgeSide::left ? idx::eLeftStartEdge : idx::eRightStartEdge;
    auto indexEnd = p_EdgeSide == EdgeSide::left ? idx::eLeftEndEdge : idx::eRightEndEdge;

    SeamState newSeamState = m_SeamState;
    StartEndDetectionInSeam::EdgePositionIndex indexToUpdate = idx::eInvalid;

    switch (m_SeamState)
    {
        case (SeamState::Invalid):
        case (SeamState::WaitingFirstEndEdge): // before endEdge there have to be a full image, so ignore index and do not change seam state.
            indexToUpdate = idx::eInvalid;
            break;

        case (SeamState::WaitingFirstStartEdge):
        case (SeamState::Unknown):
            assert( waitingForEdge() == WaitingForEdge::Start);
            newSeamState = SeamState::FirstStartEdgeFound;
            indexToUpdate = indexStart;
            break;

        case (SeamState::FirstStartEdgeFound):
            assert( waitingForEdge() == WaitingForEdge::Start);
            if (!m_EdgePositions[indexStart].found())
            {
                newSeamState = SeamState::SecondStartEdgeFound;
            }
            indexToUpdate = indexStart;
            break;

        case (SeamState::SecondStartEdgeFound):
            assert( waitingForEdge() == WaitingForEdge::Start);
            indexToUpdate = indexStart;
            break;

        case (SeamState::FullStartEdgeFound):
            assert( waitingForEdge() == WaitingForEdge::Start);
            if ( m_EdgeMisalignmentMode == EdgeMisalignmentMode::IgnorePartialEdgeAfterFullEdge)
            {
#ifndef NDEBUG
                ignoreEdge = true;
#endif
            }
            else
            {
                assert(m_EdgeMisalignmentMode == EdgeMisalignmentMode::UsePartialEdge);
                indexToUpdate = indexStart;
            }
            break;

        case (SeamState::FullImageFound):
            assert( waitingForEdge() == WaitingForEdge::End);
            newSeamState = SeamState::FirstEndEdgeFound;
            indexToUpdate = indexEnd;
            break;

        case (SeamState::FullImageFound_StartMissing):
            assert( waitingForEdge() == WaitingForEdge::End);
            newSeamState = SeamState::FirstEndEdgeFound_StartMissing;
            indexToUpdate = indexEnd;
            break;

        case (SeamState::FirstEndEdgeFound):
            assert( waitingForEdge() == WaitingForEdge::End);
            if (!m_EdgePositions[indexEnd].found())
            {
                newSeamState = SeamState::SecondEndEdgeFound;
            }
            indexToUpdate = indexEnd;
            break;

        case (SeamState::SecondEndEdgeFound):
            assert( waitingForEdge() == WaitingForEdge::End);
            indexToUpdate = indexEnd;
            break;

        case (SeamState::SecondEndEdgeFound_StartMissing):
        case (SeamState::FirstEndEdgeFound_StartMissing):
            assert( waitingForEdge() == WaitingForEdge::End);
            if (!m_EdgePositions[indexEnd].found())
            {
                newSeamState = SeamState::SecondEndEdgeFound_StartMissing;
            }
            indexToUpdate = indexEnd;
            break;

        case (SeamState::FullEndEdgeFound):
        case (SeamState::FullEndEdgeFound_StartMissing):
            assert( waitingForEdge() == WaitingForEdge::End);
            if ( m_EdgeMisalignmentMode == EdgeMisalignmentMode::IgnorePartialEdgeAfterFullEdge )
            {
#ifndef NDEBUG
                ignoreEdge = true;
#endif
                indexToUpdate = idx::eInvalid;
            }
            else
            {
                assert(m_EdgeMisalignmentMode == EdgeMisalignmentMode::UsePartialEdge);
                indexToUpdate = indexEnd;
            }
            break;

        case (SeamState::EndBackgroundImageFound):
        case (SeamState::EndBackgroundImageFound_EndMissing):
        case (SeamState::EndBackgroundImageFound_StartEndMissing):
        case (SeamState::EndBackgroundImageFound_StartMissing):
        case (SeamState::FullStartEdgeFound_EndMissing):
            assert( waitingForEdge() == WaitingForEdge::None);
            indexToUpdate = idx::eInvalid;
            break;
    }
    assert(!ignoreEdge || indexToUpdate == idx::eInvalid);

    return std::tuple<StartEndDetectionInSeam::SeamState, StartEndDetectionInSeam::EdgePositionIndex>{newSeamState, indexToUpdate};
}


std::tuple<StartEndDetectionInSeam::SeamState,StartEndDetectionInSeam::EdgePositionIndex, StartEndDetectionInSeam::EdgePositionIndex> StartEndDetectionInSeam::getNextState_FoundEdges() const
{
#ifdef DEBUG_SEAM_STATE
    std::cout << __FUNCTION__ << std::endl;
#endif

    assert(isSeamStateConsistent());

    typedef EdgePositionIndex idx;

    auto fUpdateStartEdges = [&](SeamState newSeamState)
    {
        //m_EdgePositions[idx::eLeftStartEdge]  = p_EdgeLeft;
        //m_EdgePositions[idx::eRightStartEdge]  = p_EdgeRight;
        return std::tuple<SeamState, EdgePositionIndex, EdgePositionIndex> {newSeamState, idx::eLeftStartEdge, idx::eRightStartEdge};
    };

    auto fUpdateEndEdges = [&](SeamState newSeamState)
    {
        //m_EdgePositions[idx::eLeftEndEdge]  = p_EdgeLeft;
        //m_EdgePositions[idx::eRightEndEdge]  = p_EdgeRight;
        return std::tuple<SeamState, EdgePositionIndex, EdgePositionIndex> {newSeamState, idx::eLeftEndEdge, idx::eRightEndEdge};
    };


    switch (m_SeamState)
    {
        case (SeamState::Invalid):
            return std::tuple<SeamState, EdgePositionIndex, EdgePositionIndex> {SeamState::Invalid, idx::eInvalid, idx::eInvalid};

        case (SeamState::Unknown):
        case (SeamState::WaitingFirstStartEdge):
        case (SeamState::FirstStartEdgeFound):
        case (SeamState::SecondStartEdgeFound):
        case (SeamState::FullStartEdgeFound):
            assert( waitingForEdge() == WaitingForEdge::Start);
            return fUpdateStartEdges(SeamState::FullStartEdgeFound);

        case (SeamState::FullImageFound_StartMissing):
        case (SeamState::FirstEndEdgeFound_StartMissing):
            assert( waitingForEdge() == WaitingForEdge::End);
            return fUpdateEndEdges(SeamState::SecondEndEdgeFound_StartMissing);

        case (SeamState::FullImageFound):
        case (SeamState::FirstEndEdgeFound):
            assert( waitingForEdge() == WaitingForEdge::End);
            return fUpdateEndEdges(SeamState::SecondEndEdgeFound);

        case (SeamState::SecondEndEdgeFound):
            assert(waitingForEdge() == WaitingForEdge::End);
            return fUpdateEndEdges(SeamState::FullEndEdgeFound);
        case (SeamState::SecondEndEdgeFound_StartMissing):
            assert(waitingForEdge() == WaitingForEdge::End);
            return fUpdateEndEdges(SeamState::FullEndEdgeFound_StartMissing);
        case (SeamState::FullEndEdgeFound):
        case (SeamState::FullEndEdgeFound_StartMissing):
            assert(waitingForEdge() == WaitingForEdge::End);
            return fUpdateEndEdges(m_SeamState);

        case (SeamState::EndBackgroundImageFound_StartEndMissing):
        case (SeamState::EndBackgroundImageFound_EndMissing):
        case (SeamState::EndBackgroundImageFound_StartMissing):
        case (SeamState::EndBackgroundImageFound):
        case (SeamState::FullStartEdgeFound_EndMissing):
        case (SeamState::WaitingFirstEndEdge): // ignore edges before the first fullImage
            assert( waitingForEdge() == WaitingForEdge::None);
            //ignore what comes next
            return std::tuple<SeamState, EdgePositionIndex, EdgePositionIndex> {m_SeamState, idx::eInvalid, idx::eInvalid};
    }

    assert(false && "not reachable if all cases handled");
    return std::tuple<SeamState, EdgePositionIndex, EdgePositionIndex> {m_SeamState, idx::eInvalid, idx::eInvalid};
}


StartEndDetectionInSeam::StartEndDetectionInSeam()
{
    resetState();
}

void StartEndDetectionInSeam::resetState()
{
    m_SeamState = SeamState::Unknown;
    for (auto && rPosition : m_EdgePositions)
    {
        rPosition.m_edgeLocalPosition.mAppearance = Appearance::Unknown;
    }
#ifndef NDEBUG
    for (auto && rPosition : m_EdgePositions)
    {
        assert(!rPosition.found());
    }
#endif
    m_lastImageContext = InputImageContext{};
    m_lastStartEndDetectionInImage.resetState();
    m_lastImageStateEvaluation = geo2d::StartEndInfo::ImageStateEvaluation::Unknown;
    mTransitionFromBackground_mm = -1.0;
    mTransitionFromFullImage_mm = -1.0;
    m_searchForEdges = SearchForEdges::both;
    m_assumeFullImage = false;
}


StartEndDetectionInSeam::EdgeMisalignment::EdgeMisalignment(const EdgePositionInSeam& rLeftEdge, const EdgePositionInSeam& rRightEdge)
        : m_leftEdge(rLeftEdge), m_rightEdge(rRightEdge)
{
#ifndef NDEBUG
    std::cout << "Edge EdgeMisalignment : "
              << " left " << rLeftEdge.found() << " " <<  rLeftEdge.m_edgeLocalPosition.line.q << "( " << rLeftEdge.m_contextImage_mm << ") "
              << " right " << rRightEdge.found() << " " << rRightEdge.m_edgeLocalPosition.line.q << "( " << rRightEdge.m_contextImage_mm << ") "
              << std::endl;
#endif
}


StartEndDetectionInSeam::EdgeMisalignment::EdgeMisalignment()
{
    assert(!m_leftEdge.found());
    assert(!m_rightEdge.found());
}


double StartEndDetectionInSeam::EdgeMisalignment::misalignment_mm() const
{
    assert(m_leftEdge.m_edgeLocalPosition.line.m == m_rightEdge.m_edgeLocalPosition.line.m
        && "EdgePositionInSeam assumed that left and right edge have the same slope"
    );
    assert(m_rightEdge.found() && m_leftEdge.found());
#ifndef NDEBUG
    if ( m_rightEdge.m_contextImage_mm != m_leftEdge.m_contextImage_mm )
    {
        wmLog(eDebug, "misalignment_mm: right %f + %f , left %f + %f  \n",
            m_rightEdge.m_contextImage_mm, m_rightEdge.m_distanceImageBorder_mm,
            m_leftEdge.m_contextImage_mm, m_leftEdge.m_distanceImageBorder_mm);
    }

#endif
    return (m_rightEdge.m_contextImage_mm + m_rightEdge.m_distanceImageBorder_mm)
           - (m_leftEdge.m_contextImage_mm + m_leftEdge.m_distanceImageBorder_mm);
}

Appearance StartEndDetectionInSeam::EdgeMisalignment::getAppearance() const
{
    //here we check the state of the 2 edges together: backgroundOnTop, backgroundOnBottom, edge not complete
    if ( m_leftEdge.found() && m_rightEdge.found()
          && m_leftEdge.m_edgeLocalPosition.mAppearance == m_rightEdge.m_edgeLocalPosition.mAppearance)
    {
        assert(m_leftEdge.m_edgeLocalPosition.line.m == m_rightEdge.m_edgeLocalPosition.line.m);
        return m_leftEdge.m_edgeLocalPosition.mAppearance;
    }
    return Appearance::Unsupported;  //it's not a full edge

}

double StartEndDetectionInSeam::EdgeMisalignment::distanceFromSeamStart(bool useY, bool isStartEdge) const
{
    assert(m_leftEdge.m_edgeLocalPosition.line.m == m_rightEdge.m_edgeLocalPosition.line.m);
    Appearance bothEdgesAppearance = getAppearance();
    if ( bothEdgesAppearance != Appearance::BackgroundOnTop && bothEdgesAppearance != Appearance::BackgroundOnBottom )
    {
        //edge not found
        return -1;
    }
    double distanceFromSeamStartLeft = m_leftEdge.m_contextImage_mm;
    double distanceFromSeamStartRight = m_rightEdge.m_contextImage_mm;
    if ( useY )
    {
        distanceFromSeamStartLeft += m_leftEdge.m_distanceImageBorder_mm;
        distanceFromSeamStartRight +=m_rightEdge.m_distanceImageBorder_mm;
    }
    //return the innermost edge
    if ( isStartEdge )
    {
        return std::max(distanceFromSeamStartLeft, distanceFromSeamStartRight);
    }
    else
    {
        return std::min(distanceFromSeamStartLeft, distanceFromSeamStartRight);
    }
}


const std::vector< precitec::geo2d::Point >& StartEndDetectionInSeam::viewEdgePointsInLastImage(bool left) const
{
    return m_lastStartEndDetectionInImage.viewLastEdgePoints(left);
}

const StartEndDetectionInImage::Parameters & StartEndDetectionInSeam::viewDetectionInImageParameters() const
{
    return m_lastStartEndDetectionInImage.m_parameters;
}

std::tuple< StartEndDetectionInSeam::WaitingForEdge, EdgeSearch > StartEndDetectionInSeam::nextEdgeParameters() const
{
    auto oWaitingForEdge = waitingForEdge();
    switch (oWaitingForEdge)
    {
    case WaitingForEdge::Start:
        return std::tuple< WaitingForEdge, EdgeSearch> {oWaitingForEdge, searchDirectionStartEdge()};
    case WaitingForEdge::End:
        return std::tuple< WaitingForEdge, EdgeSearch>{oWaitingForEdge, searchDirectionEndEdge()};
    case WaitingForEdge::Both:
        return std::tuple< WaitingForEdge, EdgeSearch>{oWaitingForEdge, EdgeSearch::BothDirections};
    case WaitingForEdge::None:
        return std::tuple< WaitingForEdge, EdgeSearch>{oWaitingForEdge, EdgeSearch::BothDirections};
    }
    assert(false && "not reachable if all cases handled");
    return std::tuple< WaitingForEdge, EdgeSearch>{oWaitingForEdge, EdgeSearch::BothDirections};
}

//updates m_SeamState,  m_EdgePositions, mTransitionFromBackground_mm, mTransitionFromFullImage_mm
geo2d::StartEndInfo::ImageStateEvaluation StartEndDetectionInSeam::processResult(const StartEndDetectionInImage::Result& rLastResult,
    const InputImageContext & rImageContext)
{
    typedef geo2d::StartEndInfo::ImageState ImageState;
    typedef geo2d::StartEndInfo::ImageStateEvaluation ImageStateEvaluation;


    static const std::map<std::pair<EdgePositionIndex,EdgePositionIndex>, ImageStateEvaluation> oEvaluation2Edges =
    {
        {{EdgePositionIndex::eLeftStartEdge, EdgePositionIndex::eRightStartEdge},ImageStateEvaluation::StartEdge},
        {{EdgePositionIndex::eLeftStartEdge, EdgePositionIndex::eInvalid},ImageStateEvaluation::StartEdge},
        {{EdgePositionIndex::eInvalid, EdgePositionIndex::eRightStartEdge},ImageStateEvaluation::StartEdge},
        {{EdgePositionIndex::eLeftEndEdge, EdgePositionIndex::eRightEndEdge},ImageStateEvaluation::EndEdge},
        {{EdgePositionIndex::eLeftEndEdge, EdgePositionIndex::eInvalid},ImageStateEvaluation::EndEdge},
        {{EdgePositionIndex::eInvalid, EdgePositionIndex::eRightEndEdge},ImageStateEvaluation::EndEdge}
    };
    static const std::map<EdgePositionIndex, ImageStateEvaluation> oEvaluation1Edge =
    {
        {EdgePositionIndex::eLeftStartEdge, ImageStateEvaluation::StartEdge},
        {EdgePositionIndex::eRightStartEdge, ImageStateEvaluation::StartEdge},
        {EdgePositionIndex::eLeftEndEdge, ImageStateEvaluation::EndEdge},
        {EdgePositionIndex::eRightEndEdge, ImageStateEvaluation::EndEdge}
    };

    ImageStateEvaluation oEvaluation = ImageStateEvaluation::Unknown;

    auto fIsFromBelow = [this, & oEvaluation] (const EdgePositionInImage & rEdgePosition)
    {
        switch(m_Direction)
        {
            case Direction::fromAbove:
                return false;
            case Direction::fromBelow:
                return true;
            case Direction::Unknown:
            {
                if (oEvaluation == ImageStateEvaluation::StartEdge)
                {
                   return  (rEdgePosition.mAppearance == Appearance::BackgroundOnBottom);
                }
               else
                {
                    assert(oEvaluation == ImageStateEvaluation::EndEdge && "trying to evaluate a non valid edge");
                    return (rEdgePosition.mAppearance == Appearance::BackgroundOnTop);
                }
            }
        }
        assert(false && "not reachable if all cases handled");
        return false;
    };

    auto fUpdateEdgePosition = [&rImageContext, & rLastResult, & oEvaluation, &fIsFromBelow, this](EdgeSide oEdgeSide)
    {
        auto & rEdgePosition = oEdgeSide == EdgeSide::left? rLastResult.mLeftEdgePosition
            : rLastResult.mRightEdgePosition;

        assert( (EdgePositionInSeam{ rEdgePosition, rImageContext, false}.found()));

        EdgePositionIndex newIndex = EdgePositionIndex::eInvalid;
        std::tie(m_SeamState,newIndex) = getNextState_FoundOneEdge(oEdgeSide);

        if (newIndex < EdgePositionIndex::NUMPOSITIONS)
        {
            auto it = oEvaluation1Edge.find(newIndex);
            assert( it!= oEvaluation1Edge.end());
            oEvaluation = it->second;

            m_EdgePositions[newIndex] = EdgePositionInSeam{ rEdgePosition, rImageContext, fIsFromBelow(rEdgePosition)};
            assert(m_EdgePositions[newIndex].found());

            switch(m_SeamState)
            {
                case SeamState::FirstStartEdgeFound:
                    assert( (newIndex == EdgePositionIndex::eLeftStartEdge) || (newIndex == EdgePositionIndex::eRightStartEdge) );
                    //do not update the transition position yet, we want to compute the seam length only when the seam actually starts (2 edges)
                    break;
                case SeamState::SecondStartEdgeFound:
                case SeamState::FullStartEdgeFound:
                case SeamState::FullStartEdgeFound_EndMissing:
                    assert( (newIndex == EdgePositionIndex::eLeftStartEdge) || (newIndex == EdgePositionIndex::eRightStartEdge) );
                    mTransitionFromBackground_mm = rImageContext.imagePosition_mm;
                    break;
                case SeamState::FirstEndEdgeFound:
                case SeamState::FirstEndEdgeFound_StartMissing:
                case SeamState::SecondEndEdgeFound:
                case SeamState::SecondEndEdgeFound_StartMissing:
                case SeamState::FullEndEdgeFound:
                case SeamState::FullEndEdgeFound_StartMissing:
                    assert( (newIndex == EdgePositionIndex::eLeftEndEdge) || (newIndex == EdgePositionIndex::eRightEndEdge));
                    if (mTransitionFromFullImage_mm < 0)
                    {
                        mTransitionFromFullImage_mm = rImageContext.imagePosition_mm;
                    }
                    break;
                default:
                    wmLog(eWarning, "unexpected seam state");
                    assert(false);
                    break;
            }

        }
        else
        {
            //edge position invalid: could be an invalid image (e.g both start and edge visible), or a partial edge which has been ignored
            switch(m_SeamState)
            {
                case SeamState::FullStartEdgeFound:
                    assert(m_EdgeMisalignmentMode == EdgeMisalignmentMode::IgnorePartialEdgeAfterFullEdge);
                    oEvaluation = ImageStateEvaluation::PartialStartEdgeIgnored;
                    break;
                case SeamState::FullEndEdgeFound:
                case SeamState::FullEndEdgeFound_StartMissing:
                    assert(m_EdgeMisalignmentMode == EdgeMisalignmentMode::IgnorePartialEdgeAfterFullEdge);
                    oEvaluation = ImageStateEvaluation::PartialEndEdgeIgnored;
                    break;
                default:
                    assert(oEvaluation == ImageStateEvaluation::Unknown);
                    break;
            }

        }

    };

    switch ( rLastResult.mImageValidRange.viewImageState() )
    {
    case ImageState::OnlyBackground:
        m_SeamState = getNextState_FoundOnlyBackground();
        switch(waitingForEdge())
        {
            case WaitingForEdge::Start:
                oEvaluation = ImageStateEvaluation::BackgroundBeforeStart;
                break;
            case WaitingForEdge::None:
                oEvaluation = ImageStateEvaluation::BackgroundAfterEnd;
                mTransitionFromFullImage_mm = mTransitionFromFullImage_mm < 0 ? rImageContext.imagePosition_mm : mTransitionFromFullImage_mm;
                break;
            case WaitingForEdge::End:
                oEvaluation = m_searchForEdges == SearchForEdges::end ? ImageStateEvaluation::BackgroundBeforeStart : ImageStateEvaluation::Unknown;
                break;
            default: oEvaluation = ImageStateEvaluation::Unknown;
                break;
        }
        break;

    case ImageState::OnlyMaterial:
        if (rLastResult.mLeftEdgePosition.mAppearance == Appearance::NotAvailable)
        {
            m_EdgePositions[eLeftStartEdge].m_edgeLocalPosition.mAppearance = Appearance::NotAvailable;
        }
        if (rLastResult.mRightEdgePosition.mAppearance == Appearance::NotAvailable)
        {
            m_EdgePositions[eRightStartEdge].m_edgeLocalPosition.mAppearance = Appearance::NotAvailable;
        }
        m_SeamState = getNextState_FoundOnlyMaterial();
        oEvaluation = ImageStateEvaluation::OnlyMaterial;
        mTransitionFromBackground_mm = mTransitionFromBackground_mm < 0 ? rImageContext.imagePosition_mm : mTransitionFromBackground_mm;
        break;

    case ImageState::OnlyLeftEdgeVisible:
        assert(rLastResult.mLeftEdgePosition.valid() && (!rLastResult.mRightEdgePosition.valid() || rLastResult.mRightEdgePosition.mAppearance == Appearance::NotAvailable));
        fUpdateEdgePosition(EdgeSide::left);
        break;

    case ImageState::OnlyRightEdgeVisible:
        assert((!rLastResult.mLeftEdgePosition.valid() || rLastResult.mLeftEdgePosition.mAppearance == Appearance::NotAvailable) && rLastResult.mRightEdgePosition.valid());
        fUpdateEdgePosition(EdgeSide::right);
        break;

    case ImageState::FullEdgeVisible:
    {

        assert( (EdgePositionInSeam{rLastResult.mLeftEdgePosition, rImageContext, false}.found())
        &&  (EdgePositionInSeam{rLastResult.mRightEdgePosition, rImageContext, false}.found()) );

        EdgePositionIndex newIndexLeft = EdgePositionIndex::eInvalid;
        EdgePositionIndex newIndexRight = EdgePositionIndex::eInvalid;

        std::tie(m_SeamState, newIndexLeft, newIndexRight) = getNextState_FoundEdges();
        bool acceptedLeft = newIndexLeft < EdgePositionIndex::NUMPOSITIONS;
        bool acceptedRight = newIndexRight <  EdgePositionIndex::NUMPOSITIONS;

        if ( acceptedLeft || acceptedRight )
        {
            auto it = oEvaluation2Edges.find({newIndexLeft, newIndexRight});
            assert( it != oEvaluation2Edges.end());
            oEvaluation = it->second;

            if ( (newIndexLeft == EdgePositionIndex::eLeftStartEdge) || (newIndexRight == EdgePositionIndex::eRightStartEdge) )
            {
                mTransitionFromBackground_mm = rImageContext.imagePosition_mm;
            }
            else
            {
                assert((newIndexLeft == EdgePositionIndex::eLeftEndEdge) || (newIndexRight == EdgePositionIndex::eRightEndEdge));
                mTransitionFromFullImage_mm = mTransitionFromFullImage_mm < 0 ? rImageContext.imagePosition_mm : mTransitionFromFullImage_mm;
            }

        }
        else
        {
            assert(oEvaluation == ImageStateEvaluation::Unknown);
        }


        if ( acceptedLeft )
        {
            EdgePositionInSeam oEdgeLeft{rLastResult.mLeftEdgePosition, rImageContext,  fIsFromBelow(rLastResult.mLeftEdgePosition)};
            m_EdgePositions[newIndexLeft] = oEdgeLeft;
        }
        if ( acceptedRight )
        {
            EdgePositionInSeam oEdgeRight{rLastResult.mRightEdgePosition, rImageContext,  fIsFromBelow(rLastResult.mRightEdgePosition)};
            m_EdgePositions[newIndexRight] = oEdgeRight;
        }
        break;
    }

    case ImageState::Invalid:
        // while waiting for first end edge there could be a start edge which will cause an ImageState::Invalid
        if (m_SeamState == SeamState::Unknown && m_searchForEdges == SearchForEdges::end)
        {
            m_SeamState = SeamState::WaitingFirstEndEdge;
        }
        if (m_SeamState != SeamState::WaitingFirstEndEdge)
        {
            m_SeamState = SeamState::Invalid;
            assert(oEvaluation == ImageStateEvaluation::Unknown);
        }
        break;
    case ImageState::Unknown:
        m_SeamState = SeamState::Invalid;
        assert(oEvaluation == ImageStateEvaluation::Unknown);
        break;
    }

    assert((mTransitionFromBackground_mm >= 0 || mTransitionFromFullImage_mm < 0) && "end transition can't be valid without a start transition");

    assert(isSeamStateConsistent());
    return oEvaluation;

}


void StartEndDetectionInSeam::updateOnSeamPositionInfo(SeamPositionInfo info)
{
    m_assumeFullImage = (info == SeamPositionInfo::Middle);
}


void StartEndDetectionInSeam::process(const image::BImage& image,
                                      int imageCounter, double imagePosition_mm,
                                      double pixel_to_mm, int offsetX, int offsetY, int sensorImageHeight)
{
    //m_lastProcessedInputImage = InputImageInfo{imageCounter, imagePosition_mm, pixel_to_mm, offsetX, offsetY};
    m_lastImageContext.imageCounter = imageCounter;
    m_lastImageContext.imagePosition_mm = imagePosition_mm;
    m_lastImageContext.pixel_to_mm = pixel_to_mm;
    m_lastImageContext.offsetX = offsetX;
    m_lastImageContext.offsetY = offsetY;
    m_lastImageContext.sensorImageHeight = sensorImageHeight;

    if (m_assumeFullImage)
    {
        m_lastStartEndDetectionInImage.processAsFullImage(image.size());
    }
    else
    {
        //should we expect Background on top, bottom or both?
        auto oNextEdgeParameters = nextEdgeParameters();
        m_lastStartEndDetectionInImage.process(image, std::get<1>(oNextEdgeParameters), m_minStripesMaterial);
    }

    m_lastImageStateEvaluation = processResult(m_lastStartEndDetectionInImage.m_lastResult, m_lastImageContext);

    assert(isSeamStateConsistent());
}

geo2d::StartEndInfo StartEndDetectionInSeam::getLastImageStartEndInfo(int offset) const
{

    auto oInfo = m_lastStartEndDetectionInImage.getLastImageStartEndInfo(offset);
    //now fill the remaining fields
    oInfo.m_oImageStateEvaluation = m_lastImageStateEvaluation;

    return oInfo;
}


StartEndDetectionInSeam::EdgeMisalignment StartEndDetectionInSeam::computeEdgeMisalignment(bool startEdge)
{
    assert(isSeamStateConsistent());
    typedef EdgePositionIndex idx;

    switch (m_SeamState)
    {
        case (SeamState::Unknown):
        case (SeamState::Invalid):
        case (SeamState::WaitingFirstStartEdge):
        case (SeamState::WaitingFirstEndEdge):
        case (SeamState::FirstStartEdgeFound):
        case (SeamState::FullImageFound_StartMissing):
        case (SeamState::FirstEndEdgeFound_StartMissing):
        case (SeamState::EndBackgroundImageFound_StartEndMissing):
            return {};

        case (SeamState::SecondStartEdgeFound):
        case (SeamState::FullImageFound):
        case (SeamState::FirstEndEdgeFound):
        case (SeamState::FullStartEdgeFound):
        case (SeamState::FullStartEdgeFound_EndMissing):
            if (startEdge)
            {
                auto &rLeftEdge = m_EdgePositions[idx::eLeftStartEdge];
                auto & rRightEdge = m_EdgePositions[idx::eRightStartEdge];
                return {rLeftEdge, rRightEdge};
            }
            else
            {
                //end not found yet
                return {};
            }

        case (SeamState::SecondEndEdgeFound):
        case (SeamState::EndBackgroundImageFound):
        case (SeamState::FullEndEdgeFound):
            if (startEdge)
            {
                auto &rLeftEdge = m_EdgePositions[idx::eLeftStartEdge];
                auto & rRightEdge = m_EdgePositions[idx::eRightStartEdge];
                return {rLeftEdge, rRightEdge};
            }
            else
            {
                auto &rLeftEdge = m_EdgePositions[idx::eLeftEndEdge];
                auto & rRightEdge = m_EdgePositions[idx::eRightEndEdge];
                return {rLeftEdge, rRightEdge};
            }

        case (SeamState::SecondEndEdgeFound_StartMissing):
        case (SeamState::FullEndEdgeFound_StartMissing):
        case (SeamState::EndBackgroundImageFound_StartMissing):
            if (startEdge)
            {
                return {};
            }
            else
            {
                auto &rLeftEdge = m_EdgePositions[idx::eLeftEndEdge];
                auto & rRightEdge = m_EdgePositions[idx::eRightEndEdge];
                return {rLeftEdge, rRightEdge};
            }

        case (SeamState::EndBackgroundImageFound_EndMissing):
            if (startEdge)
            {
                return {};
            }
            else
            {
                auto &rLeftEdge = m_EdgePositions[idx::eLeftEndEdge];
                auto & rRightEdge = m_EdgePositions[idx::eRightEndEdge];
                return {rLeftEdge, rRightEdge};
            }

        }

    assert(false);
    return {};
}


bool StartEndDetectionInSeam::startEdgesFound() const
{
    return ((m_EdgePositions[eLeftStartEdge].found() || m_EdgePositions[eLeftStartEdge].m_edgeLocalPosition.mAppearance == Appearance::NotAvailable)
            && (m_EdgePositions[eRightStartEdge].found() || m_EdgePositions[eRightStartEdge].m_edgeLocalPosition.mAppearance == Appearance::NotAvailable));
}

bool StartEndDetectionInSeam::endEdgesFound() const
{
    return ((m_EdgePositions[eLeftEndEdge].found() || m_EdgePositions[eLeftEndEdge].m_edgeLocalPosition.mAppearance == Appearance::NotAvailable)
            && (m_EdgePositions[eRightEndEdge].found() || m_EdgePositions[eRightEndEdge].m_edgeLocalPosition.mAppearance == Appearance::NotAvailable));
}

std::vector<std::pair<geo2d::DPoint, bool> > StartEndDetectionInSeam::getStripesResultInLastImage(bool left) const
{
    auto & rStripes = m_lastStartEndDetectionInImage.viewStripes(left);
    auto & rStripePositioning = m_lastStartEndDetectionInImage.viewStripePositionSearch(left);

    std::vector<std::pair<geo2d::DPoint, bool> > oResult;
    oResult.reserve(rStripes.size());

    for ( StripePositioning::Index index = 0, last = rStripes.size(); index < last; index++ )
    {
        auto oPointImage = rStripePositioning.computeStripeCoordinateInImage(index, StripePositioning::StripeClip::noClip);
        auto itStripe = rStripes.find(index);
        bool isTube = itStripe != rStripes.end() ? itStripe->second == ImageStripeEvaluation::Tube : false;

        oResult.push_back({oPointImage, isTube});
    }
    return oResult;
}


EdgeSearch StartEndDetectionInSeam::searchDirectionStartEdge() const
{
    switch (m_Direction)
    {
        case Direction::fromBelow:
            return EdgeSearch::OnlyBackgroundOnBottom;
        case Direction::fromAbove:
            return EdgeSearch::OnlyBackgroundOnTop;
        case Direction::Unknown:
            return EdgeSearch::BothDirections;
    };
    assert(false && "not reachable if all cases handled");
    return EdgeSearch::BothDirections;
}

EdgeSearch StartEndDetectionInSeam::searchDirectionEndEdge() const
{
    switch ( m_Direction )
    {
        case Direction::Unknown:
            return EdgeSearch::BothDirections;
        case Direction::fromBelow:
            return EdgeSearch::OnlyBackgroundOnTop;
        case Direction::fromAbove:
            return EdgeSearch::OnlyBackgroundOnBottom;
    }
    assert(false && "not reachable if all cases handled");
    return EdgeSearch::BothDirections;
}

StartEndDetectionInSeam::WaitingForEdge StartEndDetectionInSeam::waitingForEdge() const
{
    switch(m_SeamState)
    {
    case SeamState::Invalid:
        return WaitingForEdge::None;

    case SeamState::Unknown:
        return (m_searchForEdges == SearchForEdges::end ? WaitingForEdge::End : WaitingForEdge::Start);

    case SeamState::WaitingFirstStartEdge: //background - not yet started
    case SeamState::FirstStartEdgeFound:
    case SeamState::SecondStartEdgeFound:
    case SeamState::FullStartEdgeFound:
        return WaitingForEdge::Start;

    case SeamState::WaitingFirstEndEdge:
        return WaitingForEdge::End;

    case SeamState::FullImageFound:
    case SeamState::FullImageFound_StartMissing: // piece started but one or more edges not found
    case SeamState::FirstEndEdgeFound:
        if (m_searchForEdges == SearchForEdges::start) return WaitingForEdge::None; // In case start is missing
        return WaitingForEdge::End;

    case SeamState::SecondEndEdgeFound:
    case SeamState::FirstEndEdgeFound_StartMissing:
    case SeamState::SecondEndEdgeFound_StartMissing:
    case SeamState::FullEndEdgeFound:
    case SeamState::FullEndEdgeFound_StartMissing:
        return WaitingForEdge::End;
    //backgrSeamState::ound - ended
    case SeamState::EndBackgroundImageFound:
    case SeamState::EndBackgroundImageFound_StartMissing:
    case SeamState::EndBackgroundImageFound_StartEndMissing: // piece ended but one or more edges not fo
    case SeamState::EndBackgroundImageFound_EndMissing:  // piece ended but one or more edges not found
    case SeamState::FullStartEdgeFound_EndMissing:
        return WaitingForEdge::None;
    }
    assert(false && "not reachable if all cases handled");
    return WaitingForEdge::None;
}

EdgePositionInSeam::EdgePositionInSeam(EdgePositionInImage p_EdgePositionInImage, const InputImageContext& inputContext,
        bool measureDistanceFromBottomBorder )
:m_edgeLocalPosition(p_EdgePositionInImage), m_imageNumber(inputContext.imageCounter), m_contextImage_mm(inputContext.imagePosition_mm)
{
    if (m_edgeLocalPosition.valid())
    {
        assert(p_EdgePositionInImage.line.getY(512) != -1);
        //we assume that the slope is the same for the left and right edge (see also assertion s in EdgeMisalignment),
        //threfore it's enough to use always x = 0
        double y = (p_EdgePositionInImage.line.getY(0) + inputContext.offsetY);
        if (!measureDistanceFromBottomBorder)
        {
            m_distanceImageBorder_mm = y * inputContext.pixel_to_mm;
        }
        else
        {
            //the distance must be computed from the image BOTTOM border
            m_distanceImageBorder_mm = (inputContext.sensorImageHeight -y) * inputContext.pixel_to_mm;
        }
    }
}


EdgePositionInSeam::EdgePositionInSeam() {
    assert(!found());
}


bool EdgePositionInSeam::found() const {
    return m_edgeLocalPosition.valid();
}


EdgePositionInImage StartEndDetectionInSeam::getRightEdgeInLastImage() const
{
    return m_lastStartEndDetectionInImage.m_lastResult.mRightEdgePosition;
}


EdgePositionInImage StartEndDetectionInSeam::getLeftEdgeInLastImage() const
{
    return m_lastStartEndDetectionInImage.m_lastResult.mLeftEdgePosition;
}



std::string StartEndDetectionInSeam::printState() const
{
    typedef geo2d::StartEndInfo::ImageStateEvaluation ImageStateEvaluation;
    std::ostringstream oMsg;
    oMsg << "Direction: ";
    switch ( getDirection() )
    {
    case Direction::fromAbove:
        oMsg << "PartFromBelow";
        break;
    case Direction::fromBelow:
        oMsg << "PartFromAbove";
        break;
    case Direction::Unknown:
        oMsg << "Unknown";
        break;
    }
    oMsg << " Last Image (" << m_lastImageContext.imageCounter << "): ";
        // << to_string(m_lastStartEndDetectionInImage.m_lastResult.mImageValidRange.mImageState);
    if (this->m_lastStartEndDetectionInImage.hasEdge())
    {
        oMsg << " edge visible " ;
    }
    switch( m_lastImageStateEvaluation)
    {
        case ImageStateEvaluation::Unknown: oMsg << "Unknown image state";
        break;
        case ImageStateEvaluation::BackgroundBeforeStart: oMsg << "BackgroundBeforeStart";
        break;
        case ImageStateEvaluation::OnlyMaterial: oMsg << "OnlyMaterial";
        break;
        case ImageStateEvaluation::StartEdge: oMsg << "StartEdge";
        break;
        case ImageStateEvaluation::PartialStartEdgeIgnored: oMsg << "PartialStartEdgeIgnored";
        break;
        case ImageStateEvaluation::EndEdge: oMsg << "EndEdge";
        break;
        case ImageStateEvaluation::PartialEndEdgeIgnored: oMsg << "PartialEndEdgeIgnored";
        break;
        case ImageStateEvaluation::BackgroundAfterEnd: oMsg << "BackgroundAfterEnd";
        break;

    }


    oMsg << " SeamState: " << to_string(m_SeamState);

    auto oNextEdgeInfo = nextEdgeParameters();
    oMsg << " Next valid edge: ";
    switch (std::get<0>(oNextEdgeInfo))
    {
    case WaitingForEdge::Both:
        oMsg << "both";
        break;
    case WaitingForEdge::None:
        oMsg << "none";
        break;
    case WaitingForEdge::Start:
        oMsg << "start" << " (search " << to_string(std::get<1>(oNextEdgeInfo)) << ")";
        break;
    case WaitingForEdge::End:
        oMsg << "end" << " (search " << to_string(std::get<1>(oNextEdgeInfo)) << ")";
        break;
    }

    /*
    oMsg << "Seam info: ";
    oMsg <<"start found? " << std::string(this->startEdgesFound() ? "Y" : "N")
        << " end found? " << std::string(this->endEdgesFound() ? "Y" : "N");
     */
    
    oMsg << std::endl;

    return oMsg.str();
}


void StartEndDetectionInSeam::setDirection(Direction newDirection)
{
    m_Direction = newDirection;
}


StartEndDetectionInSeam::Direction StartEndDetectionInSeam::getDirection() const
{
    return m_Direction;
}


StartEndDetectionInImage::Parameters& StartEndDetectionInSeam::refParameters()
{
    return m_lastStartEndDetectionInImage.m_parameters;
}

void StartEndDetectionInSeam::updateSearchForEdges(int searchForEdges)
{
    switch (searchForEdges)
    {
        case 0:
            m_searchForEdges = SearchForEdges::both;
            break;
        case 1:
            m_searchForEdges = SearchForEdges::start;
            break;
        case 2:
            m_searchForEdges = SearchForEdges::end;
            break;
        default:
            m_searchForEdges = SearchForEdges::both;
    }
}

void StartEndDetectionInSeam::updateThresholds(int oThresholdMaterial, int oThresholdBackground)
{
    auto & rStripeParameters = m_lastStartEndDetectionInImage.m_parameters.m_stripeParameters;
    rStripeParameters.thresholdMaterial = oThresholdMaterial;
    rStripeParameters.thresholdBackground = oThresholdBackground;
    m_lastStartEndDetectionInImage.m_parameters.m_threshMaterialForEdgeRecognition = (oThresholdBackground + oThresholdMaterial)/2;
}

void StartEndDetectionInSeam::updateEdgeAngle (double angleDegrees)
{
    refParameters().m =  (angleDegrees == 0) ? 0.0 : std::tan(math::degreesToRadians(angleDegrees));
}

void StartEndDetectionInSeam::updateOffsetLeftRight(double offset)
{
    m_lastStartEndDetectionInImage.m_parameters.m_offsetLeftRight = offset;
}

bool StartEndDetectionInSeam::waitingForEndEdge() const
{
    return waitingForEdge() == WaitingForEdge::End;
}

} //end namespace start_end_detection


//////////////////////////////////////
// Filter definition
///////////////////////////////////////
using namespace fliplib;

namespace
{
    typedef precitec::filter::start_end_detection::StartEndDetectionInSeam::SeamState SeamState;
    geo2d::Rect computeValidROI(const image::BImage & rImage, const geo2d::StartEndInfo & rInfo, const int & offset = 0)
    {
        if ( !rInfo.isCropped )
        {
            assert(rInfo.m_oStartValidRangeY == 0 && rInfo.m_oEndValidRangeY == rImage.height() - 1);
            auto roiSize = rImage.size();
            roiSize.width -= offset * 2;
            return geo2d::Rect{geo2d::Point(offset, 0), roiSize};
        }

        if ( rInfo.m_oStartValidRangeY == -1 || rInfo.m_oEndValidRangeY == -1 )
        {
            //all background. no valid area
            return geo2d::Rect{0,0,0,0};
        }

        geo2d::Rect roi{geo2d::Point{offset, rInfo.m_oStartValidRangeY},
            geo2d::Size{rImage.width() + 1 - 2 * offset, rInfo.m_oEndValidRangeY - rInfo.m_oStartValidRangeY + 1}};

        return roi;

    }

    geo2d::Doublearray computeStartEndQuality(const SeamState & rSeamState, bool isSeamEnd)
    {
        bool bothStartEndFound = false;
        bool definitive = true;

        switch ( rSeamState )
        {
            case SeamState::Unknown:
                bothStartEndFound = false;
                definitive = false;
                break;
            case SeamState::Invalid:
                bothStartEndFound = false;
                definitive = true;
                break;
            case SeamState::WaitingFirstStartEdge:
            case SeamState::WaitingFirstEndEdge:
            case SeamState::FirstStartEdgeFound:
            case SeamState::SecondStartEdgeFound:
            case SeamState::FullStartEdgeFound:
            case SeamState::FullImageFound:
                bothStartEndFound = false;
                definitive = false;
                break;
            case SeamState::FullImageFound_StartMissing:
            case SeamState::FullStartEdgeFound_EndMissing:
                bothStartEndFound = false;
                definitive = true;
                break;
            case SeamState::FirstEndEdgeFound:
                bothStartEndFound = false;
                definitive = false;
                break;
            case SeamState::SecondEndEdgeFound:
            case SeamState::FullEndEdgeFound:
                bothStartEndFound = true;
                definitive = true;
                break;
            case SeamState::FirstEndEdgeFound_StartMissing:
            case SeamState::SecondEndEdgeFound_StartMissing:
            case SeamState::FullEndEdgeFound_StartMissing:
                bothStartEndFound = false;
                definitive = true;
                break;
            case SeamState::EndBackgroundImageFound:
                bothStartEndFound = true;
                definitive = true;
                break;
            case SeamState::EndBackgroundImageFound_StartMissing:
            case SeamState::EndBackgroundImageFound_StartEndMissing:
            case SeamState::EndBackgroundImageFound_EndMissing:
                bothStartEndFound = false;
                definitive = true;
                break;
        }
        if ( isSeamEnd )
        {
            definitive = true;
        }
        return geo2d::Doublearray{1, bothStartEndFound ? 1.0 : 0.0, definitive ? eRankMax : eRankMin};
    }


}

/*static*/ const std::string StartEndMisalignmentDetection::m_oFilterName = "StartEndMisalignmentDetection";
/*static*/ const int StartEndMisalignmentDetection::m_SetOffsetEndEqualToOffsetStart=-2000;

StartEndMisalignmentDetection::StartEndMisalignmentDetection()
:TransformFilter(m_oFilterName, Poco::UUID{"f8138067-49a3-449e-8814-a46dc475fe2e"}),
m_pPipeInImageFrame(nullptr),
m_pPipeInDirection(nullptr),
m_oPipeOutStartEndInfo(this, "startendinfo"),
m_oPipeOutMisalignmentStart(this, "misalignment_start"),
m_oPipeOutMisalignmentEnd(this, "misalignment_end"),
m_oPipeOutSeamLength(this, "seam_length"),
m_oPipeOutStartend_quality(this, "startend_quality"),
m_oOffsetStart(0),
m_oOffsetEnd(m_SetOffsetEndEqualToOffsetStart),
m_oOffsetLeftRight(0),
m_oThresholdMaterialStart(50),
m_oThresholdMaterialEnd(50),
m_oAngleStartDegrees(0.0),
m_oAngleEndDegrees(0.0),
m_minStripesMaterial(3),
m_oSingleImageDebug(false),
m_oSearchForEdges(0),
m_oNumberOfImagesToCheck(5),
m_oMaxStartImage(0),
m_oMinEndImage(0),
m_oIsSeamEnd(false)
{

    auto & rParameters = m_oStartEndDetectionInSeam.refParameters();

    parameters_.add("ThresholdMaterialStart", Parameter::TYPE_int, m_oThresholdMaterialStart);
    parameters_.add("ThresholdMaterialEnd", Parameter::TYPE_int, m_oThresholdMaterialEnd);
    parameters_.add("Offset", Parameter::TYPE_int, m_oOffsetStart);
    parameters_.add("OffsetEnd", Parameter::TYPE_int, m_SetOffsetEndEqualToOffsetStart);
    parameters_.add("OffsetLeftRight", Parameter::TYPE_int, m_oOffsetLeftRight);
    parameters_.add("SearchWidthLeft", Parameter::TYPE_uint, rParameters.m_searchWidthLeft);
    parameters_.add("SearchWidthRight", Parameter::TYPE_uint, rParameters.m_searchWidthRight);
    parameters_.add("ResolutionForEdgeRecognition", Parameter::TYPE_uint, rParameters.m_resolutionForEdgeRecognition);
    parameters_.add("Angle", Parameter::TYPE_double, m_oAngleStartDegrees);
    parameters_.add("AngleEnd", Parameter::TYPE_double, m_oAngleEndDegrees);
    parameters_.add("MinStripesMaterial", Parameter::TYPE_int, m_minStripesMaterial);
    parameters_.add("SingleImageDebug", Parameter::TYPE_bool, m_oSingleImageDebug);
    parameters_.add("SearchForEdges", Parameter::TYPE_int, m_oSearchForEdges);
    parameters_.add("NumImagesToCheck", Parameter::TYPE_int, m_oNumberOfImagesToCheck);

    rParameters.m_searchWholeImageForBackground = false;

    setInPipeConnectors({{Poco::UUID("41A40A13-32D2-4154-BC64-B6AA1A6FB2D0"), m_pPipeInImageFrame, "image", 1, "image"},
    {Poco::UUID("7F09DF9F-E6F1-477C-B021-4B3ECDE6C327"), m_pPipeInDirection, "direction", 1, "direction"}});
    setOutPipeConnectors({{Poco::UUID("3BC048D2-51C3-496A-9485-96AF123378F0"), &m_oPipeOutStartEndInfo, "startendinfo", 0, "startendinfo"},
    {Poco::UUID("FF53ABEA-5784-41B1-BE8C-E842ECFE999A"), &m_oPipeOutMisalignmentStart, "misalignment_start", 0, "misalignment_start"},
    {Poco::UUID("A40CF3FF-077C-43B4-957C-1635D7A8501F"), &m_oPipeOutMisalignmentEnd, "misalignment_end", 0, "misalignment_end"},
    {Poco::UUID("1049DABC-3428-4402-AC8B-D7A13EFF6966"), &m_oPipeOutSeamLength, "seam_length", 0, "seam_length"},
    {Poco::UUID("C118CD65-D87C-4FB2-892C-ABEBAF4100C6"), &m_oPipeOutStartend_quality, "startend_quality", 0, "startend_quality"}});
    setVariantID(Poco::UUID("f0316df2-4b8d-42df-94d7-d59bb18d008f"));
}

void StartEndMisalignmentDetection::setParameter()
{
    TransformFilter::setParameter();

    m_oThresholdMaterialStart = parameters_.getParameter("ThresholdMaterialStart");
    m_oThresholdMaterialEnd = parameters_.getParameter("ThresholdMaterialEnd");
    m_oOffsetStart = parameters_.getParameter("Offset");
    m_oOffsetEnd = parameters_.getParameter("OffsetEnd");
    if (m_oOffsetEnd == m_SetOffsetEndEqualToOffsetStart)
    {
        //compatibility with old graphs which did not have the distinction between offset start and offset end
        m_oOffsetEnd = m_oOffsetStart;
    }
    m_oOffsetLeftRight = parameters_.getParameter("OffsetLeftRight");

    auto & rParameters = m_oStartEndDetectionInSeam.refParameters();
    rParameters.m_searchWidthLeft = parameters_.getParameter("SearchWidthLeft");
    rParameters.m_searchWidthRight = parameters_.getParameter("SearchWidthRight");
    rParameters.m_resolutionForEdgeRecognition = parameters_.getParameter("ResolutionForEdgeRecognition");

    //check parameter range (should be done by the GUI)
    if ( m_oThresholdMaterialStart < 0 || m_oThresholdMaterialStart > 255 )
    {
        wmLog(eWarning, "Invalid value for Threshold Start (%d), set to default \n", m_oThresholdMaterialStart);
        m_oThresholdMaterialStart = 50;
    }
    if ( m_oThresholdMaterialEnd < 0 || m_oThresholdMaterialEnd > 255 )
    {
        wmLog(eWarning, "Invalid value for Threshold End (%d), set to default \n", m_oThresholdMaterialEnd);
        m_oThresholdMaterialEnd = 50;
    }

    m_oAngleStartDegrees = parameters_.getParameter("Angle");
    m_oAngleEndDegrees = parameters_.getParameter("AngleEnd");
    m_minStripesMaterial = parameters_.getParameter("MinStripesMaterial");
    m_oSingleImageDebug = parameters_.getParameter("SingleImageDebug");
    m_oSearchForEdges = parameters_.getParameter("SearchForEdges").convert<int>();
    m_oNumberOfImagesToCheck = parameters_.getParameter("NumImagesToCheck").convert<int>();
}

void StartEndMisalignmentDetection::paint()
{
    using namespace image;
    using namespace start_end_detection;

    if ( m_oSpTrafo.isNull())
    {
        return;
    }

    if ( m_oVerbosity < VerbosityType::eMedium )
    {
        return;
    }

    bool debugPaint = m_oVerbosity >= VerbosityType::eHigh;

    const auto & rTrafo(*m_oSpTrafo);
    OverlayCanvas &rCanvas(canvas<OverlayCanvas>(m_oCounter));
    OverlayLayer & rLayerContour(rCanvas.getLayerContour());
    OverlayLayer & rLayerPosition(rCanvas.getLayerPosition());
    OverlayLayer & rLayerText(rCanvas.getLayerText());

    auto oROIWithOffset = rTrafo(m_oValidROI);
    rLayerContour.add<OverlayRectangle>(oROIWithOffset, Color::Green());

    auto oLeftEdge = m_oStartEndDetectionInSeam.getLeftEdgeInLastImage();
    auto oRightEdge = m_oStartEndDetectionInSeam.getRightEdgeInLastImage();
    const auto &rSearchWidthLeft = m_oStartEndDetectionInSeam.refParameters().m_searchWidthLeft;
    const auto &rSearchWidthRight = m_oStartEndDetectionInSeam.refParameters().m_searchWidthRight;

    auto dx = rTrafo.dx();
    auto dy = rTrafo.dy();
    auto colorFull = Color::Cyan();
    auto colorTransparent = colorFull;
    colorTransparent.alpha = 125;
    bool extrapolateLine =  true; //always show the extrapolated line, even if only one edge is visible

    const auto XBorderLeft = m_oValidROI.x().start();
    const auto XBorderRight = m_oValidROI.x().end();

    auto fPointToOverlay = [&] (int x, const geo2d::StartEndInfo::FittedLine & rLine)
    {
        int y = static_cast<int>(std::round(rLine.getY(x)));
        return geo2d::Point{x + dx, y + dy};
    };

    if ( oLeftEdge.valid() )
    {
        auto oSegmentStart = fPointToOverlay(XBorderLeft, oLeftEdge.line);
        auto oSegmentEnd = fPointToOverlay(XBorderLeft + rSearchWidthLeft, oLeftEdge.line);

        rLayerContour.add<OverlayLine>(oSegmentStart, oSegmentEnd, debugPaint? colorTransparent: colorFull);
        if ( extrapolateLine )
        {
            auto oSegmentEndInterpolated = fPointToOverlay(XBorderRight, oLeftEdge.line);
            rLayerContour.add<OverlayLine>(oSegmentEnd, oSegmentEndInterpolated, colorTransparent);
        }
        if ( debugPaint )
        {
            for ( auto && point : m_oStartEndDetectionInSeam.viewEdgePointsInLastImage(true) )
            {
                rLayerPosition.add<OverlayPoint>(point.x+dx, point.y+dy, colorFull);
            }
        }

    }
    if ( oRightEdge.valid() )
    {
        auto oSegmentStart = fPointToOverlay(XBorderRight - rSearchWidthRight, oRightEdge.line);
        auto oSegmentEnd = fPointToOverlay(XBorderRight, oRightEdge.line);

        rLayerContour.add<OverlayLine>(oSegmentStart, oSegmentEnd, debugPaint ? colorTransparent : colorFull);
        if ( extrapolateLine )
        {
            auto oSegmentStartInterpolated = fPointToOverlay(XBorderLeft, oRightEdge.line);
            rLayerContour.add<OverlayLine>(oSegmentStartInterpolated, oSegmentEnd, colorTransparent);
        }
        if ( debugPaint )
        {
            for (auto && point : m_oStartEndDetectionInSeam.viewEdgePointsInLastImage(false))
            {
                rLayerPosition.add<OverlayPoint>(point.x + dx, point.y + dy, colorFull);
            }
        }
    }


    std::vector<std::pair<geo2d::DPoint, bool> > oStripes;
    const double & m = m_oStartEndDetectionInSeam.viewDetectionInImageParameters().m;
    auto fDrawStripeCenters = [this, &m, &rLayerPosition, &rTrafo, &fPointToOverlay, &oLeftEdge, &oRightEdge, &oStripes] (bool useLeft)
    {
        oStripes = m_oStartEndDetectionInSeam.getStripesResultInLastImage(useLeft);
        for ( auto & rStripe : oStripes)
        {
            auto & rStripeCenter = rStripe.first;
            auto & rIsTube = rStripe.second;

            geo2d::Point oPoint{int(rStripeCenter.x + 0.5), int(rStripeCenter.y + 0.5)};
            auto oColor = rIsTube ? Color::Green() : Color::Red();
            auto oPointOverlay = rTrafo(oPoint);

            geo2d::StartEndInfo::FittedLine oLine{m, rStripeCenter.y - m * rStripeCenter.x};
            auto oSegmentStart = fPointToOverlay(oPoint.x - 15, oLine);
            auto oSegmentEnd = fPointToOverlay(oPoint.x + 15, oLine);

            rLayerPosition.add<OverlayLine>(oPointOverlay.x, oPointOverlay.y - 5, oPointOverlay.x, oPointOverlay.y + 5, oColor); //vertical
            rLayerPosition.add<OverlayLine>(oSegmentStart, oSegmentEnd, oColor); //oblique
        }
    };

    if (m_oBypassStartEndDetection)
    {
        //in this case the internal classes were not initialized, exit before accesssing them
        return;
    }

    fDrawStripeCenters(true); //left
    fDrawStripeCenters(false); //right

    if ( !debugPaint )
    {
        return;
    }

    geo2d::Rect textRectangle(10, 10, 600, 30);
    if ( m_oBypassStartEndDetection )
    {
        textRectangle.y() += 30;
        rLayerText.add<OverlayText>("Bypass Detection", Font{12}, textRectangle, Color::Cyan());
        return;
    }


    std::string state = m_oStartEndDetectionInSeam.printState();
    wmLog(eInfo, state);

    auto itSplit1 = std::find( state.begin()+ (int)(0.33*state.length()), state.end(), ' ');
    auto itSplit2 = std::find(itSplit1 + (int)(0.33*state.length()), state.end(), ' ');

    auto n1 = itSplit2 != state.end() ? itSplit1 - state.begin() : state.size();
    auto n2 = itSplit2 != state.end() ? itSplit2 - itSplit1 : state.size()-n1;


    for ( auto & rString : {state.substr(0, n1), state.substr(n1, n2), state.substr(n1 + n2)} )
    {
        if ( rString.size() > 0 )
        {
            textRectangle.y() += 30;
            rLayerText.add<OverlayText>(rString, Font{10}, textRectangle, Color::Cyan());
        }
    }

    auto fEdgeOverlayText = [&] (const std::string & name, const EdgePositionInSeam & edge)
    {
        textRectangle.y() += 30;
        if ( edge.found() )
        {
            std::string txt = name + " "
                + std::to_string(int(std::round(edge.m_edgeLocalPosition.line.m))) + " *x + "
                + std::to_string(int(std::round(edge.m_edgeLocalPosition.line.q)))
                + " image " + std::to_string(edge.m_imageNumber) + " [ "
                + std::to_string(edge.m_contextImage_mm) + "  + "
                + std::to_string(edge.m_distanceImageBorder_mm)
                + " mm]";
            rLayerText.add<OverlayText>(txt, Font{20}, textRectangle, Color::Cyan());
        }
    };

    fEdgeOverlayText("Start left", m_oStartEndDetectionInSeam.getLeftStartEdge());
    fEdgeOverlayText("Start right", m_oStartEndDetectionInSeam.getRightStartEdge());
    fEdgeOverlayText("End left", m_oStartEndDetectionInSeam.getLeftEndEdge());
    fEdgeOverlayText("End right", m_oStartEndDetectionInSeam.getRightEndEdge());


    //print threshold values
    if ( oStripes.size() > 0 )
    {
        const auto & rParameters = m_oStartEndDetectionInSeam.refParameters();
        int threshMaterial = rParameters.m_stripeParameters.thresholdMaterial;
        int threshBackground = rParameters.m_stripeParameters.thresholdBackground;

        auto & rFirstStripe = oStripes.front();

        geo2d::Rect textRectangle(int(rFirstStripe.first.x) + 5, int(rFirstStripe.first.y) - 5, 300, 30);

        std::string txtMaterial = "Thres Material " + std::to_string(threshMaterial);
        std::string txtBg = "Thres Bg " + std::to_string(threshBackground);

        rLayerText.add<OverlayText>(txtBg, Font{}, geo2d::Rect((int)(rFirstStripe.first.x) + 5,
            (int)(rFirstStripe.first.y) - 5, 50, 30), Color::m_oButter);

        textRectangle.y() += 50;
        rLayerText.add<OverlayText>(txtMaterial, Font{}, textRectangle, Color::m_oButter);

        if ( oLeftEdge.valid() || oRightEdge.valid() )
        {
            textRectangle.y() += 50;
            std::string txtEdge = std::string("Thres Edge " + std::to_string(rParameters.m_threshMaterialForEdgeRecognition));
            rLayerText.add<OverlayText>(txtEdge, Font{}, textRectangle, Color::m_oButter);
        }
    }

}

void StartEndMisalignmentDetection::arm(const fliplib::ArmStateBase& p_rArmstate)
{
    m_oIsSeamEnd = p_rArmstate.getStateID() == ArmState::eSeamEnd;
    if ( p_rArmstate.getStateID() == ArmState::eSeamStart)
    {
        const auto * pProductData = externalData<analyzer::ProductData>();
        int numTriggers = pProductData ? pProductData->m_oNumTrigger: 0;
        m_oMaxStartImage = std::max(0, m_oNumberOfImagesToCheck - 1);
        m_oMinEndImage = numTriggers - m_oNumberOfImagesToCheck; //negative value allowed, it will be checked when computing SeamPositionInfo

        m_oStartEndDetectionInSeam.resetState();
    }
    //direction is set in the procedd method because it comes from an input pipe
}

bool StartEndMisalignmentDetection::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup)
{
    if ( p_rPipe.tag() == "image" )
    {
        m_pPipeInImageFrame = dynamic_cast<image_pipe_t*>(&p_rPipe);
    }
    else if ( p_rPipe.tag() == "direction" )
    {
        m_pPipeInDirection = dynamic_cast<double_pipe_t*>(&p_rPipe);
    }
    else
    {
        assert(false && "Error in pipes definition");
    }
    return BaseFilter::subscribe(p_rPipe, p_oGroup);

}

void StartEndMisalignmentDetection::proceedGroup(const void* sender, fliplib::PipeGroupEventArgs& e)
{
    using namespace start_end_detection;
    typedef geo2d::StartEndInfo::ImageStateEvaluation ImageStateEvaluation;

    poco_assert_dbg(m_pPipeInImageFrame != nullptr); // to be asserted by graph editor
    poco_assert_dbg(m_pPipeInDirection != nullptr);

    const auto & rFrameIn(m_pPipeInImageFrame->read(m_oCounter));
    const auto & rImage(rFrameIn.data());
    auto & rDirectionIn(m_pPipeInDirection->read(m_oCounter));

    auto oAnalysisResult = std::min(rFrameIn.analysisResult(), rDirectionIn.analysisResult());

    // check input validity
    {
        bool validInput = true;
        if ( rDirectionIn.ref().size() == 0 || inputIsInvalid(rDirectionIn) )
        {
            validInput = false;
            wmLog(eInfo, "Direction input is invalid\n");
        }
        if ( !rImage.isValid() || rImage.width() <= 0 || rImage.height() <= 0 )
        {
            validInput = false;
        }

        if (m_oStartEndDetectionInSeam.refParameters().m_searchWidthLeft == 0 && m_oStartEndDetectionInSeam.refParameters().m_searchWidthRight == 0)
        {
            validInput = false;
        }

        if ( !validInput )
        {
            m_oSpTrafo = nullptr;
            m_oValidROI = geo2d::Rect{};

            const interface::GeoStartEndInfoarray oGeoStartEnd = {
                    rFrameIn.context(),
                    geo2d::TArray<geo2d::StartEndInfo>{1, geo2d::StartEndInfo{}, eRankMin},
                    oAnalysisResult,
                    0.0
                };
            const interface::GeoDoublearray oGeoInvalidDoubleArray = {
                rFrameIn.context(),
                geo2d::Doublearray{1, 0.0, eRankMin},
                oAnalysisResult,
                0.0
            };
            preSignalAction();
            m_oPipeOutStartEndInfo.signal(oGeoStartEnd);
            m_oPipeOutMisalignmentStart.signal(oGeoInvalidDoubleArray);
            m_oPipeOutMisalignmentEnd.signal(oGeoInvalidDoubleArray);
            m_oPipeOutSeamLength.signal(oGeoInvalidDoubleArray);
            m_oPipeOutStartend_quality.signal(oGeoInvalidDoubleArray);

            return;
        }
    }

    auto & rContext = rFrameIn.context();
    m_oSpTrafo = rContext.trafo();

    m_oBypassStartEndDetection = false;
    // read direction
    {
        if ( rDirectionIn.ref().size() != 1 )
        {
            wmLog(eDebug, "Filter '%s': Received %u Direction values. Can only process first element, rest will be discarded.\n", m_oFilterName.c_str(), rDirectionIn.ref().size());
        }

        const auto previousDirection = m_oStartEndDetectionInSeam.getDirection();
        auto newDirection = previousDirection;

        switch ( int(rDirectionIn.ref().getData().front()) )
        {
            case 0:
                newDirection = StartEndDetectionInSeam::Direction::fromBelow;
                break;
            case 1:
                newDirection = StartEndDetectionInSeam::Direction::fromAbove;
                break;
            case -99:
                m_oBypassStartEndDetection = true;
                break;
            default:
                newDirection = StartEndDetectionInSeam::Direction::Unknown;
                break;
        }

        m_oStartEndDetectionInSeam.setDirection(newDirection);
        if ( m_oCounter != 0 && newDirection != previousDirection)
        {
            wmLog(eWarning, "Direction changed in same seam (was %d, now %d) \n", int(previousDirection), int(m_oStartEndDetectionInSeam.getDirection()));
        }
    }



    // complete parametrization and process


    double position_mm = rFrameIn.context().position() / 1000.0;


    geo2d::Point imageCenter(rImage.width() / 2 + m_oSpTrafo->dx() + rContext.HW_ROI_x0,
                             rImage.height() / 2 + m_oSpTrafo->dy() + rContext.HW_ROI_y0);

    auto offsetLeftRight = m_oOffsetLeftRight;
    if (offsetLeftRight > rImage.width() * 0.5)
    {
        offsetLeftRight = 0;
        wmLog(eDebug, "%s: Parameter OffsetLeftRight is too big and therefore set to zero.", m_oFilterName);
    }

    auto & rCalib =  system::CalibDataSingleton::getCalibrationCoords(math::SensorId::eSensorId0);
    int sensorImageHeight = rCalib.getSensorSize().height;

    double pixel_to_mm = rCalib.pixel_to_mm_OnHorizontalPlane(rImage.width() - 1, imageCenter.x, imageCenter.y);

    //if bypassDetection: the output is always a "full image", no further processing
    if ( m_oBypassStartEndDetection )
    {
        double oOutGeoRank = interface::Limit;

        geo2d::StartEndInfo info;
        info.m_oImageState = geo2d::StartEndInfo::ImageState::OnlyMaterial;
        info.m_oImageStateEvaluation = geo2d::StartEndInfo::ImageStateEvaluation::OnlyMaterial;
        info.threshBackground = 0;
        info.threshMaterial = 0;
        info.m_oStartValidRangeY = 0;
        info.m_oEndValidRangeY = rImage.height() - 1;
        info.isTopDark = false;
        info.isBottomDark = false;
        info.isTopMaterial = true;
        info.isBottomMaterial = true;
        info.isCropped = false;
        info.borderBgStripeY = -1;
        info.imageWidth = rImage.width();

        const interface::GeoStartEndInfoarray oGeoStartEnd = {
            rFrameIn.context(),
            geo2d::TArray<geo2d::StartEndInfo>{1, info, eRankMax},
            oAnalysisResult,
            oOutGeoRank
        };

        const interface::GeoDoublearray oGeoMisalignmentStart = {
            rFrameIn.context(),
            geo2d::Doublearray{1, 0, eRankMin},
            oAnalysisResult,
            oOutGeoRank
        };


        const interface::GeoDoublearray oGeoMisalignmentEnd = {
            rFrameIn.context(),
            geo2d::Doublearray{1, 0.0, eRankMin},
            oAnalysisResult,
            oOutGeoRank
        };

        const interface::GeoDoublearray oGeoSeamLength {
            rFrameIn.context(),
                geo2d::Doublearray{1, position_mm, eRankMin},
                oAnalysisResult,
                oOutGeoRank
        };

        const interface::GeoDoublearray oGeoStartEndQuality {
            rFrameIn.context(),
                geo2d::Doublearray{1, 0, eRankMin},
                oAnalysisResult,
                oOutGeoRank
        };

        m_oValidROI = computeValidROI(rImage, info, offsetLeftRight);
        info.imageWidth = rImage.width();

        preSignalAction();
        m_oPipeOutStartEndInfo.signal(oGeoStartEnd);
        m_oPipeOutMisalignmentStart.signal(oGeoMisalignmentStart);
        m_oPipeOutMisalignmentEnd.signal(oGeoMisalignmentEnd);
        m_oPipeOutSeamLength.signal(oGeoSeamLength);
        m_oPipeOutStartend_quality.signal(oGeoStartEndQuality);
        return;
    }


    if (m_oPipeOutMisalignmentStart.linked() || m_oPipeOutMisalignmentEnd.linked() || m_oPipeOutSeamLength.linked())
    {
        if (pixel_to_mm == 0)
        {
            wmLog(eWarning, "Error in calibration, misalignment measurement will always be 0 mm \n");
        }

        if (system::CalibDataSingleton::getCalibrationData(math::SensorId::eSensorId0).getSensorModel() != math::SensorModel::eLinearMagnification)
        {
            wmLog(eWarning, "Misalignment calculation assumes calibration of type LinearMagnification (coax) \n");
            //Coax is a reasonable assumption for gray image analysis, if Scheimpflug calibration is really needed
            //a solution could be to use Calibration3DCoords instead of pixel_to_mm
        }
    }

    if (m_oSingleImageDebug)
    {
        wmLog(eWarning, "Using single image debug, state reset \n");
        m_oStartEndDetectionInSeam.resetState();
        m_oStartEndDetectionInSeam.setDirection(StartEndDetectionInSeam::Direction::Unknown);
    }

    auto getSeamPositionInfo = [&]()
    {
        if (m_oMinEndImage <= m_oMaxStartImage)
        {
            return StartEndDetectionInSeam::SeamPositionInfo::Unknown;
        }
        auto imageNumber = rFrameIn.context().imageNumber();
        if (imageNumber <= m_oMaxStartImage)
        {
            return StartEndDetectionInSeam::SeamPositionInfo::StartCandidate;
        }
        if (imageNumber >= m_oMinEndImage)
        {
            return StartEndDetectionInSeam::SeamPositionInfo::EndCandidate;
        }
        assert(imageNumber > m_oMaxStartImage && imageNumber < m_oMinEndImage);
        return StartEndDetectionInSeam::SeamPositionInfo::Middle;
    };

    m_oStartEndDetectionInSeam.updateOnSeamPositionInfo(getSeamPositionInfo());

    const bool useEndEdgeParameters = m_oStartEndDetectionInSeam.waitingForEndEdge();

    const auto & rThresholdMaterial = useEndEdgeParameters ? m_oThresholdMaterialEnd : m_oThresholdMaterialStart;
    const auto & rOffset = useEndEdgeParameters ? m_oOffsetEnd : m_oOffsetStart ;
    const auto & rEdgeAngleDegrees = useEndEdgeParameters ? m_oAngleEndDegrees : m_oAngleStartDegrees;

    //update internal variables
    m_oStartEndDetectionInSeam.updateSearchForEdges(m_oSearchForEdges);
    m_oStartEndDetectionInSeam.updateOffsetLeftRight(offsetLeftRight);
    m_oStartEndDetectionInSeam.updateThresholds(rThresholdMaterial, rThresholdMaterial);
    m_oStartEndDetectionInSeam.updateEdgeAngle(rEdgeAngleDegrees);
    m_oStartEndDetectionInSeam.updateMinStripesMaterial(m_minStripesMaterial);
    m_oStartEndDetectionInSeam.process(rImage, m_oCounter, position_mm, pixel_to_mm,
                                       m_oSpTrafo->dx()+rContext.HW_ROI_x0 , m_oSpTrafo->dy() + rContext.HW_ROI_y0,
                                       sensorImageHeight);


    //write output

    bool validResult = m_oStartEndDetectionInSeam.getSeamState() != StartEndDetectionInSeam::SeamState::Invalid;
    ValueRankType oOutValueRank = validResult ? eRankMax : eRankMin;
    double oOutGeoRank = validResult ? interface::Limit : interface::NotPresent;

    geo2d::StartEndInfo oOutStartEnd = m_oStartEndDetectionInSeam.getLastImageStartEndInfo(rOffset);
    oOutStartEnd.imageWidth = rImage.width();
    auto oSeamState = m_oStartEndDetectionInSeam.getSeamState();

    StartEndDetectionInSeam::EdgeMisalignment oMisalignmentStart = m_oStartEndDetectionInSeam.computeEdgeMisalignment(true);
    StartEndDetectionInSeam::EdgeMisalignment oMisalignmentEnd = m_oStartEndDetectionInSeam.computeEdgeMisalignment(false);

    double startDistance = m_oStartEndDetectionInSeam.getTransitionFromBackground();
    double endDistance = m_oStartEndDetectionInSeam.getTransitionFromFullImage();
    bool hasSeamLength = true;
    double oSeamLength = 0;
    if ( startDistance >= 0 )
    {
        if ( endDistance >= 0 )
        {
            oSeamLength = endDistance - startDistance;
        }
        else
        {
            //use actual position
            hasSeamLength = false;
            oSeamLength = position_mm - startDistance;
        }
    }
    assert((startDistance >= 0 || endDistance < 0) && "end transition can't be valid without a start transition");

    const interface::GeoStartEndInfoarray oGeoStartEnd = {
            rFrameIn.context(),
            geo2d::TArray<geo2d::StartEndInfo>{1, oOutStartEnd, oOutValueRank},
            oAnalysisResult,
            oOutGeoRank
        };

    bool isValidStartEdge = false;
    bool isValidEndEdge = false;

    switch ( oOutStartEnd.m_oImageStateEvaluation )
    {
        case ImageStateEvaluation::StartEdge:
            isValidStartEdge = oMisalignmentStart.found(); //TODO: use seam state to know whether first/second edge?
            assert(!isValidStartEdge || (oMisalignmentStart.m_leftEdge.m_imageNumber == m_oCounter || oMisalignmentStart.m_rightEdge.m_imageNumber == m_oCounter));
            break;
        case ImageStateEvaluation::EndEdge:
            isValidEndEdge = oMisalignmentEnd.found();  //TODO: use seam state to know whether first/second edge?
            assert(!isValidEndEdge || ( oMisalignmentEnd.m_leftEdge.m_imageNumber == m_oCounter || oMisalignmentEnd.m_rightEdge.m_imageNumber == m_oCounter));
            break;
        case ImageStateEvaluation::Unknown:
        case ImageStateEvaluation::BackgroundBeforeStart:
        case ImageStateEvaluation::OnlyMaterial:
        case ImageStateEvaluation::BackgroundAfterEnd:
        case ImageStateEvaluation::PartialStartEdgeIgnored:
        case ImageStateEvaluation::PartialEndEdgeIgnored:
            assert(!isValidStartEdge && !isValidEndEdge);
            break;
    }

    const interface::GeoDoublearray oGeoMisalignmentStart = {
        rFrameIn.context(),
        isValidStartEdge ? geo2d::Doublearray{1, oMisalignmentStart.misalignment_mm(), eRankMax}
                                      : geo2d::Doublearray{1, 0, eRankMin},
        oAnalysisResult,
        oOutGeoRank
    };


    const interface::GeoDoublearray oGeoMisalignmentEnd = {
        rFrameIn.context(),
        isValidEndEdge ? geo2d::Doublearray{1, oMisalignmentEnd.misalignment_mm(), eRankMax}
                                    : geo2d::Doublearray{1, 0.0, eRankMin},
        oAnalysisResult,
        oOutGeoRank
    };

    const interface::GeoDoublearray oGeoSeamLength {
            rFrameIn.context(),
            geo2d::Doublearray{1, oSeamLength, hasSeamLength ? eRankMax : eRankMin},
            oAnalysisResult,
            oOutGeoRank
    };

    const interface::GeoDoublearray oGeoStartEndQuality {
            rFrameIn.context(),
            computeStartEndQuality(oSeamState, m_oIsSeamEnd),
            oAnalysisResult,
            oOutGeoRank
    };

    m_oValidROI = computeValidROI(rImage, oOutStartEnd, offsetLeftRight);

    preSignalAction();
    m_oPipeOutStartEndInfo.signal(oGeoStartEnd);
    m_oPipeOutMisalignmentStart.signal(oGeoMisalignmentStart);
    m_oPipeOutMisalignmentEnd.signal(oGeoMisalignmentEnd);
    m_oPipeOutSeamLength.signal(oGeoSeamLength);
    m_oPipeOutStartend_quality.signal(oGeoStartEndQuality);

}


}//end namespace
}

