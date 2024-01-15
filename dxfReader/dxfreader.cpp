#include "dxfreader.h"
#include <sstream>
#include <locale>
#include <cassert>
#include <optional>
#include <map>
#include <array>
#include <utility>
#include <fstream>
#include <tinysplinecxx.h>

#define _USE_MATH_DEFINES
#include <math.h>

#ifndef NDEBUG
#define _DEBUG
#endif

namespace nsDxfReader
{

bool equals(CircularDesc const& a, CircularDesc const& b, double maxError)
{
    assert(maxError > 0);
    double const sqMaxError = maxError * maxError;

    if (SqDistance(a.center, b.center) > sqMaxError)
        return false;

    {
        double const sqRa = SqLength(a.minor);
        double const sqRb = SqLength(b.minor);
        if (std::abs(sqRa - sqRb) <= sqMaxError)
        { // the minor axes of both are of (almost) equal length
            if (std::abs(sqRa - SqLength(a.major)) <= sqMaxError && std::abs(sqRb - SqLength(b.major)) <= sqMaxError)
            { // both have a major axis that has (almost) the same length as the minor axes
                if (std::abs(Dot(a.minor, a.major)) <= sqMaxError && std::abs(Dot(b.minor, b.major)) <= sqMaxError)
                    return true; // both axes are (almost) orthogonal, so center and radius are sufficient to compare them (rotation of a circle does not matter!)
            }
        }
    }

    // an axis has the same effect if it is inverted, so we invert them to point to the positive x/y direction for the major dimension
    auto axisFlipper = [](Vec2 v)
    {
        if (std::abs(v.x) > std::abs(v.y))
        {
            if (v.x < 0)
                return v * -1;
        }
        else
        {
            if (v.y < 0)
                return v * -1;
        }

        return v;
    };

    // order of axes does not matter for the comparision, so we sort them by length
    auto sortByLength = [](Vec2& a, Vec2& b)
    {
        if (SqLength(a) > SqLength(b))
            std::swap(a, b);
    };

    Vec2 a1 = axisFlipper(a.minor), a2 = axisFlipper(a.major);
    Vec2 b1 = axisFlipper(b.minor), b2 = axisFlipper(b.major);
    sortByLength(a1, a2);
    sortByLength(b1, b2);

    return SqLength(a1 - b1) <= sqMaxError && SqLength(a2 - b2) <= sqMaxError;
}

namespace
{

// quick and dirty testing
#ifdef _DEBUG
int circular_equals_test()
{
    CircularDesc c;
    c.major = {1, 0};
    c.minor = {0, 1};

    CircularDesc e;
    e.major = {10, 0};
    e.minor = {0, 1};

    double const er = .001;
    assert(equals(c, c, er));

    auto const rot = Transform2::Rotation(1.3);
    {
        auto c2 = c;
        c2.major = rot * c2.major;
        c2.minor = rot * c2.minor;

        assert(equals(c2, c2, er));
        assert(equals(c, c2, er));

        c2.minor *= -1;
        assert(equals(c, c2, er));
        c2.major *= -1;
        assert(equals(c, c2, er));

        c2.center.x += er * 1.01;
        assert(!equals(c, c2, er));

        c2 = c;
        c2.major *= er * 1.01;
        c2.minor *= er * 1.01;
        assert(!equals(c, c2, er));

        c2 = c;
        c2.major *= 1 + er * .9;
        c2.minor *= 1 + er * .9;
        assert(equals(c, c2, er));

        c2 = c;
        c2.major *= 1 + er * 1.01;
        c2.minor *= 1 + er * 1.01;
        assert(equals(c2, c2, er));
        assert(!equals(c, c2, er));

        c2 = c;
        c2.major = rot * c2.major;
        assert(!equals(c, c2, er));
    }

    assert(equals(e, e, er));
    assert(!equals(c, e, er));

    {
        auto e2 = e;
        std::swap(e2.major, e2.minor);
        assert(equals(e2, e, er));
        e2.major *= -1;
        assert(equals(e2, e, er));
        e2.minor *= -1;
        assert(equals(e2, e, er));

        e2.major = rot * e.major;
        e2.minor = rot * e.minor;
        assert(!equals(e2, e, er));
    }

    return 0;
}

int _foo = circular_equals_test();
#endif

/**
Baseclass to read records from a DXF file.

Currently only implemented for ASCII files, but maybe support
for binary files will be necessary. So this interface is used
to separate record reading from their interpretation.
*/
class IRecordReader
{
public:
    virtual ~IRecordReader() {}
    virtual bool Next() = 0;

    virtual std::string Position() = 0;

    virtual bool Matches(int gc, char const* str) = 0;

    // returns the current "group code"
    virtual int Gc() = 0;

    // consumes a value of a specific type
    virtual int Int() = 0;
    virtual double Dbl() = 0;
    virtual std::string Str() = 0;
};

class AsciiRecordReaderException : public std::exception
{
    std::string const mMsg;
public:
    AsciiRecordReaderException(std::string msg)
        : mMsg(msg)
    {
    }
    virtual char const* what() const noexcept override { return mMsg.c_str(); }
};

class AsciiRecordReader : public IRecordReader
{
    std::istream& mStream;
    bool mEof = false;
    size_t mLineNum = 0;

    std::optional<std::istringstream> mLine;
    int mGc = 0;

    void _NextLine()
    {
        std::string line;
        std::getline(mStream, line);

        if (!line.empty() && line.back() == '\r')
            line.pop_back();

        if (!mLineNum)
        {
            if (mStream.fail())
                throw AsciiRecordReaderException("failed to read first line");

            if (line == "AutoCAD Binary DXF")
                throw AsciiRecordReaderException("attempt to read binary DXF file with ASCII reader");
        }

        if (mStream.bad())
            throw AsciiRecordReaderException("failed to read next line");

        ++mLineNum;
        mLine.emplace(std::move(line));
        mLine->imbue(std::locale("C"));
        if (mStream.eof())
            mEof = true;
    }

public:
    AsciiRecordReader(std::istream& s)
        : mStream{s}
    {
    }

    virtual bool Next() override
    {
        if (mEof)
            return false;

        _NextLine();
        if (mEof)
            return false;

        mGc = Int();
        _NextLine();

        if (Matches(0, "EOF"))
        {
            mEof = true;
            return false;
        }

        return true;
    }

    virtual std::string Position() override
    {
        std::ostringstream s;
        s << "line " << mLineNum;
        return s.str();
    }

    virtual int Gc() override
    {
        assert(mLineNum > 0);
        return mGc;
    }

    virtual bool Matches(int gc, char const* str) override
    {
        assert(mLine);
        return Gc() == gc && str == mLine->str();
    }

    virtual int Int() override
    {
        assert(mLineNum > 0);
        int val;
        *mLine >> val;
        if (!*mLine)
            throw AsciiRecordReaderException("failed to read integer value");
        return val;
    }

    virtual double Dbl() override
    {
        assert(mLineNum > 0);
        double val;
        *mLine >> val;
        if (!*mLine)
            throw AsciiRecordReaderException("failed to read numeric value");
        return val;
    }

    virtual std::string Str() override
    {
        assert(mLineNum > 0);
        std::string s;
        std::getline(*mLine, s);

        if (!*mLine && !mLine->eof())
            throw AsciiRecordReaderException("failed to read string value");
        return s;
    }
};

// See https://help.autodesk.com/view/OARX/2018/ENU/?guid=GUID-D99F1509-E4E4-47A3-8691-92EA07DC88F5
// and https://help.autodesk.com/view/OARX/2018/ENU/?guid=GUID-E19E5B42-0CC7-4EBA-B29F-5E1D595149EE
Transform2 Ocs(Vec3 extrusion)
{
    Vec3 const z = Normalized(extrusion);

    double const inv64 = 1. / 64.;

    Vec3 const x = Cross(std::abs(z.x) < inv64 && std::abs(z.y) < inv64 ? Vec3{0, 1, 0} : Vec3{0, 0, 1}, z);
    Vec3 const y = Normalized(Cross(z, x));

    Transform2 t;
    t.x = x;
    t.y = y;
    return t;
}

struct Circle
{
    Point2 center;
    double radius = 0;
    Transform2 ocs;

    Point2 Plot(double u) const
    {
        return ocs * Point2{center.x + std::cos(u) * radius,
                            center.y + std::sin(u) * radius};
    }

#if 0
	// NOTE: Only makes sense for untransformed circles.
	double AngleStepsize(double maxError) const
	{
		size_t n = 4;

		if (radius > maxError)
		{
			double a = std::acos((radius - maxError) / radius) * 2;
			size_t n2 = static_cast<size_t>(std::ceil((2 * M_PI) / a));
			n = std::max(n, n2);
		}

		return 2 * M_PI / n;
	}
#endif
};

struct Arc : public Circle
{
    double startAngle = 0;
    double endAngle = 0;
};

struct Ellipse
{
    Point2 center;
    Vec2 majEnd;           // endpoint of major axis
    double ratio = 0;      // ratio of minor axis to major axis
    double start = 0;      // in radians
    double end = 2 * M_PI; // in radians

    Point2 Plot(double u) const
    {
        Vec2 const x = majEnd;
        Vec2 const y = Ortho(x) * ratio;
        return center + x * std::cos(u) + y * std::sin(u);
    }
};

struct Line
{
    Point2 start, end;
};

struct PlPoint : public Point2
{
    double bulge = 0;

    PlPoint() {}

    PlPoint(double x, double y, double bulge)
        : Point2(x, y)
        , bulge(bulge)
    {
    }
};

struct PolyLine
{
    std::vector<PlPoint> points;
    bool closed = false;
};

struct Spline
{
    enum FlagBit
    {
        Closed = 1,   // Closed spline
        Peridoc = 2,  // Periodic spline
        Rational = 4, // Rational spline
        Planar = 8,
        Linear = 16, // (planar bit is also set)
    };

    int flags = 0; // bitwise combination of FlagBit values
    int degree = 3;
    std::vector<double> knotValues;
    std::vector<Point2> ctlPoints;
};

//std::ostream& operator<<(std::ostream& s, Point2 const& p)
//{
//    s << "(" << p.x << ", " << p.y << ")";
//    return s;
//}

//std::ostream& operator<<(std::ostream& s, Line const& l)
//{
//    s << l.start << "->" << l.end;
//    return s;
//}

//std::ostream& operator<<(std::ostream& s, PolyLine const& pl)
//{
//    bool first = true;
//    for (Point2 const& p : pl.points)
//    {
//        if (!first)
//            s << "->";
//        else
//            first = false;

//        s << p;
//    }
//    return s;
//}

double Deg2Rad(double deg)
{
    return deg * M_PI / 180.;
}

/**
Adaptive "stepper" for automatically choosing a sensible step size for approximating curved paths with line segments.
*/
template<class F>
class AdaptiveStepper
{
    double const mEsq;
    double const mMin;
    double const mMax;
    std::optional<double> const mMaxDist;
    double mPos;
    double mStep;
    Point2 mPoint;
    F mF;

    void _CurveStep()
    {
        {
            Point2 next = mF(mPos + mStep);
            Point2 idealMid = mF(mPos + mStep * .5);
            Point2 lineMid = Midpoint(mPoint, next);

            bool decreased = false;
            while (SqLength(idealMid - lineMid) > mEsq)
            { // stepsize is too large
                mStep *= .5;

                if (mStep <= mMin)
                {
                    mStep = mMin;
                    mPos += mStep;
                    mPoint = mF(mPos);
                    return;
                }

                decreased = true;
                next = idealMid;
                idealMid = mF(mPos + mStep * .5);
                lineMid = Midpoint(mPoint, next);
            }

            if (decreased)
            {
                mPos += mStep;
                mPoint = next;
                return;
            }
        }

        {
            Point2 next = mF(mPos + mStep * 2);
            Point2 idealMid = mF(mPos + mStep);
            Point2 lineMid = Midpoint(mPoint, next);

            double lastLen = 0;
            double curLen;
            while ((curLen = SqLength(idealMid - lineMid)) <= mEsq)
            { // stepsize is too small
                if (curLen <= lastLen)
                    break;
                lastLen = curLen;

                mStep *= 2;

                if (mStep >= mMax)
                {
                    mStep = mMax;
                    mPos += mStep;
                    mPoint = mF(mPos);
                    return;
                }

                idealMid = next;
                next = mF(mPos + mStep * 2);
                lineMid = Midpoint(mPoint, next);
            }

            mPos += mStep;
            mPoint = idealMid;
        }
    }

public:
    AdaptiveStepper(double maxError, std::optional<double> maxDist, double minStep, double maxStep, double start, F f)
        : mEsq(maxError * maxError)
        , mMin(minStep)
        , mMax(maxStep)
        , mMaxDist(maxDist)
        , mPos(start)
        , mStep(maxStep)
        , mPoint(f(start))
        , mF(f)
    {
    }

    void Next()
    {
        Point2 const oldPoint = mPoint;
        double const oldPos = mPos;

        _CurveStep();

        if (mMaxDist && Distance(oldPoint, mPoint) > *mMaxDist)
        { // We have a limit for the distance between points and the new point would be further away, so we have to step back to meet the acceptable distance

            // We perform a simple binary search to shrink/grow the step of the curve-parameter to
            // find a point that is very close to the desired distance.
            double a = oldPos;
            double b = mPos;
            for (int i = 0; i < 32; ++i)
            {
                mPos = (a + b) * .5;
                mPoint = mF(mPos);

                double d = Distance(oldPoint, mPoint);

                // NOTE: Technically it would make sense to use the requested accuray to decide when we are close enough.
                //       But this would most likely be confusing for users: Setting a high accuracy would cause the distance
                //       configuration to have no effect!
                //       So we simply do a fixed number of iterations to get "very close" to the desired value.
                /*
                double e = d - *mMaxDist;
                if (e * e < mEsq)
                    break;
                */

                if (d > *mMaxDist)
                    b = mPos;
                else
                    a = mPos;
            }
        }
    }

    Point2 const& CurPoint() const
    {
        return mPoint;
    }

    double CurPos() const
    {
        return mPos;
    }
};

struct Insert
{
    std::string blockName;
    Point2 pos;
    double xScale = 1;
    double yScale = 1;
    double rotation = 0;
    int cols = 1;
    int rows = 1;
    double colSpacing = 0;
    double rowSpacing = 0;
};

struct Entities
{
    std::vector<Line> mLines;
    std::vector<Circle> mCircles;
    std::vector<Arc> mArcs;
    std::vector<Ellipse> mEllipses;
    std::vector<PolyLine> mPolyLines;
    std::vector<Spline> mSplines;
    std::vector<Insert> mInserts;
};

struct Block
{
    enum FlagBit
    {
        Anonymous = 1,         // This is an anonymous block generated by hatching, associative dimensioning, other internal operations, or an application
        NonConstAttrs = 2,     // This block has non-constant attribute definitions (this bit is not set if the block has any attribute definitions that are constant, or has no attribute definitions at all)
        External = 4,          // This block is an external reference (xref)
        Overlay = 8,           // This block is an xref overlay
        ExternalDep = 16,      // This block is externally dependent
        ResolvedExtRef = 32,   // This is a resolved external reference, or dependent of an external reference (ignored on input)
        ReferencedExtRef = 64, // This definition is a referenced external reference (ignored on input)
    };

    std::string name;
    int flags = 0;
    Point2 base;
    std::string path;
    std::string description;

    Entities ents;
};

static constexpr std::array<double, 21> unit2MeterTable = {
    -1,                 // 0 = Unitless
    39.37007874,        // 1 = Inches
    3.280839895,        // 2 = Feet
    0.00062137119,      // 3 = Miles
    1000.0,             // 4 = Millimeters
    100.0,              // 5 = Centimeters
    1.0,                // 6 = Meters
    0.001,              // 7 = Kilometers
    -1,                 // 8 = Microinches = 1e-6 in
    -1,                 // 9 = Mils = 0.001 in
    1.093613298,        // 10 = Yards
    10000000000.0,      // 11 = Angstroms = 1e-10m
    1000000000.0,       // 12 = Nanometers = 1e-9m
    1000000.0,          // 13 = Microns = 1e-6m
    10.0,               // 14 = Decimeters = 0.1m
    0.1,                // 15 = Decameters = 10m
    0.01,               // 16 = Hectometers = 100m
    0.000000001,        // 17 = Gigameters = 1e+9 m
    1.0 / 149597870700, // 18 = Astronomical units = 149597870700m
    1.0 / 9.46e15,      // 19 = Light years = 9.46e15 m
    1.0 / 3.09e16       // 20 = Parsecs = 3.09e16 m
};

double UnitIdx2MeterScale(int unitIdx) // for cm this will be 100
{
    if (unitIdx < 1 || static_cast<size_t>(unitIdx) >= unit2MeterTable.size() || unit2MeterTable[unitIdx] <= 0)
    {
        std::ostringstream s;
        s << "Unsupported physical unit ($INSUNITS = " << unitIdx << ")";
        throw DxfReadError(s.str());
    }

    return unit2MeterTable[unitIdx];
}

void SampleLine(std::vector<Point2>& points, std::optional<double> _maxDist, Point2 start, Point2 end, std::function<void()> allocPoint)
{
    if (!_maxDist || !std::isfinite(*_maxDist))
    {
        allocPoint();
        points.push_back(end);
        return;
    }

    Vec2 const v = end - start;
    double const vlen = Length(v);
    double const maxDist = *_maxDist;
    double const n = vlen / maxDist;
    int const in = static_cast<int>(n);
    double const s = 1. / n;

    for (int i = 1; i <= in; ++i)
    {
        allocPoint();
        points.push_back(start + v * s * i);
    }

    if (in < n)
    {
        allocPoint();
        points.push_back(end);
    }
}

} // namespace

class DxfReader
{

    IRecordReader& mR;
    Entities mEntities;
    std::vector<Block> mBlocks;
    std::optional<double> mUnit2MeterFactor_;
    std::optional<Unit> mUnit;

    bool _AtEndSec()
    {
        return mR.Matches(0, "ENDSEC");
    }

    void _EndSection()
    {
        while (mR.Next() && !_AtEndSec())
            ;
    }

    void _ReadHeaderSection()
    {
        std::string name;
        while (mR.Next() && !_AtEndSec())
        {
            switch (mR.Gc())
            {
            case 9:
                name = mR.Str();
                break;
            case 70:
                if (name == "$INSUNITS")
                {
                    int unitIdx = mR.Int();
                    mUnit2MeterFactor_ = UnitIdx2MeterScale(unitIdx);
                    mUnit = {static_cast<Unit>(unitIdx)};
                }
                break;
            }
        }
    }

    struct EntityReader
    {
        IRecordReader& mR;
        Entities& ents;

        struct PolylineEntity
        {
            struct Vertex
            {
                Point2 pos;
                int flags = 0;
                double bulge = 0;
            };

            int flags = 0;
            std::vector<Vertex> verts;
        };

        PolyLine* pline = nullptr;
        PolylineEntity::Vertex* plineVert = nullptr;
        std::optional<PolylineEntity> plineInst;

        Vec3 extrusion;

        Line* line = nullptr;
        Circle* circle = nullptr;
        Arc* arc = nullptr;
        Ellipse* ellipse = nullptr;

        PolyLine* lwpline = nullptr;
        std::vector<double> lwplinex, lwpliney, lwplineBulge;

        Spline* spline = nullptr;
        std::vector<double> splinex, spliney;
        Insert* insert = nullptr;

        EntityReader(IRecordReader& r, Entities& ents)
            : mR(r)
            , ents(ents)
        {
        }

        void ReadRecord()
        {
            switch (mR.Gc())
            {
            case 0:
                line = nullptr;
                ellipse = nullptr;
                insert = nullptr;
                plineVert = nullptr;
                pline = nullptr;

                if (circle)
                {
                    circle->ocs = Ocs(extrusion);
                    circle = nullptr;
                }

                if (arc)
                {
                    arc->ocs = Ocs(extrusion);
                    arc = nullptr;
                }

                extrusion = Vec3{0, 0, 1};

                if (plineInst && mR.Matches(0, "SEQEND"))
                {
                    ents.mPolyLines.emplace_back();
                    PolylineEntity const& src = *plineInst;
                    PolyLine& dst = ents.mPolyLines.back();
                    dst.closed = src.flags;
                    for (PolylineEntity::Vertex const& v : src.verts)
                    {
                        int const omitBits =
                            1 | // Extra vertex created by curve-fitting
                            8 | // Spline vertex created by spline-fitting
                            16; // Spline frame control point

                        if (v.flags & omitBits)
                            continue;

                        dst.points.push_back(PlPoint{v.pos.x, v.pos.y, v.bulge});
                    }

                    plineInst.reset();
                }

                if (lwpline)
                {
                    size_t n = std::min(lwplinex.size(), std::min(lwpliney.size(), lwpline->points.size()));
                    assert(n == lwpline->points.size());
                    assert(lwplineBulge.size() <= n);
                    lwplineBulge.resize(n);

                    for (size_t i = 0; i < n; ++i)
                        lwpline->points[i] = PlPoint{lwplinex[i], lwpliney[i], lwplineBulge[i]};

                    lwplinex.clear();
                    lwpliney.clear();
                    lwplineBulge.clear();
                    lwpline = nullptr;
                }

                if (spline)
                {
                    size_t n = std::min(splinex.size(), std::min(spliney.size(), spline->ctlPoints.size()));

                    for (size_t i = 0; i < n; ++i)
                        spline->ctlPoints[i] = Point2{splinex[i], spliney[i]};

                    splinex.clear();
                    spliney.clear();
                    spline = nullptr;
                }
                break;
            case 102:
                // skip application specific sections
                while (mR.Next() && mR.Gc() != 102)
                    continue;
                break;
            }

            if (mR.Matches(0, "LINE"))
            {
                ents.mLines.emplace_back();
                line = &ents.mLines.back();
            }
            else if (mR.Matches(0, "LWPOLYLINE"))
            {
                ents.mPolyLines.emplace_back();
                lwpline = &ents.mPolyLines.back();
            }
            else if (mR.Matches(0, "POLYLINE"))
            {
                ents.mPolyLines.emplace_back();
                pline = &ents.mPolyLines.back();
                plineInst.emplace();
            }
            else if (mR.Matches(0, "VERTEX") && plineInst)
            {
                plineInst->verts.emplace_back();
                plineVert = &plineInst->verts.back();
            }
            else if (mR.Matches(0, "CIRCLE"))
            {
                ents.mCircles.emplace_back();
                circle = &ents.mCircles.back();
            }
            else if (mR.Matches(0, "ARC"))
            {
                ents.mArcs.emplace_back();
                arc = &ents.mArcs.back();
            }
            else if (mR.Matches(0, "ELLIPSE"))
            {
                ents.mEllipses.emplace_back();
                ellipse = &ents.mEllipses.back();
            }
            else if (mR.Matches(0, "SPLINE"))
            {
                ents.mSplines.emplace_back();
                spline = &ents.mSplines.back();
            }
            else if (mR.Matches(0, "INSERT"))
            {
                ents.mInserts.emplace_back();
                insert = &ents.mInserts.back();
            }
            else
            {
                if (line)
                {
                    switch (mR.Gc())
                    {
                    case 10:
                        line->start.x = mR.Dbl();
                        break;
                    case 20:
                        line->start.y = mR.Dbl();
                        break;
                    case 11:
                        line->end.x = mR.Dbl();
                        break;
                    case 21:
                        line->end.y = mR.Dbl();
                        break;
                    }
                }
                else if (lwpline)
                {
                    switch (mR.Gc())
                    {
                    case 10:
                        lwplinex.push_back(mR.Dbl());
                        break;
                    case 20:
                        lwpliney.push_back(mR.Dbl());
                        break;
                    case 42:
                    {
                        size_t n = std::max(lwplinex.size(), lwpliney.size());
                        if (lwplineBulge.size() >= n)
                            lwplineBulge.push_back(mR.Dbl());
                        else
                        {
                            lwplineBulge.resize(n);
                            lwplineBulge.back() = mR.Dbl();
                        }
                        break;
                    }
                    case 70:
                    {
                        int const flags = mR.Int();
                        if (flags & 1)
                            lwpline->closed = true;
                        break;
                    }
                    case 90:
                    {
                        int n = mR.Int();
                        lwpline->points.resize(n);
                        lwplinex.reserve(n);
                        lwpliney.reserve(n);
                        break;
                    }
                    }
                }
                else if (pline)
                {
                    switch (mR.Gc())
                    {
                    case 70:
                        pline->closed = (mR.Int() & 1);
                        break;
                    }
                }
                else if (plineVert)
                {
                    switch (mR.Gc())
                    {
                    case 10:
                        plineVert->pos.x = mR.Dbl();
                        break;
                    case 20:
                        plineVert->pos.y = mR.Dbl();
                        break;
                    case 42:
                        plineVert->bulge = mR.Dbl();
                        break;
                    case 70:
                        plineVert->flags = mR.Int();
                        break;
                    }
                }
                else if (circle)
                {
                    switch (mR.Gc())
                    {
                    case 10:
                        circle->center.x = mR.Dbl();
                        break;
                    case 20:
                        circle->center.y = mR.Dbl();
                        break;
                    case 40:
                        circle->radius = mR.Dbl();
                        break;
                    case 210:
                        extrusion.x = mR.Dbl();
                        break;
                    case 220:
                        extrusion.y = mR.Dbl();
                        break;
                    case 230:
                        extrusion.z = mR.Dbl();
                        break;
                    }
                }
                else if (arc)
                {
                    switch (mR.Gc())
                    {
                    case 10:
                        arc->center.x = mR.Dbl();
                        break;
                    case 20:
                        arc->center.y = mR.Dbl();
                        break;
                    case 40:
                        arc->radius = mR.Dbl();
                        break;
                    case 50:
                        arc->startAngle = mR.Dbl();
                        break;
                    case 51:
                        arc->endAngle = mR.Dbl();
                        break;
                    case 210:
                        extrusion.x = mR.Dbl();
                        break;
                    case 220:
                        extrusion.y = mR.Dbl();
                        break;
                    case 230:
                        extrusion.z = mR.Dbl();
                        break;
                    }
                }
                else if (ellipse)
                {
                    switch (mR.Gc())
                    {
                    case 10:
                        ellipse->center.x = mR.Dbl();
                        break;
                    case 20:
                        ellipse->center.y = mR.Dbl();
                        break;
                    case 11:
                        ellipse->majEnd.x = mR.Dbl();
                        break;
                    case 21:
                        ellipse->majEnd.y = mR.Dbl();
                        break;
                    case 40:
                        ellipse->ratio = mR.Dbl();
                        break;
                    case 41:
                        ellipse->start = mR.Dbl();
                        break;
                    case 42:
                        ellipse->end = mR.Dbl();
                        break;
                    }
                }
                else if (spline)
                {
                    switch (mR.Gc())
                    {
                    case 10:
                        splinex.push_back(mR.Dbl());
                        break;
                    case 20:
                        spliney.push_back(mR.Dbl());
                        break;
                    case 70:
                        spline->flags = mR.Int();
                        break;
                    case 71:
                        spline->degree = mR.Int();
                        break;
                    case 73:
                    {
                        int n = mR.Int();
                        spline->ctlPoints.resize(n);
                        splinex.reserve(n);
                        spliney.reserve(n);
                        break;
                    }
                    case 40:
                        spline->knotValues.push_back(mR.Dbl());
                        break;
                    }
                }
                else if (insert)
                {
                    switch (mR.Gc())
                    {
                    case 2:
                        insert->blockName = mR.Str();
                        break;
                    case 10:
                        insert->pos.x = mR.Dbl();
                        break;
                    case 20:
                        insert->pos.y = mR.Dbl();
                        break;
                    case 41:
                        insert->xScale = mR.Dbl();
                        break;
                    case 42:
                        insert->yScale = mR.Dbl();
                        break;
                    case 44:
                        insert->colSpacing = mR.Dbl();
                        break;
                    case 45:
                        insert->rowSpacing = mR.Dbl();
                        break;
                    case 50:
                        insert->rotation = mR.Dbl();
                        break;
                    case 70:
                        insert->cols = mR.Int();
                        break;
                    case 71:
                        insert->rows = mR.Int();
                        break;
                    }
                }
            }
        }
    };

    void _ReadEntitiesSection()
    {
        EntityReader r(mR, mEntities);
        while (mR.Next())
        {
            r.ReadRecord();

            if (_AtEndSec())
                break;
        }
    }

    void _ReadBlockSection()
    {
        std::optional<EntityReader> r;
        Block* block = nullptr;
        while (mR.Next())
        {
            if (block)
            {
                switch (mR.Gc())
                {
                case 0:
                    r.emplace(mR, block->ents);
                    block = nullptr;
                    break;
                case 1:
                    block->path = mR.Str();
                    break;
                case 2:
                case 3:
                    block->name = mR.Str();
                    break;
                case 4:
                    block->description = mR.Str();
                    break;
                case 10:
                    block->base.x = mR.Dbl();
                    break;
                case 20:
                    block->base.y = mR.Dbl();
                    break;
                case 70:
                    block->flags = mR.Int();
                    break;
                }
            }

            if (r)
                r->ReadRecord();

            if (mR.Matches(0, "BLOCK"))
            {
                r.reset();
                mBlocks.emplace_back();
                block = &mBlocks.back();
            }

            if (mR.Matches(0, "ENDBLK"))
            {
                r.reset();
                block = nullptr;
            }

            if (_AtEndSec())
                break;
        }
    }


public:
    DxfReader(IRecordReader& r)
        : mR(r)
    {
    }

    std::optional<Unit> GetUnit() const { return mUnit; }

    void Read()
    {
        try
        {
            while (mR.Next())
            {
                if (mR.Matches(0, "SECTION"))
                {
                    mR.Next();
                    if (mR.Matches(2, "HEADER"))
                        _ReadHeaderSection();
                    else if (mR.Matches(2, "BLOCKS"))
                        _ReadBlockSection();
                    else if (mR.Matches(2, "ENTITIES"))
                        _ReadEntitiesSection();
                    else
                        _EndSection();
                }
            }
        }
        catch (...)
        {
            std::ostringstream s;
            s << "Read error at " << mR.Position();
            std::throw_with_nested(DxfReadError(s.str()));
        }
    }

    std::vector<Path> CreatePaths(std::optional<Unit> unit, IPathConfigProvider& cfg, size_t maxPoints) const
    {
        double scaleToMeter;
        if (unit)
            scaleToMeter = UnitIdx2MeterScale(static_cast<int>(*unit));
        else
        {
            if (!mUnit2MeterFactor_)
                throw MissingUnitError("Unknown dimension for geometry, input file does not define the used physical unit and no explicit unit was set.");

            scaleToMeter = *mUnit2MeterFactor_;
        }

        double const scale = 1000.0 / scaleToMeter;

        std::map<std::string, Block const*> blocks;
        for (Block const& block : mBlocks)
            blocks[block.name] = &block;

        Transform2 t;
        t.x *= scale;
        t.y *= scale;
        std::vector<Path> ret;

        size_t usedPoints = 0;
        auto allocPoint = [&]()
        {
            if (usedPoints == maxPoints)
            {
                std::ostringstream s;
                s << "Imported file exceeds the maximum of " << maxPoints << " with the current configuration.";
                throw DxfReadError(s.str());
            }

            ++usedPoints;
        };

        _CreatePaths(t, ret, mEntities, blocks, cfg, allocPoint);

        for (size_t i = 0; i < ret.size(); ++i)
            ret[i].sourcePathIndices.insert(i);

        return ret;
    }

    void _CreatePaths(Transform2 const& trans, std::vector<Path>& ret, Entities const& ents, std::map<std::string, Block const*>& blocks, IPathConfigProvider& cfg, std::function<void()> allocPoint) const
    {
        double const eps = 0.0000001;

        auto maxDist = [&]
        {
            return cfg.GetMaxDist(ret.size());
        };

        auto maxError = [&]
        {
            return cfg.GetMaxError(ret.size());
        };

        for (Insert const& ins : ents.mInserts)
        {
            Block const* block;
            try
            {
                block = blocks.at(ins.blockName);
            }
            catch (...)
            {
                std::ostringstream s;
                s << "Insertion of non-existent block: " << ins.blockName;
                std::throw_with_nested(DxfReadError(s.str()));
            }

            double const a = Deg2Rad(ins.rotation);
            Vec2 const x{std::cos(a), std::sin(a)};
            Vec2 const y = Ortho(x);

            for (int col = 0; col < ins.cols; ++col)
            {
                for (int row = 0; row < ins.rows; ++row)
                {
                    Transform2 bt;
                    bt.x = ins.xScale * x;
                    bt.y = ins.yScale * y;
                    bt.base = ins.pos + x * col * ins.colSpacing + y * row * ins.rowSpacing;

                    _CreatePaths(trans * bt, ret, block->ents, blocks, cfg, allocPoint);
                }
            }
        }

        for (Line const& l : ents.mLines)
        {
            Path path;
            Point2 start = trans * l.start;
            allocPoint();
            path.points.push_back(start);
            SampleLine(path.points, maxDist(), start, trans * l.end, allocPoint);
            ret.push_back(std::move(path));
        }

        for (PolyLine const& pl : ents.mPolyLines)
        {
            Path path;
            std::vector<Point2>& points = path.points;
            if (pl.points.empty())
                continue;

            PlPoint const* prev = &pl.points.front();
            auto emit = [&](PlPoint const& next)
            {
                if (!prev->bulge)
                    SampleLine(points, maxDist(), Point2(prev->x, prev->y), trans * next, allocPoint);
                else
                {
                    path.curved = true;
                    Point2 mid = Midpoint(next, *prev);
                    Point2 tip = mid + Ortho(mid - next) * prev->bulge;
                    double cc = SqLength(tip - next);
                    double a = Distance(mid, tip);
                    double r = .5 * cc / a;

                    double beta = M_PI - std::acos(a / std::sqrt(cc)) * 2;

                    Vec2 y = Normalized(mid - tip);
                    Vec2 x = Ortho(y);

                    Circle circle;
                    circle.radius = r;
                    Point2 const center = tip + y * r;

                    auto stepper = AdaptiveStepper(maxError(), maxDist(), eps, M_PI / 2, 0, [&](double _u)
                                                   {
						double u = (prev->bulge > 0) ? beta - _u : _u - beta;
					Point2 b = circle.Plot(u - M_PI * .5);
                    return trans * (center + x * b.x + y * b.y); });
                    stepper.Next();

                    while (stepper.CurPos() < 2 * beta)
                    {
                        allocPoint();
                        points.push_back(stepper.CurPoint());
                        stepper.Next();
                    }

                    allocPoint();
                    points.push_back(trans * next);
                }

                prev = &next;
            };

            allocPoint();
            points.push_back(trans * *prev);
            for (size_t i = 1; i < pl.points.size(); ++i)
                emit(pl.points[i]);

            if (pl.closed)
            {
                emit(pl.points[0]);
                path.cyclic = true;
                points.pop_back();
            }

            ret.push_back(std::move(path));
        }

        for (Circle const& c : ents.mCircles)
        {
            CircularInfo ci;
            ci.desc.center = trans * c.center;
            ci.desc.major = trans * Vec2{c.radius, 0};
            ci.desc.minor = trans * Vec2{0, c.radius};
            ci.startAngle = cfg.GetCircleStartAngle(ret.size(), ci.desc);

            auto stepper = AdaptiveStepper(maxError(), maxDist(), eps, M_PI / 2, 0, [&](double u)
                                           { return trans * c.Plot(u + Deg2Rad(ci.startAngle)); });
            Path path;
            while (stepper.CurPos() < 2 * M_PI)
            {
                allocPoint();
                path.points.push_back(stepper.CurPoint());
                stepper.Next();
            }

            path.cyclic = true;
            path.circular = ci;
            path.isCircle = true;
            path.optimizeStart = true;
            path.directed = false;
            path.curved = true;

            ret.push_back(std::move(path));
        }

        for (Arc const& a : ents.mArcs)
        {
            double start = std::fmod(a.startAngle, 360.);
            double end = std::fmod(a.endAngle, 360.);
            if (end <= start)
                end += 360.;

            start = Deg2Rad(start);
            end = Deg2Rad(end);

            auto stepper = AdaptiveStepper(maxError(), maxDist(), eps, M_PI / 2, start, [&](double u)
                                           { return trans * a.Plot(u); });

            Path path;
            path.curved = true;
            while (stepper.CurPos() < end)
            {
                allocPoint();
                path.points.push_back(stepper.CurPoint());
                stepper.Next();
            }

            allocPoint();
            path.points.push_back(trans * a.Plot(end));
            ret.push_back(std::move(path));
        }

        for (Ellipse const& e : ents.mEllipses)
        {
            double start = e.start;
            double end = e.end;
            while (end < start)
                end += 2 * M_PI;

            double const maxErr = maxError();
            auto stepper = AdaptiveStepper(maxErr, maxDist(), eps, M_PI / 2, start, [&](double u)
                                           { return trans * e.Plot(u); });

            Path path;
            path.curved = true;
            while (stepper.CurPos() < end)
            {
                allocPoint();
                path.points.push_back(stepper.CurPoint());
                stepper.Next();
            }

            Point2 endpoint = trans * e.Plot(e.end);
            if (!path.points.empty() && Distance(path.StartPoint(), endpoint) <= maxErr)
            {
                path.cyclic = true;

                CircularInfo ci;
                ci.desc.center = trans * e.center;
                ci.desc.major = trans * e.majEnd;
                ci.desc.minor = trans * (Ortho(e.majEnd) * e.ratio);
                path.circular = ci;

                path.optimizeStart = true;
                path.directed = false;
            }
            else
            {
                allocPoint();
                path.points.push_back(endpoint);
            }

            ret.push_back(std::move(path));
        }

        for (Spline const& s : ents.mSplines)
        {
            if (s.ctlPoints.empty())
                continue;

            tinyspline::BSpline bs(s.ctlPoints.size(), 2, s.degree); //, BSpline::Opened);
            for (size_t i = 0; i < s.ctlPoints.size(); ++i)
            {
                Point2 p = trans * s.ctlPoints[i];
                tinyspline::Vec2 v(p.x, p.y);
                bs.setControlPointVec2At(i, v);
            }

            bs.setKnots(s.knotValues);

            Point2 const endPoint = trans * s.ctlPoints.back();

            {
                AdaptiveStepper stepper = AdaptiveStepper(maxError(), maxDist(), eps, 1, 0, [&](double u)
                                                          {
                        try
                        {
                            tinyspline::Vec2 p = bs.eval(u).resultVec2();
                            return Point2{
                                p.x(),
                                p.y()};
                        }
                        catch (...)
                        {
                        return endPoint;
                        } });

                Path path;
                path.curved = true;

                while (stepper.CurPos() < bs.domain().max())
                {
                    allocPoint();
                    path.points.push_back(stepper.CurPoint());
                    stepper.Next();
                }

                if (s.flags & Spline::Closed)
                    path.cyclic = true;
                else
                {
                    allocPoint();
                    path.points.push_back(endPoint);
                }

                ret.push_back(std::move(path));
            }
        }
    }
};

namespace
{

std::pair<char const*, Unit> constexpr str2unit[]{
    {"inches", Unit::Inches},
    {"in", Unit::Inches},
    {"feet", Unit::Feet},
    {"ft", Unit::Feet},
    {"miles", Unit::Miles},
    {"mi", Unit::Miles},
    {"millimeters", Unit::Millimeters},
    {"mm", Unit::Millimeters},
    {"centimeters", Unit::Centimeters},
    {"cm", Unit::Centimeters},
    {"meters", Unit::Meters},
    {"m", Unit::Meters},
    {"kilometers", Unit::Kilometers},
    {"km", Unit::Kilometers},
    {"yards", Unit::Yards},
    {"yd", Unit::Yards},
    {"angstroms", Unit::Angstroms},
    {"nanometers", Unit::Nanometers},
    {"nm", Unit::Nanometers},
    {"micrometers", Unit::Microns},
    {"microns", Unit::Microns},
    {"ym", Unit::Microns},
    {"decimeters", Unit::Decimeters},
    {"dm", Unit::Decimeters},
    {"decameters", Unit::Decameters},
    {"dam", Unit::Decameters},
    {"hectometers", Unit::Hectometers},
    {"hm", Unit::Hectometers},
    {"gigameters", Unit::Gigameters},
    {"gm", Unit::Gigameters},
    {"astronomicalunits", Unit::AstronomicalUnits},
    {"au", Unit::AstronomicalUnits},
    {"lightyears", Unit::LightYears},
    {"ly", Unit::LightYears},
    {"parsecs", Unit::Parsecs},
    {"pc", Unit::Parsecs},
};

std::shared_ptr<DxfReader> _Read(std::istream& dxf)
{
    AsciiRecordReader arr{dxf};
    auto r = std::make_shared<DxfReader>(arr);
    r->Read();
    return r;
}

} // namespace

Unit String2Unit(std::string s)
{
    for (auto [k, v] : str2unit)
    {
        if (s == k)
            return v;
    }

    std::ostringstream e;
    e << "Unknown unit: " << s;
    throw std::invalid_argument(e.str());
}

std::vector<std::pair<char const*, Unit>> KnownUnits()
{
    return {std::begin(str2unit), std::end(str2unit)};
}

DxfData::DxfData(std::istream& dxf)
{
    mReader = _Read(dxf);
}

DxfData::DxfData(std::string dxfPath)
{
    std::ifstream dxf(dxfPath);

    if (!dxf)
    {
        std::ostringstream s;
        s << "Failed to open input file: " << dxfPath;
        throw DxfReadError(s.str());
    }

    mReader = _Read(dxf);
}

std::optional<Unit> DxfData::GetUnit() const
{
    if (!mReader)
        return {};

    return mReader->GetUnit();
}

std::vector<Path> DxfData::CreatePaths(std::optional<Unit> unit, IPathConfigProvider& cfg, size_t maxPoints) const
{
    return mReader->CreatePaths(unit, cfg, maxPoints);
}

} // nsDxfReader
