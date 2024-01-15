#include "fliplib/Fliplib.h"
#include "fliplib/BaseFilterInterface.h"

#include "blobSource.h"
#include "doubleSource.h"
#include "houghPPCandidateSource.h"
#include "imageSource.h"
#include "pointListSource.h"
#include "poorPenetrationCandidateSource.h"
#include "sampleSource.h"
#include "seamFindingSource.h"
#include "startEndInfoSource.h"
#include "surfaceInfoSource.h"
#include "blobSink.h"
#include "doubleSink.h"
#include "houghPPCandidateSink.h"
#include "imageSink.h"
#include "pointListSink.h"
#include "poorPenetrationCandidateSink.h"
#include "sampleSink.h"
#include "seamFindingSink.h"
#include "startEndInfoSink.h"
#include "surfaceInfoSink.h"
#include "lineSink.h"
#include "lineSource.h"

FLIPLIB_GET_VERSION

FLIPLIB_BEGIN_MANIFEST(fliplib::BaseFilterInterface)
    FLIPLIB_EXPORT_CLASS(precitec::filter::bridges::BlobsSource)
    FLIPLIB_EXPORT_CLASS(precitec::filter::bridges::DoubleSource)
    FLIPLIB_EXPORT_CLASS(precitec::filter::bridges::HoughPPCandidateSource)
    FLIPLIB_EXPORT_CLASS(precitec::filter::bridges::ImageSource)
    FLIPLIB_EXPORT_CLASS(precitec::filter::bridges::PointListSource)
    FLIPLIB_EXPORT_CLASS(precitec::filter::bridges::PoorPenetrationCandidateSource)
    FLIPLIB_EXPORT_CLASS(precitec::filter::bridges::SampleSource)
    FLIPLIB_EXPORT_CLASS(precitec::filter::bridges::SeamFindingSource)
    FLIPLIB_EXPORT_CLASS(precitec::filter::bridges::StartEndInfoSource)
    FLIPLIB_EXPORT_CLASS(precitec::filter::bridges::SurfaceInfoSource)
    FLIPLIB_EXPORT_CLASS(precitec::filter::bridges::BlobsSink)
    FLIPLIB_EXPORT_CLASS(precitec::filter::bridges::DoubleSink)
    FLIPLIB_EXPORT_CLASS(precitec::filter::bridges::HoughPPCandidateSink)
    FLIPLIB_EXPORT_CLASS(precitec::filter::bridges::ImageSink)
    FLIPLIB_EXPORT_CLASS(precitec::filter::bridges::PointListSink)
    FLIPLIB_EXPORT_CLASS(precitec::filter::bridges::PoorPenetrationCandidateSink)
    FLIPLIB_EXPORT_CLASS(precitec::filter::bridges::SampleSink)
    FLIPLIB_EXPORT_CLASS(precitec::filter::bridges::SeamFindingSink)
    FLIPLIB_EXPORT_CLASS(precitec::filter::bridges::StartEndInfoSink)
    FLIPLIB_EXPORT_CLASS(precitec::filter::bridges::SurfaceInfoSink)
    FLIPLIB_EXPORT_CLASS(precitec::filter::bridges::LineSink)
    FLIPLIB_EXPORT_CLASS(precitec::filter::bridges::LineSource)
FLIPLIB_END_MANIFEST
