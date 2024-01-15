#ifndef STARTENDETECTIONINIMAGE_H
#define STARTENDETECTIONINIMAGE_H

#include "startEndDetectionData.h"
#include <image/image.h>

namespace precitec
{
namespace filter
{
namespace start_end_detection
{

enum  class BackgroundPosition {Top, Bottom};

class StripePositioning
{
public:
    typedef int Index;

    struct Parameters // parameters necessary to  get the stripes position
    {
        double x;
        int width;

        double m_stripe_inclination;
        static const int m_stripe_height = 25; //INT_HEIGHT_BEGIN_END_STRIPES 

        enum StripePositionStrategy {
            AlignTopOnlyFullStripes
        }; //only AlignTop implemented , other possible values could be center, alignBottom
        static const StripePositionStrategy m_StripePositionStrategy = StripePositionStrategy::AlignTopOnlyFullStripes;
    };
    enum StripeClip {
        clipFirstIndex, clipLastIndex, clipBoth, noClip
    };

    
    StripePositioning(); //default constructor ROIValidRange
    StripePositioning(const image::Size2d & p_rImageSize, const Parameters & p_rROI);
    
    //conversions between stripe index and image coordinates
    StripePositioning::Index computeStripeIndexFromImage(double xImage, double yImage) const;
    geo2d::DPoint computeStripeCoordinateInImage(Index index, StripeClip p_direction /*= clipBoth*/) const;
    geo2d::StartEndInfo::FittedLine computeStripeMidlineInImage(Index index) const;
    
    geo2d::DPoint computeStripeCenterOnROIFromIndex(Index index) const;  //center on ROI    
    
    int getNumberOfStripes() const;
    const Parameters & viewParameters() const;
    const image::Size2d & viewImageSize() const;

    
private:

    //conversions between stripe index and roi coordinates
    static geo2d::DPoint computeStripeCenterOnROI(StripePositioning::Index index, const Parameters & rROI);
    //-1 coordinate not in image 
    Index getStripeIndexFromROI(double xROI, double yROI) const;
    
    image::Size2d m_oImageSize;
    Parameters m_parameters;
    int m_numStripes;
};

struct ROIValidRange
{
    Appearance mAppearance = Appearance::Unknown;
    StripePositioning::Index borderBackgroundStripe = -1; // it has a meaning only if BackgroundOnBottom or BackgroundOnTop - approximate position of the edge
    StripePositioning mStripePositioning;
    int m_oStartValidRangeY = -1;
    int m_oEndValidRangeY = 1;
    operator std::string() const;
};

enum class ImageStripeEvaluation{
    NotEvaluated, Tube, NotTube
};
typedef std::map<StripePositioning::Index, ImageStripeEvaluation> StripesResult;

enum class EdgeSearch {
    BothDirections, OnlyBackgroundOnTop, OnlyBackgroundOnBottom
};

struct ImageStripeCalculator
{
    struct ImageStripe
    {
        bool valid = false;

        int mean = 0;
        int min = 0;
        int max = 0;
        int m_AnzRohr = 0;
        int m_AnzBG = 0;
    };
    
    
    int thresholdMaterial = 50;
    int thresholdBackground = 50;

    int speederX = 10;
    int speederY = 3;
    int ThreshMaxMinDiff = 30;
    
    int getThreshold() const
    {
        return (thresholdMaterial + thresholdBackground) / 2;
    }
    ImageStripe computeSingleStripe(const image::BImage & p_rImage, StripePositioning::Index index,
        const StripePositioning & rStripePositioning ) const;
    
    void examineStripes(StripesResult & rStripeResult, 
        const precitec::image::BImage & rImage, const StripePositioning & rStripePositioning) const;

};


class EdgeCalculator
{
public:
    void calcEdgePosition(const image::BImage & rImage, const ROIValidRange & rValidRange, int x, int width);
    int m_threshMaterial;
    int m_windowWidth = 4;
    int m_windowHeight = 4;
    const EdgePositionInImage & viewlastEdgePositionInImage() const;
    const std::vector<geo2d::Point> & viewlastEdgePoints() const;
    
private:
    typedef geo2d::StartEndInfo::FittedLine FittedLine;
    
    template<BackgroundPosition tBackgroundPosition>
    void calcEdgePosition(const image::BImage & rImage, FittedLine midLine, int searchAbove, int searchBelow,  int minX, int maxX);

    void calcEdgePositionFromThreshold(const image::BImage & rImage, int minY, int maxY, int minX, int maxX, BackgroundPosition p_BackgroundPosition);


    static const int m_STRIPE_OFFSET = 10; 

    EdgePositionInImage m_lastEdgePositionInImage;
    std::vector<geo2d::Point> m_lastPoints;
};


class ImageValidRange
{
public:
    ImageValidRange(ROIValidRange p_LeftValidRange, ROIValidRange p_RightValidRange);
    
    bool isEdgeVisible() const;
    bool hasBackgroundOnTop() const; //true if edgevisible and background on top
    bool hasBackgroundOnBottom() const; //true if edgevisible and background on bottom
    void reset();
    void setLeftValidRange(ROIValidRange range);
    void setRightValidRange(ROIValidRange range);
    const ROIValidRange & viewSubRange(bool useLeft) const;
    const geo2d::StartEndInfo::ImageState & viewImageState() const;
    
private:
    geo2d::StartEndInfo::ImageState computeImageState() const;
    geo2d::StartEndInfo::ImageState mImageState;
    ROIValidRange mLeftValidRange;
    ROIValidRange mRightValidRange;
};

class StartEndDetectionInImage
{
public:
    struct Parameters
    {
        int m_searchWidthLeft = 100;
        int m_searchWidthRight = 100;
        int m_threshMaterialForEdgeRecognition = 50;
        unsigned int m_resolutionForEdgeRecognition = 5;
        bool m_searchWholeImageForBackground = false; // This is never true. It is some holdover from the old filter StartEndDetection which searched in the whole image not just in two stripes.
        EdgeSearch m_EdgeSearch;
        double m = 0.0; ///< inclination //FIXME
        ImageStripeCalculator m_stripeParameters; //FIXME;
        double m_offsetLeftRight = 0.;
    };
    struct Result
    {
        ImageValidRange mImageValidRange;
        EdgePositionInImage mLeftEdgePosition;
        EdgePositionInImage mRightEdgePosition;
        void reset();
        Result();
        Result(ImageValidRange p_oImageValidRange, EdgePositionInImage p_oLeftEdgePosition, EdgePositionInImage p_oRightEdgePosition);            
    };
    void process(const image::BImage& image, EdgeSearch p_EdgeSearch, int minStripesMaterial);
    void processAsFullImage(image::Size2d imageSize);
    void resetState();
    const StripesResult & viewStripes(bool left) const;
    const StripePositioning & viewStripePositionSearch(bool left) const;
    const std::vector<geo2d::Point> & viewLastEdgePoints(bool left) const;
    geo2d::StartEndInfo getLastImageStartEndInfo(int offset) const;    
    bool hasEdge() const;
    
//public for quick access //TODO
    Parameters m_parameters;
    Result m_lastResult;

private:
    
    void initializeSearchAreaStripes(const precitec::image::BImage & image);
    void initializeSearchAreaEdge(const precitec::image::BImage & image);
    
    void detectValidRange(const precitec::image::BImage & image, EdgeSearch p_EdgeSearch, int minStripesMaterial);
    void detectEdgePosition(const precitec::image::BImage & image,EdgeSearch p_EdgeSearch);
    void updateValidRangeFromEdges(const precitec::image::Size2d & imageSize);


    //intermediate results
    std::vector <std::pair<StripePositioning, StripesResult> > m_StripesSearchResult;
    
    struct PartialEdgeSearch
    {
        int x;
        int width;
        std::vector<geo2d::Point> points;
    };
    std::array<PartialEdgeSearch,2> m_EdgeSearchResults;
    
};


} //end namespaces
}
}

#endif
