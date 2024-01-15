#pragma once

#include "fliplib/Fliplib.h"
#include "fliplib/TransformFilter.h"
#include "fliplib/PipeEventArgs.h"
#include "fliplib/SynchronePipe.h"

#include "filter/armStates.h"
#include "direction.h"
#include "calcContour.h"
#include "majorAxes.h"
#include "poreStatistics.h"
#include "poreClassifierTypes.h"
#include "util/calibDataSingleton.h"

#include "common/frame.h"

namespace precitec
{
namespace filter
{
class FILTER_API PoreDetection : public fliplib::TransformFilter
{
public:
    PoreDetection();
    ~PoreDetection();

    void setParameter() override;
    void paint() override;

    // for testing
    void getFeatures(std::vector<geo2d::Doublearray>& dx, std::vector<geo2d::Doublearray>& dy, std::vector<geo2d::Doublearray>& pcRatio,
                     std::vector<geo2d::Doublearray>& gradient, std::vector<geo2d::Doublearray>& variance) const;

private:
    static const std::string m_filterName;
    static const int m_maxParameterSets = 3;

    void arm(const fliplib::ArmStateBase& p_rArmState) override;
    bool subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup) override;
    void proceed(const void* sender, fliplib::PipeEventArgs& e) override;

    struct FeatureRange {
        geo2d::Range1d boundingBoxDX;
        geo2d::Range1d boundingBoxDY;
        geo2d::Range1d pcRatio;
        geo2d::Range1d gradient;
        geo2d::Range1d surface;
        FeatureRange() : boundingBoxDX(0.1, 3), boundingBoxDY(0.1, 3), pcRatio(0.6, 1.4), gradient(50, 255), surface(0, 500) {}
    };

    enum class Visualize {
        none = 0,
        contour,
        morphologyImage,
        binarizeImage
    };

    void binarize(const image::BImage& image, uint parameterSet);
    void morphology(uint parameterSet);
    geo2d::Doublearray variance(const image::BImage& image, const geo2d::Blobarray& blobs, double boundingBoxScale);
    void calcPoreGradient(const image::BImage& image, const geo2d::Blobarray& blobs, geo2d::Doublearray& poreGradientOut, uint numberNeighbors, uint outerNeighborDistance);
    geo2d::Doublearray calcPrincipalComponents(const image::BImage& image, const geo2d::Blobarray& blobs);
    std::pair<uint, double> classify(const std::vector<double>& area, uint parameterSet);
    void classifyScaled(const std::vector<double>& area, const int parameterScaling, const int parameterSet);

    std::array<geo2d::DataBlobDetectionT, m_maxParameterSets> m_dataBlobDetection; ///< used for segmentationImage

    const fliplib::SynchronePipe<interface::ImageFrame>* m_pipeInImageFrame;
    fliplib::SynchronePipe<interface::GeoDoublearray>    m_pipeOutPoreCount;
    fliplib::SynchronePipe<interface::GeoDoublearray>    m_pipeOutPoreCount1;
    fliplib::SynchronePipe<interface::GeoDoublearray>    m_pipeOutPoreCount2;
    fliplib::SynchronePipe<interface::GeoDoublearray>    m_pipeOutSizeMax;

    interface::SmpTrafo m_smpTrafo; ///< roi translation

    // parameters
    unsigned int m_activeParameterSets;
    std::array<Visualize, m_maxParameterSets>       m_visualize;
    std::array<byte, m_maxParameterSets>            m_distToMeanIntensity;
    std::array<BinarizeType, m_maxParameterSets>    m_binarizeType;
    std::array<uint, m_maxParameterSets>            m_numberIterationsMorphology;
    std::array<uint, m_maxParameterSets>            m_maxNumberBlobs;
    std::array<uint, m_maxParameterSets>            m_minBlobSize; ///< in pixel
    std::array<double, m_maxParameterSets>          m_boundingBoxScale;
    std::array<uint, m_maxParameterSets>            m_numberNeighbors; ///< for gradient determination
    std::array<uint, m_maxParameterSets>            m_outerNeighborDistance; ///< for gradient determination
    std::array<FeatureRange, m_maxParameterSets>    m_featureRange; ///< for classify
    std::array<int, m_maxParameterSets>             m_parameterScaling;

    std::array<FeatureRange, m_maxParameterSets>    m_featureRangeScaled; ///< for classify

    // used for paint
    std::array<std::array<std::vector<PoreClassType>, eNbFeatureTypes-1>, m_maxParameterSets> m_classByFeature; ///< Classification by feature results for candidate array
    std::array<std::vector<PoreClassType>, m_maxParameterSets>                                m_classMerged;    ///< Classification results for candidate array.
    std::array<geo2d::Blobarray, m_maxParameterSets>                                          m_blobs;
    std::array<image::BImage, m_maxParameterSets>                                             m_binaryImage;
    std::array<image::BImage, m_maxParameterSets>                                             m_morphologyImage;
    std::array<geo2d::Doublearray, m_maxParameterSets>                                        m_poreGradient;
    std::array<geo2d::Doublearray, m_maxParameterSets>                                        m_pcRatios;
    std::array<geo2d::Doublearray, m_maxParameterSets>                                        m_surface;
    std::array<geo2d::Doublearray, m_maxParameterSets>                                        m_boundingBoxDX;
    std::array<geo2d::Doublearray, m_maxParameterSets>                                        m_boundingBoxDY;
};
}
}
