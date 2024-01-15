#pragma once
#include "fliplib/PipeConnector.h"
#include "common/frame.h"

namespace fliplib
{

template<>
struct TypeToDataType<precitec::interface::ImageFrame>
{
    static const PipeConnector::DataType Type = PipeConnector::DataType::Image;
};

template<>
struct TypeToDataType<precitec::interface::GeoDoublearray>
{
    static const PipeConnector::DataType Type = PipeConnector::DataType::Double;
};

template<>
struct TypeToDataType<precitec::interface::SampleFrame>
{
    static const PipeConnector::DataType Type = PipeConnector::DataType::Sample;
};

template<>
struct TypeToDataType<precitec::interface::GeoSurfaceInfoarray>      //SurfaceInfo
{
    static const PipeConnector::DataType Type = PipeConnector::DataType::SurfaceInfo;
};

template<>
struct TypeToDataType<precitec::interface::GeoPoorPenetrationCandidatearray>
{
    static const PipeConnector::DataType Type = PipeConnector::DataType::PoorPenetrationCandidate;
};

template<>
struct TypeToDataType<precitec::interface::GeoVecDoublearray>
{
    static const PipeConnector::DataType Type = PipeConnector::DataType::Line;
};

template<>
struct TypeToDataType<precitec::interface::ResultDoubleArray>
{
    static const PipeConnector::DataType Type = PipeConnector::DataType::Double;
};

template<>
struct TypeToDataType<precitec::interface::GeoSeamFindingarray>
{
    static const PipeConnector::DataType Type = PipeConnector::DataType::SeamFinding;
};

template<>
struct TypeToDataType<precitec::interface::GeoVecAnnotatedDPointarray>
{
    static const PipeConnector::DataType Type = PipeConnector::DataType::PointList;
};

template<>
struct TypeToDataType<precitec::interface::GeoStartEndInfoarray>
{
    static const PipeConnector::DataType Type = PipeConnector::DataType::StartEndInfo;
};

template<>
struct TypeToDataType<precitec::interface::GeoHoughPPCandidatearray>
{
    static const PipeConnector::DataType Type = PipeConnector::DataType::HoughPPCandidate;
};

template<>
struct TypeToDataType<precitec::interface::GeoLineModelarray>
{
    static const PipeConnector::DataType Type = PipeConnector::DataType::LineModel;
};

template<>
struct TypeToDataType<precitec::interface::GeoBlobarray>
{
    static const PipeConnector::DataType Type = PipeConnector::DataType::Blobs;
};

}
