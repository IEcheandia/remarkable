/**
* @file
* @copyright   Precitec Vision GmbH & Co. KG
* @author      MM
* @date        2021
* @brief       Filter that classifies pore candidates into pores and non-pores. Number of pores is the result. A NIO is thrown if a pore was classified.
*/

#pragma once

// WM includes
#include "fliplib/Fliplib.h"
#include "fliplib/TransformFilter.h"
#include "fliplib/SynchronePipe.h"
#include "filter/armStates.h"
#include "geo/range.h"
#include "geo/array.h"
#include "event/results.h"
#include "math/3D/projectiveMathStructures.h"
#include "poreClassifierTypes.h"
// stdlib includes
#include <array>

namespace precitec
{
namespace filter
{
/**
* @brief Filter that checks if image contains a signal and set the pore count.
*/
class FILTER_API PoreClassifierOutputTriple : public fliplib::TransformFilter
{
public:
    PoreClassifierOutputTriple();

    /**
    * @brief Set filter parameters.
    */
    void setParameter() override;

    /**
    * @brief In-pipe registration.
    * @param p_rPipe Reference to pipe that is getting connected to the filter.
    * @param p_oGroup Group number (0 - proceed is called whenever a pipe has a data element, 1 - proceed is only called when all pipes have a data element).
    */
    bool subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup) override;

    /**
    * @brief Arms filter. Enables the filter to react on different arm signals, like seam start.
    * @param p_rArmState Arm state or reason, like seam start.
    */
    void arm(const fliplib::ArmStateBase& p_rArmState) override;

    /**
    * @brief Processing routine.
    * @param p_pSender pointer to
    * @param p_rEvent
    */
    void proceedGroup(const void* p_pSender, fliplib::PipeGroupEventArgs& p_rEvent) override;

    /**
    * @brief Paint overlay output.
    */
    void paint() override;

private:
    class FeatureRange;
    /**
    * @brief	Dilates or erodes a range by a given percentage. Eg [50, 100] and 10 percent -> [45, 110]; -10 percent -> [55, 90].
    */
    void dilateOrErodeByPercent(const FeatureRange& featureRange, int paramScaling, FeatureRange& featureRangeScaled);

    void classifyScaled();
    /**
    * @brief Classifies a blob as pore if all features lie within the given feature range
    */
    uint classify(const FeatureRange& featureRange, double& maxPoreSize, double& minPoreSize, int activeParamSet);

    static int m_poreId;

    typedef fliplib::SynchronePipe<interface::GeoDoublearray>	scalar_pipe_t;

    // In Pipe
    const scalar_pipe_t* m_pPipeInBoundingBoxDX;
    const scalar_pipe_t* m_pPipeInBoundingBoxDY;
    const scalar_pipe_t* m_pPipeInPcRatio;
    const scalar_pipe_t* m_pPipeInGradient;
    const scalar_pipe_t* m_pPipeInSurface;
    const fliplib::SynchronePipe<interface::GeoBlobarray>* m_pPipeInBlob;

    // Out Pipe
    scalar_pipe_t m_oPipeOutPoreCount;
    scalar_pipe_t m_oPipeOutPoreSizeMax;
    scalar_pipe_t m_oPipeOutPoreSizeMin;

    interface::GeoDoublearray   m_oInputBoundingBoxDX;      ///< result received from in-pipe
    interface::GeoDoublearray   m_oInputBoundingBoxDY;      ///< result received from in-pipe
    interface::GeoDoublearray   m_oInputPcRatio;            ///< result received from in-pipe
    interface::GeoDoublearray   m_oInputGradient;           ///< result received from in-pipe
    interface::GeoDoublearray   m_oInputSurface;            ///< result received from in-pipe
    interface::GeoBlobarray     m_oInputBlob;               ///< result received from in-pipe

    interface::SmpTrafo m_oSpTrafo; ///< roi translation
    int m_imageNumber;

    class FeatureRange {
    public:
        geo2d::Range1d m_oArea;				///< feature range
        geo2d::Range1d m_oBoundingBoxDX;	///< feature range
        geo2d::Range1d m_oBoundingBoxDY;	///< feature range
        geo2d::Range1d m_oPcRatio;			///< feature range
        geo2d::Range1d m_oGradient;			///< feature range
        geo2d::Range1d m_oSurface;			///< feature range
    };
    int m_activeParameterSets;
    FeatureRange m_featureRange, m_featureRange1, m_featureRange2;
    std::vector<FeatureRange> m_featureRangeScaled;
    int  m_paramScaling, m_paramScaling1, m_paramScaling2;


    typedef std::array<std::vector<PoreClassType>, eNbFeatureTypes> pore_class_feature_array_t;

    std::vector<double> m_oArea;                                ///< size (area) feature vector, which is filled from blob list.
    std::vector<pore_class_feature_array_t> m_oClassByFeature;  ///< Classification by feature results for candidate array.
    std::vector<std::vector<PoreClassType>> m_oClassMerged;     ///< Classification results for candidate array.
    std::vector<int> m_oClassMergedRank;                        ///< Classification results rank for candidate array.
};

}
}
