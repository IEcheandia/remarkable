/**
*  @copyright:		Precitec Vision GmbH & Co. KG
*  @author			LB
*  @date			2019
*  @file
*  @brief			Computes notch size
*/

#ifndef NOTCHSIZE_H
#define NOTCHSIZE_H

#include "fliplib/Fliplib.h"
#include "fliplib/TransformFilter.h"
#include "fliplib/SynchronePipe.h"

#include "system/types.h"
#include "common/frame.h"
#include "filter/parameterEnums.h"
#include "image/image.h"

#include "startEndMisalignmentDetection.h" 
#include <math/2D/parabolaEquation.h>

namespace precitec
{
namespace filter
{

class NotchSize; //forward declaration for friend definition
    
namespace start_end_detection{
class NotchSizeCalculator
{
    friend  NotchSize;
    
public:
    enum class ReferencePosition {
        BetweenEdges,InternalEdge,ExternalEdge
    };
    struct FittedParabola
    {
        double a; double b; double c; int index_left; int index_right;
    };

    void reset();

    bool init(const image::BImage & rImage, int liNaht, int reNaht,
        int LastStripeOnTubeIndex, const StripePositioning & rStripePositioning,
        BackgroundPosition backgroundPosition);
    
    bool init(const image::BImage & rImage, int liNaht, int reNaht, int threshTube, 
        geo2d::StartEndInfo::FittedLine lineEdgeLeft, geo2d::StartEndInfo::FittedLine lineEdgeRight,
        BackgroundPosition backgroundPosition);

    bool calc(const image::BImage & rImage);
    
    std::pair<double, bool> getNotchSize() const;
    std::pair<int, int> getFitXBounds() const;
    double getYReference() const;
    geo2d::DPoint getIndentedPoint() const;
    const std::string & viewErrorMessage() const;
    bool isInitialized() const
    {
        return mInitialized;
    }
    
    //parameters
    ReferencePosition mReferencePosition;
    int m_distImgBorderX;
    
private:
    
    bool initializeStart(const image::BImage & rImage);
    
    template<BackgroundPosition tBackgroundPosition>
    void searchNotchPoints(std::vector<geo2d::Point> & rPoints, const image::BImage & rImage) const;
    
    FittedParabola fitNotchParabola(const std::vector<geo2d::Point> & rPoints, precitec::math::ParabolaEquation & rParabola);

    template<BackgroundPosition tBackgroundPosition>
    bool calc(const image::BImage & rImage);   //ID2 iLastStripeOnTube_ + 1
    
    static const int NOTCH_WINDOW_H = 2;
    static const int NOTCH_WINDOW_W = 5;
    static const int HEIGHT_SEARCH_NOTCH = 80;

    //intermediate results
    BackgroundPosition mBackgroundPosition;
    int mLiNaht;
    int mReNaht;
    double mYStart;
    int mThresholdIntensityNotchInWindow;
    geo2d::StartEndInfo::FittedLine mLeftEdge;
    geo2d::StartEndInfo::FittedLine mRightEdge;
    geo2d::StartEndInfo::FittedLine mMeanEdge;
    std::string m_errorMessage;

    bool mInitialized = false;

    FittedParabola mFittedNotch;
    std::vector<geo2d::Point> mNotchPoints;
    precitec::math::ParabolaEquation mNotchParabola;
    double mYReference;
    geo2d::DPoint mIndentation;
    bool mComputed = false;

};

} //end namespace start_end_detection

class FILTER_API NotchSize : public fliplib::TransformFilter 
{

public:
    static const std::string m_oFilterName;		///< Filter name.
    NotchSize();
    ~NotchSize() {}

    void setParameter();
    void paint();

private:

    typedef fliplib::SynchronePipe<interface::ImageFrame> image_pipe_t;
    typedef fliplib::SynchronePipe<interface::GeoDoublearray>  double_pipe_t;
    typedef fliplib::SynchronePipe<interface::GeoStartEndInfoarray> startendinfo_pipe_t;

    bool subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup);
    void proceedGroup(const void* sender, fliplib::PipeGroupEventArgs& e);

    // pipes
    const image_pipe_t* m_pPipeInImageFrame;
    const startendinfo_pipe_t* m_pPipeInStartEndInfo;
    const double_pipe_t* m_pPipeInXSeamLeft;
    const double_pipe_t* m_pPipeInXSeamRight;
    double_pipe_t m_oPipeOutNotchSizeStart;
    double_pipe_t m_oPipeOutNotchSizeEnd;

    // parameters
    int m_oOffset;
    int m_oThresholdSeam;

    //internal structures
    bool m_hasEdge;
    start_end_detection::NotchSizeCalculator m_oNotchSizeCalculator;
    interface::SmpTrafo m_oSpTrafo;
};

} //end namespaces
}

#endif
