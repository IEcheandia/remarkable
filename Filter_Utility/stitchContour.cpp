#include "stitchContour.h"

#include "module/moduleLogger.h"

#include "fliplib/TypeToDataTypeImpl.h"
#include "overlay/overlayCanvas.h"
#include "overlay/overlayPrimitive.h"

#include <vector>
#include <cmath>
#include <limits>

#define FILTER_ID                 "94b792f3-8944-4106-9cac-3c342f725527"
#define VARIANT_ID                "0932c7b2-98e0-4a54-9447-9d59b10e1090"

#define PIPE_ID_CONTOUR1          "e08ddfdd-6bd1-4b02-8605-284b17cb69a4"
#define PIPE_ID_CONTOUR2          "cd507870-f2ac-4c92-9e07-929873ed1446"
#define PIPE_ID_CONTOUROUT        "1240c8b4-6936-4043-a6da-9d26ce71ae5e"

typedef std::vector<precitec::geo2d::DPoint> ContourType;

/**
Given a point (px, py) and a line segment (lx1, ly1) to (lx2, ly2),
find the closest distance from the point to the line segment and the
corresponding location (projx, projy) that lies on the line segment.
 **/
double plDistance(double px, double py, double lx1, double ly1, double lx2, double ly2, double& projx, double& projy)
{
    const auto a = px - lx1;
    const auto b = py - ly1;
    const auto c = lx2 - lx1;
    const auto d = ly2 - ly1;

    const auto dot = a * c + b * d;
    const auto l2 = c * c + d * d;

    double bcos = -1;

    if (dot != 0)
    {
        bcos = dot / l2;
    }

    if (bcos < 0)
    {
        projx = lx1;
        projy = ly1;
    }
    else if (bcos > 1)
    {
        projx = lx2;
        projy = ly2;
    }
    else
    {
        projx = lx1 + bcos * c;
        projy = ly1 + bcos * d;
    }

    const auto dx = px - projx;
    const auto dy = py - projy;

    return std::sqrt(dx * dx + dy * dy);
}

/**
Given a point and a contour, find the closest distance from
the point to the contour and the corresponding location
(projx, projy) that lies on the contour. Determine the index
of point that lies previous to (projx, projy).
 **/
double pcDistance(double px, double py, const ContourType& contour, double& projx, double& projy, int& index)
{
    double dmin = std::numeric_limits<double>::infinity();
    projx = px;
    projy = py;
    index = 0;

    for (std::size_t i = 1; i < contour.size(); ++i)
    {
        double x;
        double y;
        const auto d = plDistance(px, py, contour[i-1].x, contour[i-1].y, contour[i].x, contour[i].y, x, y);

        if (d < dmin)
        {
            dmin = d;
            projx = x;
            projy = y;
            index = i - 1;
        }
    }

    return dmin;
}

/**
For each point in the contour, find the accumulated length
between the point and the start of contour.
 **/
std::vector<double> integrateContour(const ContourType& contourIn)
{
    if (contourIn.empty())
    {
        return {0};
    }

    std::vector<double> length(contourIn.size(), 0);

    for (std::size_t i = 1; i < contourIn.size(); ++i)
    {
        const auto& p1 = contourIn[i - 1];
        const auto& p2 = contourIn[i];

        const auto dx = p2.x - p1.x;
        const auto dy = p2.y - p1.y;
        length[i] = length[i - 1] + std::sqrt(dx * dx + dy * dy);
    }
    return length;
}

/**
Stitch the end of contour1 with the start of contour2. If
the start of contour2 overlaps with the end of contour1,
then remove the overlapping part of contour2. If 'stitchBothEnds'
is set to true, the start of contour1 will be stitched with
the end of contour2 as well.
 **/
ContourType stitchContour(const ContourType& contour1, const ContourType& contour2, bool stitchBothEnds)
{
    if (contour1.empty())
    {
        return contour2;
    }
    if (contour2.empty())
    {
        return contour1;
    }

    // project starting point of contour 2 onto contour 1
    double projx1;
    double projy1;
    int index1;
    pcDistance(contour2[0].x, contour2[0].y, contour1, projx1, projy1, index1);

    // project end point of contour 1 onto contour 2
    double projx2;
    double projy2;
    int index2;
    pcDistance(contour1.back().x, contour1.back().y, contour2, projx2, projy2, index2);

    // divide contour 1 into 2 parts contour1A and contour1B
    ContourType contour1A = {contour1.begin(), contour1.begin() + index1 + 1};
    contour1A.push_back({projx1, projy1});
    ContourType contour1B = {{projx1, projy1}};
    contour1B.insert(contour1B.end(), contour1.begin() + index1 + 1, contour1.end());

    // divide contour 2 into 2 parts contour2A and contour2B
    ContourType contour2A = {contour2.begin(), contour2.begin() + index2 + 1};
    contour2A.push_back({projx2, projy2});
    ContourType contour2B = {{projx2, projy2}};
    contour2B.insert(contour2B.end(), contour2.begin() + index2 + 1, contour2.end());

    // if stitch on both ends, then stich 2B with 1A
    // project end point of contour 2B onto contour 1A
    double projx1a;
    double projy1a;
    int index1a;
    pcDistance(contour2B.back().x, contour2B.back().y, contour1A, projx1a, projy1a, index1a);

    // project starting point of contour 1A onto contour 2B
    double projx2b;
    double projy2b;
    int index2b;
    pcDistance(contour1A[0].x, contour1A[0].y, contour2B, projx2b, projy2b, index2b);

    // divide contour1A into two parts contour1AA and contour1AB
    ContourType contour1AA = {contour1A.begin(), contour1A.begin() + index1a + 1};
    contour1AA.push_back({projx1a, projy1a});
    ContourType contour1AB = {{projx1a, projy1a}};
    contour1AB.insert(contour1AB.end(), contour1A.begin() + index1a + 1, contour1A.end());

    // divide contour2B into two parts contour2BA and contour2BB
    ContourType contour2BA = {contour2B.begin(), contour2B.begin() + index2b + 1};
    contour2BA.push_back({projx2b, projy2b});
    ContourType contour2BB = {{projx2b, projy2b}};
    contour2BB.insert(contour2BB.end(), contour2B.begin() + index2b + 1, contour2B.end());

    const auto length1AA = integrateContour(contour1AA).back();
    const auto length1AB = integrateContour(contour1AB).back();
    const auto length2BA = integrateContour(contour2BA).back();
    const auto length2BB = integrateContour(contour2BB).back();

    ContourType contourOut;

    if (stitchBothEnds)
    {
        contourOut.insert(contourOut.end(), contour1AB.begin(), contour1AB.end());
        contourOut.insert(contourOut.end(), contour1B.begin(), contour1B.end());
        contourOut.insert(contourOut.end(), contour2B.begin(), contour2B.end());

        if (length1AA == 0 || length1AB == 0 || length2BA == 0 || length2BB == 0)
        {
            contourOut.insert(contourOut.end(), contour1AA.begin(), contour1AA.end());
        }
    }
    else
    {
        contourOut.insert(contourOut.end(), contour1.begin(), contour1.end());
        contourOut.insert(contourOut.end(), contour2B.begin(), contour2B.end());
    }

    return contourOut;
}

namespace precitec
{
namespace filter
{

StitchContour::StitchContour()
    : TransformFilter("StitchContour", Poco::UUID(FILTER_ID))
    , m_contour1(nullptr)
    , m_contour2(nullptr)
    , m_contourOut(this, "ContourOut")
    , m_stitchBothEnds(false)
{
    parameters_.add("StitchBothEnds", fliplib::Parameter::TYPE_bool, m_stitchBothEnds);

    setInPipeConnectors({
        {Poco::UUID(PIPE_ID_CONTOUR1), m_contour1, "Contour1", 1, "Contour1"},
        {Poco::UUID(PIPE_ID_CONTOUR2), m_contour2, "Contour2", 1, "Contour2"},
    });

    setOutPipeConnectors({
        {Poco::UUID(PIPE_ID_CONTOUROUT), &m_contourOut, "ContourOut", 1, "ContourOut"},
    });

    setVariantID(Poco::UUID(VARIANT_ID));
}

void StitchContour::setParameter()
{
    TransformFilter::setParameter();

    m_stitchBothEnds = parameters_.getParameter("StitchBothEnds").convert<bool>();
}

void StitchContour::paint()
{
    if (m_oVerbosity == eNone)
    {
        return;
    }
}

bool StitchContour::subscribe(fliplib::BasePipe& pipe, int group)
{
    if (pipe.tag() == "Contour1")
    {
        m_contour1 = dynamic_cast<fliplib::SynchronePipe<interface::GeoVecAnnotatedDPointarray>*>(&pipe);
    }
    else if (pipe.tag() == "Contour2")
    {
        m_contour2 = dynamic_cast<fliplib::SynchronePipe<interface::GeoVecAnnotatedDPointarray>*>(&pipe);
    }

    return BaseFilter::subscribe(pipe, group);
}

void StitchContour::proceedGroup(const void* sender, fliplib::PipeGroupEventArgs& event)
{
    const auto &contour1Array = m_contour1->read(m_oCounter);
    const auto &contour2Array = m_contour2->read(m_oCounter);

    interface::GeoVecAnnotatedDPointarray contourOutArray(contour1Array.context(), {}, contour1Array.analysisResult(), contour1Array.rank());

    // safety check
    if (contour1Array.ref().empty() || contour2Array.ref().empty())
    {

    }

    const auto contour1 = contour1Array.ref().front().getData();
    const auto contour2 = contour2Array.ref().front().getData();

    const auto contour = stitchContour(contour1, contour2, m_stitchBothEnds);

    geo2d::TAnnotatedArray<geo2d::DPoint> contourOut;
    contourOut.getData() = contour;
    contourOut.getRank() = std::vector<int>(contour.size(), eRankMax);

    contourOutArray.ref().emplace_back(contourOut);

    preSignalAction();
    m_contourOut.signal(contourOutArray);
}


} //namespace filter
} //namespace precitec
