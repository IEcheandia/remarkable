/**
 * 	@file
 * 	@copyright  Precitec Vision GmbH & Co. KG
 * 	@author     MM
 * 	@date       2022
 * 	@brief      Filter that extracts blobs of a binary image and classifies them if they are pores.
 *              In Pipe: image, Out Pipes: number found pores, size of biggest pore for each parameter set
 */

#include "poreDetection.h"

#include "filter/algoImage.h"
#include "filter/algoStl.h"
#include "filter/morphologyImpl.h"
#include "segmentateImage.h"
#include "module/moduleLogger.h"
#include "overlay/overlayPrimitive.h"

#include <fliplib/TypeToDataTypeImpl.h>

namespace precitec
{
namespace filter
{
const std::string PoreDetection::m_filterName = std::string("PoreDetection");

PoreDetection::PoreDetection()
: fliplib::TransformFilter(m_filterName, Poco::UUID("5BC6F37B-2F0C-4D7A-A2D7-1B9599C45FBF"))
, m_pipeInImageFrame (nullptr)
, m_pipeOutPoreCount (this, "poreCount")
, m_pipeOutPoreCount1 (this, "poreCount1")
, m_pipeOutPoreCount2 (this, "poreCount2")
, m_pipeOutSizeMax (this, "sizeMax")
, m_activeParameterSets (1)
, m_visualize {Visualize::none, Visualize::none, Visualize::none}
, m_distToMeanIntensity {128, 128, 128}
, m_binarizeType {BinarizeType::eGlobal, BinarizeType::eGlobal, BinarizeType::eGlobal}
, m_numberIterationsMorphology {2, 2, 2}
, m_maxNumberBlobs {100, 100, 100}
, m_minBlobSize {20, 20, 20}
, m_boundingBoxScale {0.5, 0.5, 0.5}
, m_numberNeighbors {1, 1, 1}
, m_outerNeighborDistance {3, 3, 3}
, m_featureRange {FeatureRange(), FeatureRange(), FeatureRange()}
, m_parameterScaling {0, 0, 0}
{
    for (uint i = 0; i < m_maxParameterSets; i++)
    {
        m_dataBlobDetection[i].alloc(m_maxNumberBlobs[i]);
    }

    parameters_.add("NumberParameterSets",         fliplib::Parameter::TYPE_uint,   m_activeParameterSets);
    parameters_.add("Visualize",                   fliplib::Parameter::TYPE_int,    static_cast<int>(m_visualize[0]));
    parameters_.add("Visualize1",                  fliplib::Parameter::TYPE_int,    static_cast<int>(m_visualize[1]));
    parameters_.add("Visualize2",                  fliplib::Parameter::TYPE_int,    static_cast<int>(m_visualize[2]));
    // binarize
    parameters_.add("DistToMeanIntensity",         fliplib::Parameter::TYPE_uint,   static_cast<uint>(m_distToMeanIntensity[0]));
    parameters_.add("DistToMeanIntensity1",        fliplib::Parameter::TYPE_uint,   static_cast<uint>(m_distToMeanIntensity[1]));
    parameters_.add("DistToMeanIntensity2",        fliplib::Parameter::TYPE_uint,   static_cast<uint>(m_distToMeanIntensity[2]));
    parameters_.add("BinarizeType",                fliplib::Parameter::TYPE_int,    static_cast<int>(m_binarizeType[0]));
    parameters_.add("BinarizeType1",               fliplib::Parameter::TYPE_int,    static_cast<int>(m_binarizeType[1]));
    parameters_.add("BinarizeType2",               fliplib::Parameter::TYPE_int,    static_cast<int>(m_binarizeType[2]));
    // morphology
    parameters_.add("NumberIterationsMorphology",  fliplib::Parameter::TYPE_uint,   m_numberIterationsMorphology[0]);
    parameters_.add("NumberIterationsMorphology1", fliplib::Parameter::TYPE_uint,   m_numberIterationsMorphology[1]);
    parameters_.add("NumberIterationsMorphology2", fliplib::Parameter::TYPE_uint,   m_numberIterationsMorphology[2]);
    // blob detection
    parameters_.add("MaxNumberBlobs",              fliplib::Parameter::TYPE_uint,   m_maxNumberBlobs[0]);
    parameters_.add("MaxNumberBlobs1",             fliplib::Parameter::TYPE_uint,   m_maxNumberBlobs[1]);
    parameters_.add("MaxNumberBlobs2",             fliplib::Parameter::TYPE_uint,   m_maxNumberBlobs[2]);
    parameters_.add("MinBlobSize",                 fliplib::Parameter::TYPE_uint,   m_minBlobSize[0]);
    parameters_.add("MinBlobSize1",                fliplib::Parameter::TYPE_uint,   m_minBlobSize[1]);
    parameters_.add("MinBlobSize2",                fliplib::Parameter::TYPE_uint,   m_minBlobSize[2]);
    // variance (surface)
    parameters_.add("BoundingBoxScale",            fliplib::Parameter::TYPE_double, m_boundingBoxScale[0]);
    parameters_.add("BoundingBoxScale1",           fliplib::Parameter::TYPE_double, m_boundingBoxScale[1]);
    parameters_.add("BoundingBoxScale2",           fliplib::Parameter::TYPE_double, m_boundingBoxScale[2]);
    // pore gradient
    parameters_.add("NumberNeighbors",             fliplib::Parameter::TYPE_uint, m_numberNeighbors[0]);
    parameters_.add("NumberNeighbors1",            fliplib::Parameter::TYPE_uint, m_numberNeighbors[1]);
    parameters_.add("NumberNeighbors2",            fliplib::Parameter::TYPE_uint, m_numberNeighbors[2]);
    parameters_.add("OuterNeighborDistance",       fliplib::Parameter::TYPE_uint, m_outerNeighborDistance[0]);
    parameters_.add("OuterNeighborDistance1",      fliplib::Parameter::TYPE_uint, m_outerNeighborDistance[1]);
    parameters_.add("OuterNeighborDistance2",      fliplib::Parameter::TYPE_uint, m_outerNeighborDistance[2]);
    // blob classifier
    parameters_.add("MinimalWidth",                fliplib::Parameter::TYPE_double, m_featureRange[0].boundingBoxDX.start());
    parameters_.add("MinimalWidth1",               fliplib::Parameter::TYPE_double, m_featureRange[1].boundingBoxDX.start());
    parameters_.add("MinimalWidth2",               fliplib::Parameter::TYPE_double, m_featureRange[2].boundingBoxDX.start());
    parameters_.add("MaximalWidth",                fliplib::Parameter::TYPE_double, m_featureRange[0].boundingBoxDX.end());
    parameters_.add("MaximalWidth1",               fliplib::Parameter::TYPE_double, m_featureRange[1].boundingBoxDX.end());
    parameters_.add("MaximalWidth2",               fliplib::Parameter::TYPE_double, m_featureRange[2].boundingBoxDX.end());
    parameters_.add("MinimalHeight",               fliplib::Parameter::TYPE_double, m_featureRange[0].boundingBoxDY.start());
    parameters_.add("MinimalHeight1",              fliplib::Parameter::TYPE_double, m_featureRange[1].boundingBoxDY.start());
    parameters_.add("MinimalHeight2",              fliplib::Parameter::TYPE_double, m_featureRange[2].boundingBoxDY.start());
    parameters_.add("MaximalHeight",               fliplib::Parameter::TYPE_double, m_featureRange[0].boundingBoxDY.end());
    parameters_.add("MaximalHeight1",              fliplib::Parameter::TYPE_double, m_featureRange[1].boundingBoxDY.end());
    parameters_.add("MaximalHeight2",              fliplib::Parameter::TYPE_double, m_featureRange[2].boundingBoxDY.end());
    parameters_.add("MinimalPcRatio",              fliplib::Parameter::TYPE_double, m_featureRange[0].pcRatio.start());
    parameters_.add("MinimalPcRatio1",             fliplib::Parameter::TYPE_double, m_featureRange[1].pcRatio.start());
    parameters_.add("MinimalPcRatio2",             fliplib::Parameter::TYPE_double, m_featureRange[2].pcRatio.start());
    parameters_.add("MaximalPcRatio",              fliplib::Parameter::TYPE_double, m_featureRange[0].pcRatio.end());
    parameters_.add("MaximalPcRatio1",             fliplib::Parameter::TYPE_double, m_featureRange[1].pcRatio.end());
    parameters_.add("MaximalPcRatio2",             fliplib::Parameter::TYPE_double, m_featureRange[2].pcRatio.end());
    parameters_.add("MinimalContrast",             fliplib::Parameter::TYPE_uint,   m_featureRange[0].gradient.start());
    parameters_.add("MinimalContrast1",            fliplib::Parameter::TYPE_uint,   m_featureRange[1].gradient.start());
    parameters_.add("MinimalContrast2",            fliplib::Parameter::TYPE_uint,   m_featureRange[2].gradient.start());
    parameters_.add("MaximalContrast",             fliplib::Parameter::TYPE_uint,   m_featureRange[0].gradient.end());
    parameters_.add("MaximalContrast1",            fliplib::Parameter::TYPE_uint,   m_featureRange[1].gradient.end());
    parameters_.add("MaximalContrast2",            fliplib::Parameter::TYPE_uint,   m_featureRange[2].gradient.end());
    parameters_.add("MinimalSurface",              fliplib::Parameter::TYPE_uint,   m_featureRange[0].surface.start());
    parameters_.add("MinimalSurface1",             fliplib::Parameter::TYPE_uint,   m_featureRange[1].surface.start());
    parameters_.add("MinimalSurface2",             fliplib::Parameter::TYPE_uint,   m_featureRange[2].surface.start());
    parameters_.add("MaximalSurface",              fliplib::Parameter::TYPE_uint,   m_featureRange[0].surface.end());
    parameters_.add("MaximalSurface1",             fliplib::Parameter::TYPE_uint,   m_featureRange[1].surface.end());
    parameters_.add("MaximalSurface2",             fliplib::Parameter::TYPE_uint,   m_featureRange[2].surface.end());
    parameters_.add("ParameterScaling",            fliplib::Parameter::TYPE_int,   m_parameterScaling[0]);
    parameters_.add("ParameterScaling1",           fliplib::Parameter::TYPE_int,   m_parameterScaling[1]);
    parameters_.add("ParameterScaling2",           fliplib::Parameter::TYPE_int,   m_parameterScaling[2]);

    setInPipeConnectors({{Poco::UUID("889C22AE-FA0C-459B-AF27-9A9E3DA1C81A"), m_pipeInImageFrame, "ImageFrame", 0, ""}});
    setOutPipeConnectors({{Poco::UUID("B1D1340D-277F-438D-B3E1-6F1E42C495B7"), &m_pipeOutPoreCount, "poreCount", 0, ""},
                         {Poco::UUID("86E6F348-E493-4F09-84AD-DBCA2376147F"), &m_pipeOutPoreCount1, "poreCount1", 0, ""},
                         {Poco::UUID("8D4C124C-8C63-49BB-9A31-56815FE3B234"), &m_pipeOutPoreCount2, "poreCount2", 0, ""},
                          {Poco::UUID("BD322A9E-2249-42DB-9CE6-C10D40DCDA93"), &m_pipeOutSizeMax,   "sizeMax",   0, ""}});
    setVariantID(Poco::UUID("E5B77AEA-34F2-421C-A08F-1264E9E0FFD8"));
};

PoreDetection::~PoreDetection()
{
    for (uint i = 0; i < m_maxParameterSets; i++)
    {
        m_dataBlobDetection[i].free();
    }
}

void PoreDetection::arm(const fliplib::ArmStateBase& p_rArmState)
{
}

bool PoreDetection::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup)
{
    m_pipeInImageFrame = dynamic_cast<fliplib::SynchronePipe<interface::ImageFrame>*>(&p_rPipe);

    return BaseFilter::subscribe(p_rPipe, p_oGroup);
}

void PoreDetection::setParameter()
{
    TransformFilter::setParameter();

    m_activeParameterSets = parameters_.getParameter("NumberParameterSets");
    m_visualize = {static_cast<Visualize>(parameters_.getParameter("Visualize").convert<int>()), static_cast<Visualize>(parameters_.getParameter("Visualize1").convert<int>()),
        static_cast<Visualize>(parameters_.getParameter("Visualize2").convert<int>())};
    // binarize
    m_distToMeanIntensity = {parameters_.getParameter("DistToMeanIntensity"), parameters_.getParameter("DistToMeanIntensity1"), parameters_.getParameter("DistToMeanIntensity2")};
    m_binarizeType = {static_cast<BinarizeType>(parameters_.getParameter("BinarizeType").convert<int>()), static_cast<BinarizeType>(parameters_.getParameter("BinarizeType1").convert<int>()),
        static_cast<BinarizeType>(parameters_.getParameter("BinarizeType2").convert<int>())};
    // morphology
    m_numberIterationsMorphology = {parameters_.getParameter("NumberIterationsMorphology"), parameters_.getParameter("NumberIterationsMorphology1"),
        parameters_.getParameter("NumberIterationsMorphology2")};
    // blob detection
    m_maxNumberBlobs = {parameters_.getParameter("MaxNumberBlobs"), parameters_.getParameter("MaxNumberBlobs1"), parameters_.getParameter("MaxNumberBlobs2")};
    m_minBlobSize = {parameters_.getParameter("MinBlobSize"), parameters_.getParameter("MinBlobSize1"), parameters_.getParameter("MinBlobSize2")};
    // variance (surface)
    m_boundingBoxScale = {parameters_.getParameter("BoundingBoxScale"), parameters_.getParameter("BoundingBoxScale1"), parameters_.getParameter("BoundingBoxScale2")};
    // pore gradient
    m_numberNeighbors = {parameters_.getParameter("NumberNeighbors"), parameters_.getParameter("NumberNeighbors1"), parameters_.getParameter("NumberNeighbors2")};
    m_outerNeighborDistance = {parameters_.getParameter("OuterNeighborDistance"), parameters_.getParameter("OuterNeighborDistance1"), parameters_.getParameter("OuterNeighborDistance2")};
    // blob classifier
    m_featureRange[0].boundingBoxDX.start() = parameters_.getParameter("MinimalWidth");
    m_featureRange[0].boundingBoxDX.end() = parameters_.getParameter("MaximalWidth");
    m_featureRange[0].boundingBoxDY.start() = parameters_.getParameter("MinimalHeight");
    m_featureRange[0].boundingBoxDY.end() = parameters_.getParameter("MaximalHeight");
    m_featureRange[0].pcRatio.start() = parameters_.getParameter("MinimalPcRatio");
    m_featureRange[0].pcRatio.end() = parameters_.getParameter("MaximalPcRatio");
    m_featureRange[0].gradient.start() = parameters_.getParameter("MinimalContrast");
    m_featureRange[0].gradient.end() = parameters_.getParameter("MaximalContrast");
    m_featureRange[0].surface.start() = parameters_.getParameter("MinimalSurface");
    m_featureRange[0].surface.end() = parameters_.getParameter("MaximalSurface");
    m_featureRange[1].boundingBoxDX.start() = parameters_.getParameter("MinimalWidth1");
    m_featureRange[1].boundingBoxDX.end() = parameters_.getParameter("MaximalWidth1");
    m_featureRange[1].boundingBoxDY.start() = parameters_.getParameter("MinimalHeight1");
    m_featureRange[1].boundingBoxDY.end() = parameters_.getParameter("MaximalHeight1");
    m_featureRange[1].pcRatio.start() = parameters_.getParameter("MinimalPcRatio1");
    m_featureRange[1].pcRatio.end() = parameters_.getParameter("MaximalPcRatio1");
    m_featureRange[1].gradient.start() = parameters_.getParameter("MinimalContrast1");
    m_featureRange[1].gradient.end() = parameters_.getParameter("MaximalContrast1");
    m_featureRange[1].surface.start() = parameters_.getParameter("MinimalSurface1");
    m_featureRange[1].surface.end() = parameters_.getParameter("MaximalSurface1");
    m_featureRange[2].boundingBoxDX.start() = parameters_.getParameter("MinimalWidth2");
    m_featureRange[2].boundingBoxDX.end() = parameters_.getParameter("MaximalWidth2");
    m_featureRange[2].boundingBoxDY.start() = parameters_.getParameter("MinimalHeight2");
    m_featureRange[2].boundingBoxDY.end() = parameters_.getParameter("MaximalHeight2");
    m_featureRange[2].pcRatio.start() = parameters_.getParameter("MinimalPcRatio2");
    m_featureRange[2].pcRatio.end() = parameters_.getParameter("MaximalPcRatio2");
    m_featureRange[2].gradient.start() = parameters_.getParameter("MinimalContrast2");
    m_featureRange[2].gradient.end() = parameters_.getParameter("MaximalContrast2");
    m_featureRange[2].surface.start() = parameters_.getParameter("MinimalSurface2");
    m_featureRange[2].surface.end() = parameters_.getParameter("MaximalSurface2");
    m_parameterScaling = {parameters_.getParameter("ParameterScaling"), parameters_.getParameter("ParameterScaling1"), parameters_.getParameter("ParameterScaling2")};
}

void PoreDetection::proceed(const void* sender, fliplib::PipeEventArgs& e)
{
    const interface::ImageFrame& frame(m_pipeInImageFrame->read(m_oCounter));
    const image::BImage& imageIn = frame.data();
    const interface::ImageContext& context = frame.context();
    const interface::ResultType& analysisResult = frame.analysisResult();
    m_smpTrafo = context.trafo();

    if (!imageIn.isValid())
    {
        wmLog(eDebug, "Filter %s: Invalid input image.", m_filterName);
        interface::GeoDoublearray geoDoublearraySize (frame.context(), geo2d::Doublearray(), frame.analysisResult(), interface::NotPresent);
        interface::GeoDoublearray geoDoublearrayCount (frame.context(), geo2d::Doublearray(), frame.analysisResult(), interface::NotPresent);
        preSignalAction();
        m_pipeOutPoreCount.signal(geoDoublearrayCount);
        m_pipeOutPoreCount1.signal(geoDoublearrayCount);
        m_pipeOutPoreCount2.signal(geoDoublearrayCount);
        m_pipeOutSizeMax.signal(geoDoublearraySize);

        return;
    }

    double maxPoreSize = 0;
    std::array<double, m_maxParameterSets> numberPoresNotBadRank = {-1, -1, -1};
    ValueRankType outpipeRank = eRankMin;
    auto &rCalib(system::CalibDataSingleton::getCalibrationCoords(math::SensorId::eSensorId0));

    for (uint i = 0; i < m_activeParameterSets; ++i)
    {
        m_binaryImage[i].resizeFill(imageIn.size(), 0);
        m_morphologyImage[i].clear();
        m_morphologyImage[i].resize(imageIn.size());

        geo2d::Blobarray& blobArray = m_blobs[i];

        // Binarize
        binarize(imageIn, i);

        // Morphology
        morphology(i);

        // BlobDetection
        SSF_SF_InputStruct sgmima;
        sgmima.img = m_morphologyImage[i].begin();
        sgmima.npixx = m_morphologyImage[i].width();
        sgmima.npixy = m_morphologyImage[i].height();
        sgmima.pitch = m_morphologyImage[i].stride();
        sgmima.roistart = sgmima.img;
        sgmima.roix0 = 0;
        sgmima.roiy0 = 0;
        sgmima.roidx = sgmima.npixx;
        sgmima.roidy = sgmima.npixy;
        blobArray.clear();
        segmentateimage(sgmima, m_dataBlobDetection[i], m_minBlobSize[i]);

        std::vector<geo2d::Blob>& rBlobVector = blobArray.getData();
        const auto& dataBlobDetection = m_dataBlobDetection[i];

        blobArray.assign(dataBlobDetection.nspots, geo2d::Blob(), eRankMax);
        for(int j = 0; j < dataBlobDetection.nspots; ++j)
        {
            rBlobVector[j] = dataBlobDetection.outspot[j];
        }

        // Compute Contour (could possibly be done in segmentateImage)
        geo2d::Doublearray valX, valY; ///< just for compatible reasons to use calcContour()
        std::vector<geo2d::AnnotatedDPointarray> contours; ///< just for compatible reasons to use calcContour()
        calcContour(m_morphologyImage[i], blobArray, valX, valY, contours, m_filterName);

        // Surface
        m_surface[i].clear();
        m_surface[i] = variance(imageIn, blobArray, m_boundingBoxScale[i]);

        // Pore gradient
        m_poreGradient[i].clear();
        calcPoreGradient(imageIn, blobArray, m_poreGradient[i], m_numberNeighbors[i], m_outerNeighborDistance[i]);

        // Main Axis
        m_pcRatios[i].clear();
        m_pcRatios[i] = calcPrincipalComponents(m_morphologyImage[i], blobArray);

        // Bounding Box
        geo2d::Doublearray& rBoundingBoxDX = m_boundingBoxDX[i];
        geo2d::Doublearray& rBoundingBoxDY = m_boundingBoxDY[i];
        rBoundingBoxDX.clear();
        rBoundingBoxDY.clear();
        rBoundingBoxDX.assign(blobArray.size(), 0, eRankMax);
        rBoundingBoxDY.assign(blobArray.size(), 0, eRankMax);

        auto boundingBoxDXIt = rBoundingBoxDX.getData().begin();
        auto boundingBoxDYIt = rBoundingBoxDY.getData().begin();
        for (geo2d::Blob& blob : rBlobVector )
        {
            *boundingBoxDXIt = rCalib.distanceOnHorizontalPlane(blob.xmin, blob.ymin, blob.xmax, blob.ymin) / frame.context().SamplingX_;
            *boundingBoxDYIt = rCalib.distanceOnHorizontalPlane(blob.xmin, blob.ymin, blob.xmin, blob.ymax) / frame.context().SamplingY_;

            ++boundingBoxDXIt;
            ++boundingBoxDYIt;
        }

        // Classifier to check if blob is pore
        std::vector<double> area; ///< size of blob in mm2
        boundingBoxDXIt = rBoundingBoxDX.getData().begin();
        boundingBoxDYIt = rBoundingBoxDY.getData().begin();
        for (const geo2d::Blob& blob : blobArray.getData())
        { // calculate area of blob in mm^2 from bounding box (given in mm^2) and blob vector (given in pixel)
            const int numberPixelPerBox = (blob.xmax - blob.xmin) * (blob.ymax - blob.ymin);
            area.push_back(*boundingBoxDXIt * *boundingBoxDYIt * blob.npix / (1. * numberPixelPerBox));

            boundingBoxDXIt++;
            boundingBoxDYIt++;
        }
        double maxPoreSizeTemp = -1;
        uint numberPoresNotBadRankTemp = 0;
        std::tie(numberPoresNotBadRankTemp, maxPoreSizeTemp) = classify(area, i);
        numberPoresNotBadRank[i] = numberPoresNotBadRankTemp;
        if (maxPoreSize < maxPoreSizeTemp)
        {
            maxPoreSize = maxPoreSizeTemp;
            if (numberPoresNotBadRankTemp != 0)
            {
                outpipeRank = eRankMax;
            }
        }

        if (m_parameterScaling[i] != 0)
        {
            classifyScaled(area, m_parameterScaling[i], i);
        }
    }

    const geo2d::Doublearray resultPoreSizeMax = geo2d::Doublearray{1, maxPoreSize, outpipeRank};
    const geo2d::Doublearray resultNumberPores = geo2d::Doublearray{1, numberPoresNotBadRank[0], numberPoresNotBadRank[0] < 0 ? eRankMin : eRankMax};
    const geo2d::Doublearray resultNumberPores1 = geo2d::Doublearray{1, numberPoresNotBadRank[1], numberPoresNotBadRank[1] < 0 ? eRankMin : eRankMax};
    const geo2d::Doublearray resultNumberPores2 = geo2d::Doublearray{1, numberPoresNotBadRank[2], numberPoresNotBadRank[2] < 0 ? eRankMin : eRankMax};
    const interface::GeoDoublearray geoValueOutNumberPores(context, resultNumberPores, analysisResult, interface::Limit);
    const interface::GeoDoublearray geoValueOutNumberPores1(context, resultNumberPores1, analysisResult, interface::Limit);
    const interface::GeoDoublearray geoValueOutNumberPores2(context, resultNumberPores2, analysisResult, interface::Limit);
    const interface::GeoDoublearray geoValueOutSizeMax(context, resultPoreSizeMax, analysisResult, interface::Limit);
    preSignalAction();
    m_pipeOutPoreCount.signal(geoValueOutNumberPores);
    m_pipeOutPoreCount1.signal(geoValueOutNumberPores1);
    m_pipeOutPoreCount2.signal(geoValueOutNumberPores2);

    m_pipeOutSizeMax.signal(geoValueOutSizeMax);
}

void PoreDetection::binarize(const image::BImage& image, uint parameterSet)
{
    image::BImage& binaryImage = m_binaryImage[parameterSet];
    if (m_distToMeanIntensity[parameterSet] == 0)
    {
        return;
    }
    switch (m_binarizeType[parameterSet])
    {
        case BinarizeType::eGlobal: // global and dynamically
            calcBinarizeDynamic(image, ComparisonType::eLess, m_distToMeanIntensity[parameterSet], binaryImage);
            break;
        case BinarizeType::eLocal: // local and dynamically
            calcBinarizeLocal(image, ComparisonType::eLess, m_distToMeanIntensity[parameterSet], binaryImage);
            break;
        default:
            calcBinarizeDynamic(image, ComparisonType::eLess, m_distToMeanIntensity[parameterSet], binaryImage);
            break;
    }
}

void PoreDetection::morphology(uint parameterSet)
{
    const image::BImage& binaryImage = m_binaryImage[parameterSet];
    image::BImage& morphologyImage = m_morphologyImage[parameterSet];

    if (binaryImage.size().width >= 32 && binaryImage.size().height >= 32)
    { // packed computation not possible on small images (at least one int must be filled)
        const geo2d::Size oSizePacked(int(std::ceil(double(binaryImage.size().width) / 32)), binaryImage.size().height);
        image::U32Image oImgInPacked(oSizePacked);
        image::U32Image oImgOutPacked(oSizePacked);

        bin2ToBin32(binaryImage, oImgInPacked); // pack image

        image::U32Image oImgTmp(oImgInPacked.size());

        opening32(oImgInPacked, oImgTmp, m_numberIterationsMorphology[parameterSet]);
        closing32(oImgTmp, oImgOutPacked, m_numberIterationsMorphology[parameterSet]);

        bin32ToBin2(oImgOutPacked, morphologyImage); // unpack image
    }
    else
    {
        image::BImage oImgTmp(binaryImage.size());
        opening(binaryImage, oImgTmp, m_numberIterationsMorphology[parameterSet]);
        closing(oImgTmp, morphologyImage, m_numberIterationsMorphology[parameterSet]);
    }
}

geo2d::Doublearray PoreDetection::variance(const image::BImage& image, const geo2d::Blobarray& blobs, double boundingBoxScale)
{
    geo2d::Doublearray variance;
    variance.assign(blobs.size(), 0, eRankMin);
    std::vector<geo2d::Rect> scaledRects;
    scaledRects.assign(blobs.size(), geo2d::Rect());

    const geo2d::Range validImageRangeX (0, image.size().width);
    const geo2d::Range validImageRangeY (0, image.size().height);
    const auto&        blobVector       (blobs.getData());

    auto blobRankIt    (std::begin(blobs.getRank()));
    auto varianceIt    (std::begin(variance.getData()));
    auto varianceRankIt(std::begin(variance.getRank()));
    auto scaledRectIt  (std::begin(scaledRects));

    auto incrementIterators = [](auto& blobRank, auto& variance, auto& varianceRank, auto& scaledRect){
            ++blobRank;
            ++variance;
            ++varianceRank;
            ++scaledRect;
        };

    for (auto& blob : blobVector)
    {
        if (*blobRankIt == eRankMin)
        {
            incrementIterators(blobRankIt, varianceIt, varianceRankIt, scaledRectIt);
            continue;
        }

        const geo2d::Point  boundingBoxStart   (blob.xmin, blob.ymin);
        const geo2d::Point  boundingBoxEnd     (blob.xmax, blob.ymax);
        const geo2d::Rect   boundingBox        (boundingBoxStart, boundingBoxEnd);

        const uint oldWith = blob.xmax - blob.xmin + 1;
        const uint oldHeight = blob.ymax - blob.ymin + 1;
        const interface::LinearTrafo offset (int((1. - boundingBoxScale) * oldWith * 0.5), int((1. - boundingBoxScale) * oldHeight * 0.5));
        const uint newWidth (roundToT<uint>(oldWith * boundingBoxScale));
        const uint newHeight (roundToT<uint>(oldHeight * boundingBoxScale));
        const uint newWidthClipped = newWidth > 0 ? newWidth : 1;
        const uint newHeightClipped = newHeight > 0 ? newHeight : 1;
        const geo2d::Rect scaled (boundingBoxStart.x, boundingBoxStart.y, newWidthClipped, newHeightClipped);
        const geo2d::Rect scaledAndOffset (offset(scaled)); // move scaled bounding box to center of unscaled bounding box
        const geo2d::Rect scaledOffCorr (intersect(scaledAndOffset, geo2d::Rect(validImageRangeX, validImageRangeY))); // cut off the part that is not in the image

        // calculate texture feature like sample mean (de: stichprobenvarianz)
        const int area = scaledAndOffset.sizeWithBorder().area();
        if (area <= 1)
        { // 'area - 1' will be divisor
            if (area == 1)
            {
                *varianceIt     = 0;
                *varianceRankIt = interface::Limit; // handle degenerated case for very small images. Prevent zero rank to obtain a result still valid in PoreClassificator.
            }

            incrementIterators(blobRankIt, varianceIt, varianceRankIt, scaledRectIt);
            continue;
        }
        const image::BImage oPatch (image, scaledOffCorr, true);
        *scaledRectIt   = scaledOffCorr;
        *varianceIt     = calcVariance(oPatch);
        *varianceRankIt = eRankMax;

        incrementIterators(blobRankIt, varianceIt, varianceRankIt, scaledRectIt);
    }
    return variance;
}

void PoreDetection::calcPoreGradient(const image::BImage& image, const geo2d::Blobarray& blobs, geo2d::Doublearray& poreGradientOut, uint numberNeighbors, uint outerNeighborDistance)
{
    const uint numberBlobs (blobs.size());

    poreGradientOut.assign(numberBlobs, 0, eRankMin);
    if (numberBlobs == 0)
    {
        return;
    }

    const geo2d::Range              validImageRangeX (0, image.size().width - 1);
    const geo2d::Range              validImageRangeY (0, image.size().height - 1);
    const std::vector<geo2d::Blob>& blobVector       (blobs.getData());

    auto blobRankIt         = std::begin(blobs.getRank());
    auto poreGradientIt     = std::begin(poreGradientOut.getData());
    auto poreGradientRankIt = std::begin(poreGradientOut.getRank());
    for(auto blobIt = std::begin(blobVector); blobIt != std::end(blobVector); ++blobIt, ++poreGradientIt, ++poreGradientRankIt)
    {
        if (*blobRankIt == eRankMin || blobIt->m_oContour.empty() == true)
        {
            continue;
        }

        geo2d::Point contourPositionIn;
        geo2d::Point contourPositionOut;
        Dir          direction              = Dir::N;
        Dir          direction90In          = Dir::N;
        Dir          direction90Out         = Dir::N;
        uint         innerIntensitySum      = 0;
        uint         outerIntensitySum      = 0;
        uint         numberInnerPosOnBorder = 0;
        uint         numberOuterPosOnBorder = 0;
        const auto   contourEndIt           = blobIt->m_oContour.cend();

        for (auto contourIt = blobIt->m_oContour.begin(); contourIt != contourEndIt - 1; ++contourIt)
        { // not empty assertion above
            direction = getDir(*contourIt, *(contourIt + 1));
            get90GradDirection(direction, direction90In, direction90Out);
            contourPositionIn = *contourIt;
            contourPositionOut = *contourIt;

            // inner neighbors
            geo2d::Point contourPositionInNext;
            for (uint i = 0; i < numberNeighbors; i++)
            {
                contourPositionInNext = getNeighborFrom(contourPositionIn, direction90In);
                if (validImageRangeX.contains(contourPositionInNext.x) && validImageRangeY.contains(contourPositionInNext.y))
                {
                    contourPositionIn = contourPositionInNext;
                    innerIntensitySum += image[contourPositionIn.y][contourPositionIn.x];
                }
                else
                {
                    ++numberInnerPosOnBorder;
                }
            }

            // outer neighbors
            geo2d::Point contourPositionOutNext;
            for (uint i = 0; i < (outerNeighborDistance + numberNeighbors); i++)
            {
                contourPositionOutNext = getNeighborFrom(contourPositionOut, direction90Out);
                if (validImageRangeX.contains(contourPositionOutNext.x) && validImageRangeY.contains(contourPositionOutNext.y))
                {
                    contourPositionOut = contourPositionOutNext;
                    if (i >= outerNeighborDistance)
                    {
                        outerIntensitySum += image[contourPositionOut.y][contourPositionOut.x];
                    }
                }
                else if (i >= outerNeighborDistance)
                {
                    ++numberOuterPosOnBorder;
                }
            }
        }

        const uint numberContourPoints = blobIt->m_oContour.size();
        const uint numberValidInnerIntensities = numberContourPoints * numberNeighbors - numberInnerPosOnBorder;
        innerIntensitySum = numberValidInnerIntensities != 0 ? innerIntensitySum / numberValidInnerIntensities : 0;

        const uint numberValidOuterIntensities = numberContourPoints * numberNeighbors - numberOuterPosOnBorder;
        outerIntensitySum = numberValidOuterIntensities != 0 ? outerIntensitySum / numberValidOuterIntensities : 0;

        const int diffIntensity = outerIntensitySum - innerIntensitySum;

        *poreGradientIt = diffIntensity >= 0 ? diffIntensity : 0;
        *poreGradientRankIt = eRankMax;
    }
}

geo2d::Doublearray PoreDetection::calcPrincipalComponents(const image::BImage& image, const geo2d::Blobarray& blobs)
{
    geo2d::Doublearray pcRatios;
    std::vector<MajorAxes> majorAxes;
    PoreStatistics poreStatistics;
    // determine principal axes
    for (geo2d::Blob blob : blobs.getData())
    {
        const geo2d::Point boundingBoxStart (blob.xmin, blob.ymin);
        const geo2d::Point boundingBoxEnd   (blob.xmax, blob.ymax);
        const geo2d::Rect  boundingBox      (boundingBoxStart, boundingBoxEnd);

        if (boundingBox.isEmpty() == true)
        {
            majorAxes.push_back(MajorAxes()); // dummy value 1 will lead to bad rank as done in PoreStatistics::calcEigenValues ()
            continue;
        }

        poreStatistics.reset();
        poreStatistics.calcMomentsV2(boundingBox, image);
        poreStatistics.calcEigenValues();
        poreStatistics.calcEigenVectors();
        poreStatistics.calcCenterOfMass();
        majorAxes.push_back(poreStatistics.getMajorAxes());
    }

    pcRatios.assign(majorAxes.size(), 0, eRankMax); // we do not obtain rank information, however all input should be ok

    auto axesRatioOutIt     (pcRatios.getData().begin());
    auto axesRatioRankOutIt (pcRatios.getRank().begin());

    // determine pc ratio
    for (MajorAxes majorAxis : majorAxes)
    {
        poco_assert_dbg(majorAxis.m_oEigenVector2Length != 0); // should be asserted in calculation in PoreStatistics

        *axesRatioOutIt = majorAxis.m_oEigenVector1Length / majorAxis.m_oEigenVector2Length;

        if (majorAxis.m_oEigenVector2Length == PoreStatistics::m_oEvBadValue) // dummy value used in PoreStatistics::calcEigenValues ()
        {
            *axesRatioRankOutIt = eRankMin;
        }

        ++axesRatioOutIt;
        ++axesRatioRankOutIt;
    }
    return pcRatios;
}

std::pair<uint, double> PoreDetection::classify(const std::vector<double>& area, uint parameterSet)
{
    using namespace std::placeholders;

    const geo2d::Doublearray& boundingBoxDX = m_boundingBoxDX[parameterSet];
    const geo2d::Doublearray& boundingBoxDY = m_boundingBoxDY[parameterSet];
    const geo2d::Doublearray& gradient = m_poreGradient[parameterSet];
    const geo2d::Doublearray& pcRatio = m_pcRatios[parameterSet];
    const geo2d::Doublearray& surface = m_surface[parameterSet];

    // get data
    const uint numberCandidates = boundingBoxDX.size();	// could be any other feature

    const std::vector<double>& dataBoundingBoxDX = boundingBoxDX.getData();
    const std::vector<double>& dataBoundingBoxDY = boundingBoxDY.getData();
    const std::vector<double>& dataPcRatio = pcRatio.getData();
    const std::vector<double>& dataGradient = gradient.getData();
    const std::vector<double>& dataSurface = surface.getData();

    const std::vector<int>& rankBoundingBoxDX = boundingBoxDX.getRank();
    const std::vector<int>& rankBoundingBoxDY = boundingBoxDY.getRank();
    const std::vector<int>& rankPcRatio = pcRatio.getRank();
    const std::vector<int>& rankGradient = gradient.getRank();
    const std::vector<int>& rankSurface = surface.getRank();

    // assert equal number of features
    poco_assert_dbg(boundingBoxDX.size() == numberCandidates);
    poco_assert_dbg(boundingBoxDY.size() == numberCandidates);
    poco_assert_dbg(pcRatio.size() == numberCandidates);
    poco_assert_dbg(gradient.size() == numberCandidates);
    poco_assert_dbg(surface.size() == numberCandidates);

    const auto resetVector([numberCandidates](std::vector<PoreClassType>& p_oVec) { p_oVec.assign(numberCandidates, PoreClassType::ePore); }); // reset to ePore

    std::vector<int> classMergedRank;
    const FeatureRange& featureRange = m_featureRange[parameterSet];
    std::array<std::vector<PoreClassType>, eNbFeatureTypes-1>& classByFeature = m_classByFeature[parameterSet];
    std::vector<PoreClassType>& classMerged = m_classMerged[parameterSet];

    std::for_each(std::begin(classByFeature), std::end(classByFeature), resetVector); // reset feature classification result, nb candidates may have changed
    classMerged.assign(numberCandidates, PoreClassType::ePore); // reset classification result, nb candidates may have changed
    classMergedRank.assign(numberCandidates, eRankMax); // reset classification result rank, nb candidates may have changed. eRankMax!

    const auto classifyByRange = [&](double feature, geo2d::Range1d featureRange)->PoreClassType {
        return featureRange.contains(feature) ? ePore : eNoPore; }; // if a feature value lies in range, it is a pore

    // calculate feature BoundingBox
    auto classByFeatureXIt = classByFeature[0].begin();
    auto classByFeatureYIt = classByFeature[1].begin();
    for (auto bbX : dataBoundingBoxDX)
    {
        *classByFeatureXIt = classifyByRange(bbX, featureRange.boundingBoxDX);
        classByFeatureXIt++;
    }
    for (auto bbY : dataBoundingBoxDY)
    {
        *classByFeatureYIt = classifyByRange(bbY, featureRange.boundingBoxDY);
        classByFeatureYIt++;
    }

    for (size_t i = 0; i < numberCandidates; i++)
    {
        m_classByFeature[parameterSet][eBoundingBoxDX-1][i] = featureRange.boundingBoxDX.contains(dataBoundingBoxDX[i]) ? ePore : eNoPore;
        m_classByFeature[parameterSet][eBoundingBoxDY-1][i] = featureRange.boundingBoxDY.contains(dataBoundingBoxDY[i]) ? ePore : eNoPore;
        m_classByFeature[parameterSet][ePcRatio-1][i] = featureRange.pcRatio.contains(dataPcRatio[i]) ? ePore : eNoPore;
        m_classByFeature[parameterSet][eGradient-1][i] = featureRange.gradient.contains(dataGradient[i]) ? ePore : eNoPore;
        m_classByFeature[parameterSet][eSurface-1][i] = featureRange.surface.contains(dataSurface[i]) ? ePore : eNoPore;
    }

    // calculate final classification by ANDing individual classification

    const auto mergeClassification([](PoreClassType oFirst, PoreClassType oSecond)->PoreClassType { return (oFirst == ePore && oSecond == ePore) ? ePore : eNoPore; });

    std::transform(std::begin(classByFeature[0]), std::end(classByFeature[0]), std::begin(classMerged), std::begin(classMerged), mergeClassification);
    std::transform(std::begin(classByFeature[1]), std::end(classByFeature[1]), std::begin(classMerged), std::begin(classMerged), mergeClassification);
    std::transform(std::begin(classByFeature[2]), std::end(classByFeature[2]), std::begin(classMerged), std::begin(classMerged), mergeClassification);
    std::transform(std::begin(classByFeature[3]), std::end(classByFeature[3]), std::begin(classMerged), std::begin(classMerged), mergeClassification);
    std::transform(std::begin(classByFeature[4]), std::end(classByFeature[4]), std::begin(classMerged), std::begin(classMerged), mergeClassification);

    // calculate rank by taking min rank of all features

    // std::min<int> does not work with std::transform in gcc 4.6:no matching function for call to 'transform(
    // std::vector<int>::const_iterator, std::vector<int>::const_iterator, std::vector<int>::iterator, std::vector<int>::iterator, <unresolved overloaded function type>)'
    const auto min([](int a, int b)->int {return std::min(a,b); });

    std::transform(std::begin(rankBoundingBoxDX), std::end(rankBoundingBoxDX), std::begin(classMergedRank), std::begin(classMergedRank), min);
    std::transform(std::begin(rankBoundingBoxDY), std::end(rankBoundingBoxDY), std::begin(classMergedRank), std::begin(classMergedRank), min);
    std::transform(std::begin(rankPcRatio), std::end(rankPcRatio), std::begin(classMergedRank), std::begin(classMergedRank), min);
    std::transform(std::begin(rankGradient), std::end(rankGradient), std::begin(classMergedRank), std::begin(classMergedRank), min);
    std::transform(std::begin(rankSurface), std::end(rankSurface), std::begin(classMergedRank), std::begin(classMergedRank), min);

    // signal number of pores as result - and nio if no pore classified
    uint numberPoresNotBadRank = 0;
    double maxPoreSize = -1.0;

    auto areaIt = std::begin(area);
    auto classMergedRankIt = std::begin(classMergedRank);
    for (auto classMergedIt = std::begin(classMerged); classMergedIt != std::end(classMerged); ++classMergedIt, ++classMergedRankIt, ++areaIt)
    {
        if (*classMergedIt == ePore && *classMergedRankIt != eRankMin)
        {
            ++numberPoresNotBadRank;
            if (maxPoreSize < *areaIt)
            {
                maxPoreSize = *areaIt;
            }
        }
    }

    return std::make_pair(numberPoresNotBadRank, maxPoreSize);
}

void PoreDetection::classifyScaled(const std::vector<double>& area, const int parameterScaling, const int parameterSet)
{
    /**
    * @brief    Dilates or erodes a range by a given percentage. Eg [50, 100] and 10 percent -> [45, 110]; -10 percent -> [55, 90].
    * @param    p_oRange Percentage [-100, +100] Negative value means erosion, positive value dilation.
    * @return   Dilated or eroded range.
    */
    const auto	dilateOrErodeByPercent = [](geo2d::Range1d range, const int parameterScaling) {
        const auto	scaleFactorStart = 1. - parameterScaling * 0.01;
        const auto	scaleFactorEnd = 1. + parameterScaling * 0.01;

        return geo2d::Range1d {range.start() * scaleFactorStart, range.end() * scaleFactorEnd};
    };
    FeatureRange& featureRangeScaled = m_featureRangeScaled[parameterSet];
    const FeatureRange& featureRange = m_featureRange[parameterSet];

    featureRangeScaled.boundingBoxDX = dilateOrErodeByPercent(featureRange.boundingBoxDX, parameterScaling);
    featureRangeScaled.boundingBoxDY = dilateOrErodeByPercent(featureRange.boundingBoxDY, parameterScaling);
    featureRangeScaled.gradient = dilateOrErodeByPercent(featureRange.gradient, parameterScaling);
    featureRangeScaled.pcRatio = dilateOrErodeByPercent(featureRange.pcRatio, parameterScaling);
    featureRangeScaled.surface = dilateOrErodeByPercent(featureRange.surface, parameterScaling);

    const auto classByFeatureNotScaled = m_classByFeature[parameterSet];
    const auto classMergedNotScaled = m_classMerged[parameterSet];

    classify(area, parameterSet);

    // mark if classification result per feature has changed
    auto oItClassNotScaled = std::begin(classByFeatureNotScaled);
    for (auto& rClass : m_classByFeature[parameterSet])
    {
        auto	oItResultNotScaled = std::begin(*oItClassNotScaled);
        for (auto& rResult : rClass)
        {
            if (*oItResultNotScaled != rResult)
            {
                rResult = rResult == ePore ? ePoreIfScaled : eNoPoreIfScaled;
            }

            ++oItResultNotScaled;
        }

        ++oItClassNotScaled;
    }

    // mark if merged classification result has changed
    auto oItResultMergedNotScaled = std::begin(classMergedNotScaled);
    for (auto& rResultMerged : m_classMerged[parameterSet])
    {
        if (*oItResultMergedNotScaled != rResultMerged)
        {
            rResultMerged = rResultMerged == ePore ? ePoreIfScaled : eNoPoreIfScaled;
        }

        ++oItResultMergedNotScaled;
    }
}

void PoreDetection::paint()
{
    if (m_oVerbosity < eLow || m_smpTrafo.isNull())
    {
        return;
    }

    const interface::Trafo& trafo (*m_smpTrafo);
    image::OverlayCanvas& overlayCanvas (canvas<image::OverlayCanvas>(m_oCounter));
    image::OverlayLayer&  layerContour (overlayCanvas.getLayerContour());
    image::OverlayLayer&  layerImage (overlayCanvas.getLayerImage());
    image::OverlayLayer&  layerInfoBox(overlayCanvas.getLayerInfoBox());
    image::OverlayLayer&  layerLine(overlayCanvas.getLayerLine());

    const auto imagePosition = trafo(geo2d::Point(0, 0));
    auto title = image::OverlayText("Binarized image", image::Font(), geo2d::Rect(150, 18), image::Color::Black());
    for (uint i = 0; i < m_activeParameterSets; i++)
    {
        if (m_visualize[i] == Visualize::binarizeImage)
        {
            layerImage.add<image::OverlayImage>(imagePosition, m_binaryImage[i], title);
            break;
        }
    }

    title = image::OverlayText("Morphology image", image::Font(), geo2d::Rect(150, 18), image::Color::Black());
    for (uint i = 0; i < m_activeParameterSets; i++)
    {
        if (m_visualize[i] == Visualize::morphologyImage)
        {
            layerImage.add<image::OverlayImage>(imagePosition, m_morphologyImage[i], title);
            break;
        }
    }

    const uint numberFeatureTypes = FeatureType::eNbFeatureTypes - 1; // -1 because feature size is not used
    const std::array<std::string, numberFeatureTypes> featureTypeString = { {"DX", "DY", "PcRatio", "Contrast", "Surface"} };
    const std::array<std::string, numberFeatureTypes> featureUnitString = {interface::g_oLangKeyUnitMm, interface::g_oLangKeyUnitMm, interface::g_oLangKeyUnitNone,
        interface::g_oLangKeyUnitNone, interface::g_oLangKeyUnitNone};

    std::vector<std::array<std::vector<double>::const_iterator, eNbFeatureTypes>> featureCIts;
    for (uint i = 0; i < m_activeParameterSets; ++i)
    {
        const std::array<std::vector<double>::const_iterator, eNbFeatureTypes> featureCIt = {{m_boundingBoxDX[i].getData().cbegin(), m_boundingBoxDY[i].getData().cbegin(),
            m_pcRatios[i].getData().cbegin(), m_poreGradient[i].getData().cbegin(), m_surface[i].getData().cbegin()}};
        featureCIts.push_back(featureCIt);
    }

    // these colors are used to paint the blob contours and the feature strings depending on classification result, see oPoreClassType
    static const auto& poreClassTypeToColor = std::array<image::Color, ePoreClassTypeMax + 1> {{
        image::Color::m_oScarletRed, image::Color::m_oChameleonDark, image::Color::m_oSkyBlue, image::Color::m_oMagenta}};

    // prepare one info box per candidate per parameter set
    for (uint parameterSet = 0; parameterSet < m_activeParameterSets; parameterSet++)
    {
        if (m_visualize[parameterSet] == Visualize::none)
        {
            continue;
        }
        for (uint candidate = 0; candidate < m_blobs[parameterSet].getData().size(); candidate++)
        {
            const geo2d::Blob& blob = m_blobs[parameterSet].getData()[candidate];
            // get the bounding box
            const auto boundingBoxStart = geo2d::Point(blob.xmin, blob.ymin);
            const auto boundingBoxEnd = geo2d::Point(blob.xmax, blob.ymax);
            const auto boundingBox = geo2d::Rect(boundingBoxStart, boundingBoxEnd);

            auto featureLines = std::vector<image::OverlayText>(numberFeatureTypes + 1);

            // Filter ID
            featureLines[0] = image::OverlayText(id().toString() + ":FILTERGUID:0", image::Font(12, true, false, "Courier New"), trafo(geo2d::Rect(10, 10, 100, 200)), image::Color::Black(), 0);

            // Output for special "value info box" over the camera image
            const auto classByFeature = m_classByFeature[parameterSet];

            for (uint i = 0; i < numberFeatureTypes; ++i)
            {
                const image::Color txtColor = poreClassTypeToColor[classByFeature[i][candidate]];
                std::ostringstream oMsg;
                oMsg << featureTypeString[i] << ":" << featureUnitString[i] << ":" << // colon is key-key-value delimiter
                    std::setprecision(2) << std::fixed << featureCIts[parameterSet][i][candidate];
                featureLines[i + 1] = image::OverlayText(oMsg.str(), image::Font(12), geo2d::Rect(), txtColor, parameterSet * m_maxParameterSets + i);
            }

            // paint corresponding info box
            auto contentType = parameterSet == 0 ? image::ePoreSet1 : (parameterSet == 1 ? image::ePoreSet2 : image::ePoreSet3);
            layerInfoBox.add<image::OverlayInfoBox>(contentType, candidate, std::move(featureLines), trafo(boundingBox));

            if (m_oVerbosity > eLow || m_visualize[parameterSet] == Visualize::contour)
            {
                // paint bounding box and contour in green, red or blue
                int color = 1; // green
                for (const auto& classMerged : m_classMerged[parameterSet])
                {
                    if (classMerged == ePore)
                    {
                        color = 0; // red
                        break;
                    }
                    if (classMerged == ePoreIfScaled)
                    {
                        color = 2; // blue
                    }
                }
                const auto contourColor = poreClassTypeToColor[color];

                if (m_oVerbosity > eLow)
                {
                    layerLine.add<image::OverlayRectangle>(trafo(boundingBox), contourColor);
                }

                if (m_visualize[parameterSet] == Visualize::contour)
                {
                    for (auto oContourPosIt = std::begin(blob.m_oContour); oContourPosIt != std::end(blob.m_oContour); oContourPosIt++)
                    {
                        layerContour.add<image::OverlayPoint>(trafo(*oContourPosIt), contourColor);
                    }
                }
            }
        }
    }
}

void PoreDetection::getFeatures ( std::vector< precitec::geo2d::Doublearray >& dx, std::vector< precitec::geo2d::Doublearray >& dy, std::vector< precitec::geo2d::Doublearray >& pcRatio,
                                        std::vector< precitec::geo2d::Doublearray >& gradient, std::vector< precitec::geo2d::Doublearray >& variance ) const
{
    for (uint i = 0; i < m_activeParameterSets; i++)
    {
        dx.push_back(m_boundingBoxDX[i]);
        dy.push_back(m_boundingBoxDY[i]);
        pcRatio.push_back(m_pcRatios[i]);
        gradient.push_back(m_poreGradient[i]);
        variance.push_back(m_surface[i]);
    }
}
}
}
