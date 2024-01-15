#ifndef  STARTENDMISALIGNMENTDETECTION_H
#define STARTENDMISALIGNMENTDETECTION_H

#include "startEndDetectionInImage.h"

#include <common/frame.h>				///< ImageFrame
#include <image/image.h>				///< BImage
#include <geo/geo.h>					///< Size2d, Intarray
#include <geo/array.h>					///< ByteArray

#include "fliplib/Fliplib.h"			// export macro
#include "fliplib/TransformFilter.h"	// base class
#include "fliplib/SynchronePipe.h"		// in- / output


namespace precitec
{
namespace filter
{

namespace start_end_detection
{    

struct InputImageContext
{
    int imageCounter = -1;
    double imagePosition_mm = -1.0;
    double pixel_to_mm = 0.0;
    int offsetX = 0;//trafo->dx() + HWROI_x
    int offsetY = 0; //trafo->dy() + HWROI_y
    int sensorImageHeight = 0;
};

struct EdgePositionInSeam  
{
    EdgePositionInImage m_edgeLocalPosition;
    int m_imageNumber;
    double m_contextImage_mm; //typically imageCounter, input of process method
    double m_distanceImageBorder_mm;
    bool found() const;
    EdgePositionInSeam();
    EdgePositionInSeam(EdgePositionInImage p_EdgePositionInImage, const InputImageContext & inputContext,
        bool measureDistanceFromBottomBorder);
};

class StartEndDetectionInSeam
{
public:
    enum class SeamPositionInfo
    {
        StartCandidate, Middle, EndCandidate, Unknown
    };
    enum class Direction
    {
        fromBelow, fromAbove,  // see also SoutubeHandler, SeamConstant bDirection = 0 = "von Unten" // bDirection = 1 = "von Oben"
        Unknown // edges will be searched in both directions
    }; 

    enum class EdgeMisalignmentMode
    {
        UsePartialEdge, IgnorePartialEdgeAfterFullEdge
    };
    
    enum class SeamState{
        Unknown, Invalid,
        WaitingFirstStartEdge, //background - not yet started
        WaitingFirstEndEdge, ///< only used if parameter searchForEdges == end

        FirstStartEdgeFound, 
        SecondStartEdgeFound,
        FullStartEdgeFound,
        FullStartEdgeFound_EndMissing, // piece started but no end edge found because it should not search for an end
        
        FullImageFound,
        FullImageFound_StartMissing, // piece started but one or more edges not found
                
        FirstEndEdgeFound,
        SecondEndEdgeFound,
        FirstEndEdgeFound_StartMissing,
        SecondEndEdgeFound_StartMissing,
        FullEndEdgeFound,
        FullEndEdgeFound_StartMissing,
        
        EndBackgroundImageFound,
        EndBackgroundImageFound_StartMissing,
        EndBackgroundImageFound_StartEndMissing, // piece ended but one or more edges not fo
        EndBackgroundImageFound_EndMissing  // piece ended but one or more edges not found
    };

    enum class SearchForEdges
    {
        both,
        start,
        end
    };

    struct EdgeMisalignment
    {
        EdgePositionInSeam m_leftEdge;
        EdgePositionInSeam m_rightEdge;
        Appearance getAppearance() const;
        double misalignment_mm () const;
        double distanceFromSeamStart(bool useY, bool isStartEdge) const;
        EdgeMisalignment();
        EdgeMisalignment(const EdgePositionInSeam & rLeftEdge, const EdgePositionInSeam & rRightEdge);
        bool found() const { return m_leftEdge.found() && m_rightEdge.found(); }
    };
    

    StartEndDetectionInSeam();

    void resetState();

    //force state base on position in seam
    void updateOnSeamPositionInfo (SeamPositionInfo info);

    void process(const precitec::image::BImage & image, 
                 int imageCounter, double imagePosition_mm, 
                 double pixel_to_mm, 
                 int offsetX, int offsetY, int imageHeight);
    

    //setters and getters
    void updateSearchForEdges(int searchForEdges);
    void updateThresholds(int oThresholdMaterial, int oThresholdBackground);
    void updateEdgeAngle (double angleDegrees);
    void updateMinStripesMaterial(unsigned int minStripesMaterial)
    {
        m_minStripesMaterial = minStripesMaterial;
    }
    void updateOffsetLeftRight(double offset);
    
    StartEndDetectionInImage::Parameters & refParameters(); //not const!;
    Direction getDirection() const;
    void setDirection(Direction newDirection);
    
    EdgeSearch searchDirectionStartEdge() const;
    EdgeSearch searchDirectionEndEdge() const;
    
    geo2d::StartEndInfo getLastImageStartEndInfo(int offset) const;
    EdgeMisalignment computeEdgeMisalignment(bool startEdge);
    
    double getTransitionFromBackground() const
    {
        return mTransitionFromBackground_mm;
    }
    double getTransitionFromFullImage() const
    {
        return mTransitionFromFullImage_mm;
    }
    
    //for debugging the current state
    bool alwaysAcceptPartialEdges() const { return m_EdgeMisalignmentMode == EdgeMisalignmentMode::UsePartialEdge;}
    bool waitingForEndEdge() const;
    std::string printState() const;
    bool startEdgesFound() const;
    bool endEdgesFound() const;
    SeamState getSeamState() const {return m_SeamState;}
    EdgePositionInSeam getLeftStartEdge() const { return m_EdgePositions[eLeftStartEdge];}
    EdgePositionInSeam getRightStartEdge() const { return m_EdgePositions[eRightStartEdge];}
    EdgePositionInSeam getLeftEndEdge() const { return m_EdgePositions[eLeftEndEdge];}
    EdgePositionInSeam getRightEndEdge() const { return m_EdgePositions[eRightEndEdge];}
    EdgePositionInImage getLeftEdgeInLastImage() const;
    EdgePositionInImage getRightEdgeInLastImage() const;
    ImageValidRange getLastImageValidRange() const
    {
        return m_lastStartEndDetectionInImage.m_lastResult.mImageValidRange;
    }
    std::vector<std::pair<geo2d::DPoint, bool> > getStripesResultInLastImage(bool left) const;  // (y, isTubeStripe)
    const std::vector<geo2d::Point> &  viewEdgePointsInLastImage(bool left) const;
    const StartEndDetectionInImage::Parameters & viewDetectionInImageParameters() const;

private:
    enum class EdgeSide {left, right};
    enum EdgePositionIndex{eLeftStartEdge=0, eRightStartEdge, eLeftEndEdge, eRightEndEdge, 
        NUMPOSITIONS, eInvalid};
    enum WaitingForEdge {Start, End, None, Both};
    
    bool isSeamStateConsistent() const;  //only for test 
    
    //to be called internally in process method
    geo2d::StartEndInfo::ImageStateEvaluation processResult(const StartEndDetectionInImage::Result & rLastResult, const InputImageContext & rImageContext);
    SeamState getNextState_FoundOnlyBackground() const;
    SeamState getNextState_FoundOnlyMaterial() const;
    std::tuple<SeamState,EdgePositionIndex> getNextState_FoundOneEdge(EdgeSide p_EdgeSide) const;
    std::tuple<SeamState,EdgePositionIndex, EdgePositionIndex>  getNextState_FoundEdges() const;

    WaitingForEdge waitingForEdge() const;
    std::tuple<WaitingForEdge, EdgeSearch> nextEdgeParameters() const;
    
    // parameters
    Direction m_Direction = Direction::Unknown;
    EdgeMisalignmentMode m_EdgeMisalignmentMode = EdgeMisalignmentMode::IgnorePartialEdgeAfterFullEdge;
    unsigned int m_minStripesMaterial = 1; //avoids false material detection due to laser line on the backgound

    // state
    SeamState  m_SeamState = SeamState::Unknown;
    std::array<EdgePositionInSeam, EdgePositionIndex::NUMPOSITIONS> m_EdgePositions;
    double mTransitionFromBackground_mm;
    double mTransitionFromFullImage_mm;
    bool m_assumeFullImage;

    InputImageContext m_lastImageContext;
    StartEndDetectionInImage m_lastStartEndDetectionInImage;
    geo2d::StartEndInfo::ImageStateEvaluation m_lastImageStateEvaluation;
    SearchForEdges m_searchForEdges;
};

} //end namespace start_end_detection

class FILTER_API StartEndMisalignmentDetection : public fliplib::TransformFilter
{
public:
    StartEndMisalignmentDetection();
    virtual ~StartEndMisalignmentDetection()
    {}

    static const std::string m_oFilterName;
    static const int m_SetOffsetEndEqualToOffsetStart;

    void setParameter();
    void paint();
    void arm(const fliplib::ArmStateBase& p_rArmstate);

protected:
    typedef fliplib::SynchronePipe<interface::ImageFrame> image_pipe_t;
    typedef fliplib::SynchronePipe<interface::GeoDoublearray>  double_pipe_t;
    typedef fliplib::SynchronePipe<interface::GeoStartEndInfoarray> startendinfo_pipe_t;

    bool subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup);
    void proceedGroup(const void* sender, fliplib::PipeGroupEventArgs& e);
  
    // pipes
    const image_pipe_t* m_pPipeInImageFrame;
    const double_pipe_t* m_pPipeInDirection;
    startendinfo_pipe_t m_oPipeOutStartEndInfo;
    double_pipe_t m_oPipeOutMisalignmentStart;
    double_pipe_t m_oPipeOutMisalignmentEnd;
    double_pipe_t m_oPipeOutSeamLength;
    double_pipe_t m_oPipeOutStartend_quality;

    // parameters
    int m_oOffsetStart;
    int m_oOffsetEnd;
    int m_oOffsetLeftRight;
    int m_oThresholdMaterialStart;
    int m_oThresholdMaterialEnd;
    double m_oAngleStartDegrees;
    double m_oAngleEndDegrees;
    int m_minStripesMaterial;
    bool m_oSingleImageDebug;
    int m_oSearchForEdges; // both edges, only start or only end edge
    int m_oNumberOfImagesToCheck;
    bool m_oBypassStartEndDetection;
    int m_oMaxStartImage;
    int m_oMinEndImage;

    //internal structures
    start_end_detection::StartEndDetectionInSeam m_oStartEndDetectionInSeam;
    interface::SmpTrafo m_oSpTrafo;
    geo2d::Rect m_oValidROI;
    bool m_oIsSeamEnd;

};

}
}

#endif
