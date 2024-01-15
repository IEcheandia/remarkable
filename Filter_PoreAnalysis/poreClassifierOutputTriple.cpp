/**
* @file
* @copyright   Precitec Vision GmbH & Co. KG
* @author      MM
* @date        2021
* @brief       Filter that classifies pore candidates into pores and non-pores. Number of pores is the result. A NIO is thrown if a pore was classified.
*/

#include "poreClassifierOutputTriple.h"
#include "fliplib/PipeEventArgs.h"
#include "fliplib/Parameter.h"
#include "overlay/overlayCanvas.h"
#include "overlay/overlayPrimitive.h"
#include "geo/array.h"
#include "module/moduleLogger.h"
#include "common/defines.h"

#include <algorithm>
#include <functional>
#include <fliplib/TypeToDataTypeImpl.h>

using namespace fliplib;
namespace precitec
{
namespace filter
{
int PoreClassifierOutputTriple::m_poreId = 0;

PoreClassifierOutputTriple::PoreClassifierOutputTriple()
    : TransformFilter("PoreClassifierOutputTriple", Poco::UUID{"bd5159c0-511c-4c3d-a92c-5841809166ef"})
    , m_pPipeInBoundingBoxDX(nullptr)
    , m_pPipeInBoundingBoxDY(nullptr)
    , m_pPipeInPcRatio(nullptr)
    , m_pPipeInGradient(nullptr)
    , m_pPipeInSurface(nullptr)
    , m_pPipeInBlob(nullptr)
    , m_oPipeOutPoreCount(this, "PoreCount")
    , m_oPipeOutPoreSizeMax(this, "maxPoreSize")
    , m_oPipeOutPoreSizeMin(this, "minPoreSize")
    , m_imageNumber(0)
{
    parameters_.add("ActiveParameterSets", Parameter::TYPE_int, m_activeParameterSets);

    parameters_.add("SizeMin", Parameter::TYPE_double, m_featureRange.m_oArea.start());
    parameters_.add("SizeMax", Parameter::TYPE_double, m_featureRange.m_oArea.end());
    parameters_.add("BoundingBoxDXMin", Parameter::TYPE_double, m_featureRange.m_oBoundingBoxDX.start());
    parameters_.add("BoundingBoxDXMax", Parameter::TYPE_double, m_featureRange.m_oBoundingBoxDX.end());
    parameters_.add("BoundingBoxDYMin", Parameter::TYPE_double, m_featureRange.m_oBoundingBoxDY.start());
    parameters_.add("BoundingBoxDYMax", Parameter::TYPE_double, m_featureRange.m_oBoundingBoxDY.end());
    parameters_.add("PcRatioMin", Parameter::TYPE_double, m_featureRange.m_oPcRatio.start());
    parameters_.add("PcRatioMax", Parameter::TYPE_double, m_featureRange.m_oPcRatio.end());
    parameters_.add("GradientMin", Parameter::TYPE_double, m_featureRange.m_oGradient.start());
    parameters_.add("GradientMax", Parameter::TYPE_double, m_featureRange.m_oGradient.end());
    parameters_.add("SurfaceMin", Parameter::TYPE_double, m_featureRange.m_oSurface.start());
    parameters_.add("SurfaceMax", Parameter::TYPE_double, m_featureRange.m_oSurface.end());
    parameters_.add("ParamScaling", Parameter::TYPE_int, m_paramScaling);

    parameters_.add("SizeMin1", Parameter::TYPE_double, m_featureRange1.m_oArea.start());
    parameters_.add("SizeMax1", Parameter::TYPE_double, m_featureRange1.m_oArea.end());
    parameters_.add("BoundingBoxDXMin1", Parameter::TYPE_double, m_featureRange1.m_oBoundingBoxDX.start());
    parameters_.add("BoundingBoxDXMax1", Parameter::TYPE_double, m_featureRange1.m_oBoundingBoxDX.end());
    parameters_.add("BoundingBoxDYMin1", Parameter::TYPE_double, m_featureRange1.m_oBoundingBoxDY.start());
    parameters_.add("BoundingBoxDYMax1", Parameter::TYPE_double, m_featureRange1.m_oBoundingBoxDY.end());
    parameters_.add("PcRatioMin1", Parameter::TYPE_double, m_featureRange1.m_oPcRatio.start());
    parameters_.add("PcRatioMax1", Parameter::TYPE_double, m_featureRange1.m_oPcRatio.end());
    parameters_.add("GradientMin1", Parameter::TYPE_double, m_featureRange1.m_oGradient.start());
    parameters_.add("GradientMax1", Parameter::TYPE_double, m_featureRange1.m_oGradient.end());
    parameters_.add("SurfaceMin1", Parameter::TYPE_double, m_featureRange1.m_oSurface.start());
    parameters_.add("SurfaceMax1", Parameter::TYPE_double, m_featureRange1.m_oSurface.end());
    parameters_.add("ParamScaling1", Parameter::TYPE_int, m_paramScaling1);

    parameters_.add("SizeMin2", Parameter::TYPE_double, m_featureRange2.m_oArea.start());
    parameters_.add("SizeMax2", Parameter::TYPE_double, m_featureRange2.m_oArea.end());
    parameters_.add("BoundingBoxDXMin2", Parameter::TYPE_double, m_featureRange2.m_oBoundingBoxDX.start());
    parameters_.add("BoundingBoxDXMax2", Parameter::TYPE_double, m_featureRange2.m_oBoundingBoxDX.end());
    parameters_.add("BoundingBoxDYMin2", Parameter::TYPE_double, m_featureRange2.m_oBoundingBoxDY.start());
    parameters_.add("BoundingBoxDYMax2", Parameter::TYPE_double, m_featureRange2.m_oBoundingBoxDY.end());
    parameters_.add("PcRatioMin2", Parameter::TYPE_double, m_featureRange2.m_oPcRatio.start());
    parameters_.add("PcRatioMax2", Parameter::TYPE_double, m_featureRange2.m_oPcRatio.end());
    parameters_.add("GradientMin2", Parameter::TYPE_double, m_featureRange2.m_oGradient.start());
    parameters_.add("GradientMax2", Parameter::TYPE_double, m_featureRange2.m_oGradient.end());
    parameters_.add("SurfaceMin2", Parameter::TYPE_double, m_featureRange2.m_oSurface.start());
    parameters_.add("SurfaceMax2", Parameter::TYPE_double, m_featureRange2.m_oSurface.end());
    parameters_.add("ParamScaling2", Parameter::TYPE_int, m_paramScaling2);

    setInPipeConnectors({{Poco::UUID("7b91e68d-443d-4535-88ea-a299f82690ba"), m_pPipeInBoundingBoxDX, "BoundingBoxDXIn", 1, "bounding_box_dx"},
    {Poco::UUID("3fa97ef2-7af9-4527-bc92-01b9487c9498"), m_pPipeInBoundingBoxDY, "BoundingBoxDYIn", 1, "bounding_box_dy"},
    {Poco::UUID("d56a6bda-c511-4a60-96af-5c24aefa77bb"), m_pPipeInPcRatio, "PcRatioIn", 1, "pc_ratio"},
    {Poco::UUID("29cf6210-bebd-49f1-99f3-afb761efb574"), m_pPipeInGradient, "GradientIn", 1, "gradient"},
    {Poco::UUID("ae710c06-962b-47b8-b758-2fa82a7047fd"), m_pPipeInSurface, "SurfaceIn", 1, "surface"},
    {Poco::UUID("a51fb8fa-5bed-484e-b449-5ae3aba41983"), m_pPipeInBlob, "BlobIn", 1, "contour"}});
    setOutPipeConnectors({{Poco::UUID("eb4019c3-3b69-42e7-99d9-233a37336e09"), &m_oPipeOutPoreCount, "PoreCount", 0, ""},
    {Poco::UUID("527d90df-201f-4d84-b636-e152e393e545"), &m_oPipeOutPoreSizeMax, "maxPoreSize", 0, ""},
    {Poco::UUID("52b81bc7-495b-4ea7-a138-df7151ae674b"), &m_oPipeOutPoreSizeMin, "minPoreSize", 0, ""}});
    setVariantID(Poco::UUID("3dfb8921-0013-4fae-a4b0-9abdf4461de6"));
}



void PoreClassifierOutputTriple::setParameter()
{
    TransformFilter::setParameter();

    m_activeParameterSets = parameters_.getParameter("ActiveParameterSets");

    m_featureRange.m_oArea.start() = parameters_.getParameter("SizeMin");
    m_featureRange.m_oArea.end() = parameters_.getParameter("SizeMax");
    m_featureRange.m_oBoundingBoxDX.start() = parameters_.getParameter("BoundingBoxDXMin");
    m_featureRange.m_oBoundingBoxDX.end() = parameters_.getParameter("BoundingBoxDXMax");
    m_featureRange.m_oBoundingBoxDY.start() = parameters_.getParameter("BoundingBoxDYMin");
    m_featureRange.m_oBoundingBoxDY.end() = parameters_.getParameter("BoundingBoxDYMax");
    m_featureRange.m_oPcRatio.start() = parameters_.getParameter("PcRatioMin");
    m_featureRange.m_oPcRatio.end() = parameters_.getParameter("PcRatioMax");
    m_featureRange.m_oGradient.start() = parameters_.getParameter("GradientMin");
    m_featureRange.m_oGradient.end() = parameters_.getParameter("GradientMax");
    m_featureRange.m_oSurface.start() = parameters_.getParameter("SurfaceMin");
    m_featureRange.m_oSurface.end() = parameters_.getParameter("SurfaceMax");
    m_paramScaling = parameters_.getParameter("ParamScaling");

    m_featureRange1.m_oArea.start() = parameters_.getParameter("SizeMin1");
    m_featureRange1.m_oArea.end() = parameters_.getParameter("SizeMax1");
    m_featureRange1.m_oBoundingBoxDX.start() = parameters_.getParameter("BoundingBoxDXMin1");
    m_featureRange1.m_oBoundingBoxDX.end() = parameters_.getParameter("BoundingBoxDXMax1");
    m_featureRange1.m_oBoundingBoxDY.start() = parameters_.getParameter("BoundingBoxDYMin1");
    m_featureRange1.m_oBoundingBoxDY.end() = parameters_.getParameter("BoundingBoxDYMax1");
    m_featureRange1.m_oPcRatio.start() = parameters_.getParameter("PcRatioMin1");
    m_featureRange1.m_oPcRatio.end() = parameters_.getParameter("PcRatioMax1");
    m_featureRange1.m_oGradient.start() = parameters_.getParameter("GradientMin1");
    m_featureRange1.m_oGradient.end() = parameters_.getParameter("GradientMax1");
    m_featureRange1.m_oSurface.start() = parameters_.getParameter("SurfaceMin1");
    m_featureRange1.m_oSurface.end() = parameters_.getParameter("SurfaceMax1");
    m_paramScaling1 = parameters_.getParameter("ParamScaling1");

    m_featureRange2.m_oArea.start() = parameters_.getParameter("SizeMin2");
    m_featureRange2.m_oArea.end() = parameters_.getParameter("SizeMax2");
    m_featureRange2.m_oBoundingBoxDX.start() = parameters_.getParameter("BoundingBoxDXMin2");
    m_featureRange2.m_oBoundingBoxDX.end() = parameters_.getParameter("BoundingBoxDXMax2");
    m_featureRange2.m_oBoundingBoxDY.start() = parameters_.getParameter("BoundingBoxDYMin2");
    m_featureRange2.m_oBoundingBoxDY.end() = parameters_.getParameter("BoundingBoxDYMax2");
    m_featureRange2.m_oPcRatio.start() = parameters_.getParameter("PcRatioMin2");
    m_featureRange2.m_oPcRatio.end() = parameters_.getParameter("PcRatioMax2");
    m_featureRange2.m_oGradient.start() = parameters_.getParameter("GradientMin2");
    m_featureRange2.m_oGradient.end() = parameters_.getParameter("GradientMax2");
    m_featureRange2.m_oSurface.start() = parameters_.getParameter("SurfaceMin2");
    m_featureRange2.m_oSurface.end() = parameters_.getParameter("SurfaceMax2");
    m_paramScaling2 = parameters_.getParameter("ParamScaling2");
}



bool PoreClassifierOutputTriple::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup)
{
    if (p_rPipe.tag() == "bounding_box_dx")
    {
        m_pPipeInBoundingBoxDX = dynamic_cast<scalar_pipe_t*>(&p_rPipe);
    }
    else if (p_rPipe.tag() == "bounding_box_dy")
    {
        m_pPipeInBoundingBoxDY = dynamic_cast<scalar_pipe_t*>(&p_rPipe);
    }
    else if (p_rPipe.tag() == "pc_ratio")
    {
        m_pPipeInPcRatio = dynamic_cast<scalar_pipe_t*>(&p_rPipe);
    }
    else if (p_rPipe.tag() == "gradient")
    {
        m_pPipeInGradient = dynamic_cast<scalar_pipe_t*>(&p_rPipe);
    }
    else if (p_rPipe.tag() == "surface")
    {
        m_pPipeInSurface = dynamic_cast<scalar_pipe_t*>(&p_rPipe);
    }
    else if (p_rPipe.tag() == "contour")
    {
        m_pPipeInBlob = dynamic_cast<fliplib::SynchronePipe<interface::GeoBlobarray>*>(&p_rPipe);
    }

    return BaseFilter::subscribe(p_rPipe, p_oGroup);
}



void PoreClassifierOutputTriple::arm(const fliplib::ArmStateBase& p_rArmState)
{
    if (p_rArmState.getStateID() == eSeamStart)
    {
        m_poreId = 0; // additionally reset pore ids on seam start for simulation
    }
}


void PoreClassifierOutputTriple::proceedGroup(const void* p_pSender, fliplib::PipeGroupEventArgs& p_rEventArgs)
{
    // assert connected pipes
    poco_assert_dbg(m_pPipeInBoundingBoxDX != nullptr);
    poco_assert_dbg(m_pPipeInBoundingBoxDY != nullptr);
    poco_assert_dbg(m_pPipeInPcRatio != nullptr);
    poco_assert_dbg(m_pPipeInGradient != nullptr);
    poco_assert_dbg(m_pPipeInSurface != nullptr);
    poco_assert_dbg(m_pPipeInBlob != nullptr);

    m_oInputBoundingBoxDX = m_pPipeInBoundingBoxDX->read(m_oCounter);
    m_oInputBoundingBoxDY = m_pPipeInBoundingBoxDY->read(m_oCounter);
    m_oInputPcRatio = m_pPipeInPcRatio->read(m_oCounter);
    m_oInputGradient = m_pPipeInGradient->read(m_oCounter);
    m_oInputSurface = m_pPipeInSurface->read(m_oCounter);
    m_oInputBlob = m_pPipeInBlob->read(m_oCounter);

    poco_assert_dbg(m_oInputBoundingBoxDX.context() == m_oInputBoundingBoxDY.context());
    poco_assert_dbg(m_oInputBoundingBoxDX.context() == m_oInputPcRatio.context());
    poco_assert_dbg(m_oInputBoundingBoxDX.context() == m_oInputGradient.context());
    poco_assert_dbg(m_oInputBoundingBoxDX.context() == m_oInputSurface.context());
    poco_assert_dbg(m_oInputBoundingBoxDX.context() == m_oInputBlob.context());

    const interface::ImageContext& rContext = m_oInputBoundingBoxDX.context();
    const interface::ResultType oAnalysisResult = m_oInputBoundingBoxDX.analysisResult();

    m_oSpTrafo = rContext.trafo();

    // reset pore id if image has changed
    if (rContext.imageNumber() != m_imageNumber)
    {
        m_imageNumber = rContext.imageNumber();
        m_poreId = 0;
    }

    // calculate area of blob in mm^2 from bounding box (given in mm^2) and blob vector (given in pixel)
    m_oArea.clear();
    auto sizeX = std::begin(m_oInputBoundingBoxDX.ref().getData());
    auto sizeY = std::begin(m_oInputBoundingBoxDY.ref().getData());
    for (const auto& blob : m_oInputBlob.ref().getData())
    {
        const auto oNbPixPerBox = (blob.xmax - blob.xmin) * (blob.ymax - blob.ymin);
        m_oArea.push_back(*sizeX * *sizeY * blob.npix / (1. * oNbPixPerBox));

        sizeX++;
        sizeY++;
    }

    m_oClassByFeature.resize(m_activeParameterSets);
    m_oClassMerged.resize(m_activeParameterSets);

    // classify with unchanged feature range
    double poreSizeMax = -1;
    double poreSizeMin = -1;
    uint nrOfPoresNotBadRank = 0;

    if (m_activeParameterSets > 0)
    {
        nrOfPoresNotBadRank += classify(m_featureRange, poreSizeMax, poreSizeMin, 0);
        if (m_activeParameterSets > 1)
        {
            nrOfPoresNotBadRank += 100 * classify(m_featureRange1, poreSizeMax, poreSizeMin, 1);
            if (m_activeParameterSets > 2)
            {
                nrOfPoresNotBadRank += 10000 * classify(m_featureRange2, poreSizeMax, poreSizeMin, 2);
            }
        }
    }

    // Set number '1' as 'leading number' to indicate, how many sets were used
    if (m_activeParameterSets == 1)
    {
        nrOfPoresNotBadRank += 100;
    }
    else if (m_activeParameterSets == 2)
    {
        nrOfPoresNotBadRank += 10000;
    }
    else if (m_activeParameterSets == 3)
    {
        nrOfPoresNotBadRank += 1000000;
    }

    const geo2d::Doublearray oResultNbPores = geo2d::Doublearray{ 1, 1. * nrOfPoresNotBadRank, eRankMax };

    const ValueRankType oOutpipeRank = nrOfPoresNotBadRank != 0 ? eRankMax : eRankMin;
    const geo2d::Doublearray oResultPoreSizeMax = geo2d::Doublearray{ 1, poreSizeMax, oOutpipeRank };
    const geo2d::Doublearray oResultPoreSizeMin = geo2d::Doublearray{ 1, poreSizeMin, oOutpipeRank };

    // send result now. result vectors could be changed after due to scaled classification.
    // So, result does not include results from  scaled classification, which only served as an indication to the user.
    const interface::GeoDoublearray	oGeoValueOut(rContext, oResultNbPores, oAnalysisResult, interface::Limit);
    const interface::GeoDoublearray	oGeoValueOutSizeMax(rContext, oResultPoreSizeMax, oAnalysisResult, interface::Limit);
    const interface::GeoDoublearray	oGeoValueOutSizeMin(rContext, oResultPoreSizeMin, oAnalysisResult, interface::Limit);

    // skip if scaled classification is not desired (scale factor equals zero)
    classifyScaled();

    preSignalAction();
    m_oPipeOutPoreCount.signal(oGeoValueOut);
    m_oPipeOutPoreSizeMax.signal(oGeoValueOutSizeMax);
    m_oPipeOutPoreSizeMin.signal(oGeoValueOutSizeMin);
}

void PoreClassifierOutputTriple::dilateOrErodeByPercent(const FeatureRange& featureRange, int paramScaling, FeatureRange& featureRangeScaled)
{
    const double scaleFactorStart = 1. - paramScaling * 0.01;
    const double scaleFactorEnd = 1. + paramScaling * 0.01;

    auto range { [&](geo2d::Range1d r)->geo2d::Range1d { return geo2d::Range1d{ r.start() * scaleFactorStart, r.end() * scaleFactorEnd }; }};

    featureRangeScaled.m_oBoundingBoxDX = range(featureRange.m_oBoundingBoxDX);
    featureRangeScaled.m_oBoundingBoxDY = range(featureRange.m_oBoundingBoxDY);
    featureRangeScaled.m_oGradient      = range(featureRange.m_oGradient);
    featureRangeScaled.m_oPcRatio       = range(featureRange.m_oPcRatio);
    featureRangeScaled.m_oArea          = range(featureRange.m_oArea);
    featureRangeScaled.m_oSurface       = range(featureRange.m_oSurface);
}

void PoreClassifierOutputTriple::classifyScaled()
{
    if (!m_paramScaling && !m_paramScaling1 && !m_paramScaling2)
    {
        return;
    }

    m_featureRangeScaled.resize(m_activeParameterSets);

    // classify with scaled feature range
    if (m_activeParameterSets > 0)
    {
        dilateOrErodeByPercent(m_featureRange, m_paramScaling, m_featureRangeScaled[0]);
        if (m_activeParameterSets > 1)
        {
            dilateOrErodeByPercent(m_featureRange1, m_paramScaling1, m_featureRangeScaled[1]);
            if (m_activeParameterSets > 2)
            {
                dilateOrErodeByPercent(m_featureRange2, m_paramScaling2, m_featureRangeScaled[2]);
            }
        }
    }

    for (auto paramSet = 0; paramSet < m_activeParameterSets; paramSet++)
    {
        const pore_class_feature_array_t& classByFeatureNotScaled = m_oClassByFeature[paramSet];
        const std::vector<PoreClassType>& classMergedNotScaled = m_oClassMerged[paramSet];

        double poreSizeMax, poreSizeMin;
        classify(m_featureRangeScaled[paramSet], poreSizeMax, poreSizeMin, paramSet);

        // mark if classification result per feature has changed
        auto classNotScaled = std::begin(classByFeatureNotScaled);
        for (auto& rClass : m_oClassByFeature[paramSet])
        {
            auto resultNotScaled = std::begin(*classNotScaled);
            for (auto& result : rClass)
            {
                if (*resultNotScaled != result)
                {
                    result = result == ePore ? ePoreIfScaled : eNoPoreIfScaled;
                }

                resultNotScaled++;
            }

            classNotScaled++;
        }

        // mark if merged classification result has changed
        auto resultMergedNotScaled = std::begin(classMergedNotScaled);
        for (auto& resultMerged : m_oClassMerged[paramSet])
        {
            if (*resultMergedNotScaled != resultMerged)
            {
                resultMerged = resultMerged == ePore ? ePoreIfScaled : eNoPoreIfScaled;
            }

            resultMergedNotScaled++;
        }
    }
}

uint PoreClassifierOutputTriple::classify(const FeatureRange& featureRange, double& maxPoreSize, double& minPoreSize, int activeParamSet)
{
    const size_t numberOfCandidates = m_oArea.size(); // could be any other feature

    // get data
    const std::vector<double>& rDataBoundingBoxDX = m_oInputBoundingBoxDX.ref().getData();
    const std::vector<double>& rDataBoundingBoxDY = m_oInputBoundingBoxDY.ref().getData();
    const std::vector<double>& rDataPcRatio = m_oInputPcRatio.ref().getData();
    const std::vector<double>& rDataGradient = m_oInputGradient.ref().getData();
    const std::vector<double>& rDataSurface = m_oInputSurface.ref().getData();

    const std::vector<int>& rRankBoundingBoxDX = m_oInputBoundingBoxDX.ref().getRank();
    const std::vector<int>& rRankBoundingBoxDY = m_oInputBoundingBoxDY.ref().getRank();
    const std::vector<int>& rRankPcRatio = m_oInputPcRatio.ref().getRank();
    const std::vector<int>& rRankGradient = m_oInputGradient.ref().getRank();
    const std::vector<int>& rRankSurface = m_oInputSurface.ref().getRank();

    // assert equal number of features
    poco_assert_dbg(rDataBoundingBoxDX.size() == numberOfCandidates);
    poco_assert_dbg(rDataBoundingBoxDY.size() == numberOfCandidates);
    poco_assert_dbg(rDataPcRatio.size() == numberOfCandidates);
    poco_assert_dbg(rDataGradient.size() == numberOfCandidates);
    poco_assert_dbg(rDataSurface.size() == numberOfCandidates);

    // reset feature classification result, nr of candidates may have changed
    for (auto& classVec : m_oClassByFeature[activeParamSet])
    {
        classVec.assign(numberOfCandidates, ePore);
    }
    m_oClassMerged[activeParamSet].assign(numberOfCandidates, ePore); // reset classification result, nb candidates may have changed
    m_oClassMergedRank.assign(numberOfCandidates, eRankMax); // reset classification result rank, nb candidates may have changed. eRankMax!

    // calculate features, if a feature value lies in range, it is a pore
    for (size_t i = 0; i < numberOfCandidates; i++)
    {
        m_oClassByFeature[activeParamSet][eSize][i] = featureRange.m_oArea.contains(m_oArea[i]) ? ePore : eNoPore;
        m_oClassByFeature[activeParamSet][eBoundingBoxDX][i] = featureRange.m_oBoundingBoxDX.contains(rDataBoundingBoxDX[i]) ? ePore : eNoPore;
        m_oClassByFeature[activeParamSet][eBoundingBoxDY][i] = featureRange.m_oBoundingBoxDY.contains(rDataBoundingBoxDY[i]) ? ePore : eNoPore;
        m_oClassByFeature[activeParamSet][ePcRatio][i] = featureRange.m_oPcRatio.contains(rDataPcRatio[i]) ? ePore : eNoPore;
        m_oClassByFeature[activeParamSet][eGradient][i] = featureRange.m_oGradient.contains(rDataGradient[i]) ? ePore : eNoPore;
        m_oClassByFeature[activeParamSet][eSurface][i] = featureRange.m_oSurface.contains(rDataSurface[i]) ? ePore : eNoPore;
    }

    // calculate final classification by ANDing individual classification

    const auto oMergeClassification([](PoreClassType oFirst, PoreClassType oSecond)->PoreClassType { return (oFirst == ePore && oSecond == ePore) ? ePore : eNoPore; });

    std::transform(std::begin(m_oClassByFeature[activeParamSet][eSize]), std::end(m_oClassByFeature[activeParamSet][eSize]),
                   std::begin(m_oClassMerged[activeParamSet]), std::begin(m_oClassMerged[activeParamSet]), oMergeClassification);
    std::transform(std::begin(m_oClassByFeature[activeParamSet][eBoundingBoxDX]), std::end(m_oClassByFeature[activeParamSet][eBoundingBoxDX]),
                   std::begin(m_oClassMerged[activeParamSet]), std::begin(m_oClassMerged[activeParamSet]), oMergeClassification);
    std::transform(std::begin(m_oClassByFeature[activeParamSet][eBoundingBoxDY]), std::end(m_oClassByFeature[activeParamSet][eBoundingBoxDY]),
                   std::begin(m_oClassMerged[activeParamSet]), std::begin(m_oClassMerged[activeParamSet]), oMergeClassification);
    std::transform(std::begin(m_oClassByFeature[activeParamSet][ePcRatio]), std::end(m_oClassByFeature[activeParamSet][ePcRatio]),
                   std::begin(m_oClassMerged[activeParamSet]), std::begin(m_oClassMerged[activeParamSet]), oMergeClassification);
    std::transform(std::begin(m_oClassByFeature[activeParamSet][eGradient]), std::end(m_oClassByFeature[activeParamSet][eGradient]),
                   std::begin(m_oClassMerged[activeParamSet]), std::begin(m_oClassMerged[activeParamSet]), oMergeClassification);
    std::transform(std::begin(m_oClassByFeature[activeParamSet][eSurface]), std::end(m_oClassByFeature[activeParamSet][eSurface]),
                   std::begin(m_oClassMerged[activeParamSet]), std::begin(m_oClassMerged[activeParamSet]), oMergeClassification);

    // calculate rank by taking min rank of all features
    const auto oMin([](int a, int b)->int {return std::min(a,b); });

    // rank for size not included
    std::transform(std::begin(rRankBoundingBoxDX), std::end(rRankBoundingBoxDX), std::begin(m_oClassMergedRank), std::begin(m_oClassMergedRank), oMin);
    std::transform(std::begin(rRankBoundingBoxDY), std::end(rRankBoundingBoxDY), std::begin(m_oClassMergedRank), std::begin(m_oClassMergedRank), oMin);
    std::transform(std::begin(rRankPcRatio), std::end(rRankPcRatio), std::begin(m_oClassMergedRank), std::begin(m_oClassMergedRank), oMin);
    std::transform(std::begin(rRankGradient), std::end(rRankGradient), std::begin(m_oClassMergedRank), std::begin(m_oClassMergedRank), oMin);
    std::transform(std::begin(rRankSurface), std::end(rRankSurface), std::begin(m_oClassMergedRank), std::begin(m_oClassMergedRank), oMin);

    // signal number of pores as result - and nio if at leat one pore classified

    uint numberOfPoresNotBadRank = 0;

    auto oAreaIt = std::begin(m_oArea);
    auto oClassMergedRankIt = std::begin(m_oClassMergedRank);
    for (auto oClassMergedIt = std::begin(m_oClassMerged[activeParamSet]); oClassMergedIt != std::end(m_oClassMerged[activeParamSet]); oClassMergedIt++, oClassMergedRankIt++, oAreaIt++)
    {
        if (*oClassMergedIt == ePore && *oClassMergedRankIt != eRankMin)
        {
            numberOfPoresNotBadRank++;
            if (maxPoreSize < *oAreaIt)
            {
                maxPoreSize = *oAreaIt;
            }
            if (minPoreSize > *oAreaIt || minPoreSize == -1.0)
            {
                minPoreSize = *oAreaIt;
            }
        }
    }

    return numberOfPoresNotBadRank;
}



void PoreClassifierOutputTriple::paint()
{
    if (m_oVerbosity < eLow || m_oSpTrafo.isNull())
    {
        return;
    }

    if (m_oClassMerged[0].empty() || m_oInputBlob.ref().getData().empty()) // nothing there to paint
    {
        return;
    }

    const std::array<std::string, eNbFeatureTypes * 3> featureTypeString = { { "Size 1", "DX 1", "DY 1", "PcRatio 1", "Contrast 1", "Surface 1", "Size 2", "DX 2", "DY 2", "PcRatio 2", "Contrast 2", "Surface 2", "Size 3", "DX 3", "DY 3", "PcRatio 3", "Contrast 3", "Surface 3" } };

    const interface::Trafo &rTrafo(*m_oSpTrafo);
    image::OverlayCanvas &rCanvas(canvas<image::OverlayCanvas>(m_oCounter));
    image::OverlayLayer	&rLayerContour(rCanvas.getLayerContour());
    image::OverlayLayer	&rLayerText(rCanvas.getLayerText());
    image::OverlayLayer	&rLayerInfoBox(rCanvas.getLayerInfoBox());

    const std::array<std::vector<double>::const_iterator, eNbFeatureTypes> oFeatureCIts = { {
        m_oArea.cbegin(),
        m_oInputBoundingBoxDX.ref().getData().cbegin(),
        m_oInputBoundingBoxDY.ref().getData().cbegin(),
        m_oInputPcRatio.ref().getData().cbegin(),
        m_oInputGradient.ref().getData().cbegin(),
        m_oInputSurface.ref().getData().cbegin() } };

    static const std::array<const std::string, eNbFeatureTypes> unitsByFeature = { {
        interface::g_oLangKeyUnitSqMm,      // area in sq mm
        interface::g_oLangKeyUnitMm,        // width in mm
        interface::g_oLangKeyUnitMm,        // height in mm
        interface::g_oLangKeyUnitNone,      // no unit for ratio
        interface::g_oLangKeyUnitNone } };  // surface has no unit

    // prepare one info box per candidate

    // these colors are used to paint the blob contours and the feature strings depending on classification result, see oPoreClassType
    static const auto& oPoreClassTypeToColor = std::array<image::Color, ePoreClassTypeMax + 1>{{
        image::Color::m_oScarletRed, image::Color::m_oChameleonDark, image::Color::m_oSkyBlue, image::Color::m_oMagenta }};

    std::vector<std::vector<PoreClassType>::iterator> itClassMerged;
    itClassMerged.push_back(std::begin(m_oClassMerged[0]));
    if (m_oClassMerged.size() > 1 && !m_oClassMerged[1].empty())
    {
        itClassMerged.push_back(std::begin(m_oClassMerged[1]));
        if (m_oClassMerged.size () > 2 && !m_oClassMerged[2].empty())
        {
            itClassMerged.push_back(std::begin(m_oClassMerged[2]));
        }
    }
    auto oItArea = std::begin(m_oArea);

    poco_assert_dbg(m_oInputBlob.ref().size() == m_oClassMerged[0].size());

    int candidate = 0;

    for (const auto& blob : m_oInputBlob.ref().getData())
    {
        // get the bounding box
        const auto oBoundingBoxStart = geo2d::Point(blob.xmin, blob.ymin);
        const auto oBoundingBoxEnd = geo2d::Point(blob.xmax, blob.ymax);
        const auto oBoundingBox = geo2d::Rect(oBoundingBoxStart, oBoundingBoxEnd);

        auto oFeatureLines = std::vector<image::OverlayText>(eNbFeatureTypes*m_activeParameterSets+1);

        // Filter ID
        oFeatureLines[0] = image::OverlayText(id().toString() + ":FILTERGUID:0", image::Font(12, true, false, "Courier New"), rTrafo(geo2d::Rect(10, 10, 100, 200)), image::Color::Black(), 0);
        // Output for special "value info box" over the camera image
        auto oFeatureLinesIt = std::next(std::begin(oFeatureLines));
        for (auto i = 0; i < m_activeParameterSets; i++)
        {
            auto oUnitByFeatureIt = std::begin(unitsByFeature);
            auto oClassByFeatureIt = std::begin(m_oClassByFeature[i]);
            for (int j = 0; j < eNbFeatureTypes; j++) {
                const auto oIncFeature = std::distance(std::begin(oFeatureLines), oFeatureLinesIt);
                const auto oTextColor = oPoreClassTypeToColor[(*oClassByFeatureIt)[candidate]];
                std::ostringstream oMsg;

                oMsg << featureTypeString[oIncFeature-1] << ":" << *oUnitByFeatureIt << ":" << // colon is key-key-value delimiter
                    std::setprecision(2) << std::fixed << oFeatureCIts[j][candidate];
                *oFeatureLinesIt = image::OverlayText(oMsg.str(), image::Font(12), geo2d::Rect(), oTextColor, oIncFeature-1);

                oClassByFeatureIt++;
                oUnitByFeatureIt++;
                oFeatureLinesIt++;
            }
        }

        static const auto oXDistToPore = 6;
        // paint pore id close to pore. For GUI: ID + 1
        rLayerText.add<image::OverlayText>(std::to_string(m_poreId + 1), image::Font(16), rTrafo(geo2d::Rect(blob.xmax + oXDistToPore, blob.ymin - 1 * 15, 100, 20)), image::Color::Yellow()); // - 1*15 because zeroth

        // area overlay text
        if (m_oVerbosity > eLow)
        {
            std::ostringstream	oMsg;
            oMsg << "Size: " << std::setprecision(2) << std::fixed << *oItArea << " mm2";
            rLayerText.add<image::OverlayText>(oMsg.str(), image::Font(14), rTrafo(geo2d::Rect(blob.xmax, blob.ymin + 0 * 15, 200, 20)), image::Color::Yellow());  // +0*15 because 1st
        }

        // paint corresponding info box
        rLayerInfoBox.add<image::OverlayInfoBox>(image::ePore, m_poreId, std::move(oFeatureLines), rTrafo(oBoundingBox));

        // paint contour in green, red or blue
        int color = 1;
        for (std::size_t i = 0; i < itClassMerged.size(); i++)
        {
            if (*itClassMerged[i] == ePore)
            {
                color = 0;
                break;
            }
            if (*itClassMerged[i] == ePoreIfScaled)
            {
                color = 2;
            }
        }
        const auto contourColor = oPoreClassTypeToColor[color];
        for (auto oContourPosIt = std::begin(blob.m_oContour); oContourPosIt != std::end(blob.m_oContour); oContourPosIt++)
        {
            rLayerContour.add<image::OverlayPoint>(rTrafo(*oContourPosIt), contourColor);
        }

        for (std::size_t i = 0; i < itClassMerged.size(); i++)
        {
            itClassMerged[i]++;
        }
        oItArea++;
        m_poreId++; // counts for current image starting from zero - counts for all instances
        candidate++;
    }
}
}
}
