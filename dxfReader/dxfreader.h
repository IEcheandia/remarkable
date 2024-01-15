#pragma once

#include "linalg.h"
#include <vector>
#include <set>
#include <iosfwd>
#include <exception>
#include <string>
#include <cassert>
#include <optional>
#include <memory>
#include <functional>

namespace nsDxfReader
{

class DxfReader;

/**
Describes an ellipse or a circle.
*/
struct CircularDesc
{
    Point2 center;
    Vec2 minor, major;
};

/**
Decides if two circular elements can be considered identical for a given tolerance.
*/
bool equals(CircularDesc const& a, CircularDesc const& b, double maxError);

struct CircularInfo
{
    CircularDesc desc;
    double startAngle = 0;
};

struct Path
{
    bool cyclic = false;
    std::optional<CircularInfo> circular; ///< set only for circles and closed ellipses
    bool isCircle = false;
    bool optimizeStart = false; ///< allows the routing algorithm to pick the start vertex (if cyclic any vertex can be start and end, otherwise the direction can be changed)
    bool directed = true;       ///< the direction is meaningful (which is not the case for circles and full ellipses)
    bool curved = false;        ///< this path uses the accuracy to approximate a curved element
    std::vector<Point2> points;
    std::set<size_t> sourcePathIndices; ///< Inidices of the paths from DxfData::CreatePaths to construct this path

    Point2 const& EndPoint() const
    {
        assert(!points.empty());
        return cyclic ? points.front() : points.back();
    }

    Point2 const& StartPoint() const
    {
        assert(!points.empty());
        return points.front();
    }
};

class DxfReadError : public std::exception
{
    std::string const mMsg;
public:
    DxfReadError(std::string msg)
        : mMsg(msg)
    {
    }

    virtual char const* what() const noexcept override
    {
        return mMsg.c_str();
    }
};

class MissingUnitError : public DxfReadError
{
public:
    MissingUnitError(std::string msg)
        : DxfReadError(msg)
    {
    }
};

enum class Unit
{ // NOTE: Numerical values must agree with DXF specification!
    Inches = 1,
    Feet = 2,
    Miles = 3,
    Millimeters = 4,
    Centimeters = 5,
    Meters = 6,
    Kilometers = 7,
    Yards = 10,
    Angstroms = 11,
    Nanometers = 12,
    Microns = 13,
    Decimeters = 14,
    Decameters = 15,
    Hectometers = 16,
    Gigameters = 17,
    AstronomicalUnits = 18,
    LightYears = 19,
    Parsecs = 20,
};

Unit String2Unit(std::string s);
std::vector<std::pair<char const*, Unit>> KnownUnits();

/**
Used to query configuration for the imported items.
*/
class IPathConfigProvider
{
public:
    virtual ~IPathConfigProvider() {}
    virtual double GetCircleStartAngle(size_t pathIdx, CircularDesc const& circleDesc) = 0;
    virtual std::optional<double> GetMaxDist(size_t pathIdx) = 0;
    virtual double GetMaxError(size_t pathIdx) = 0;
};

class DxfData
{
    std::shared_ptr<DxfReader> mReader;
public:
    DxfData() = default;
    DxfData(std::istream& dxf);
    DxfData(std::string dxfPath);

    void clear() { mReader.reset(); }

    operator bool() const { return !!mReader; }

    std::optional<Unit> GetUnit() const;
    std::vector<Path> CreatePaths(std::optional<Unit> unit, IPathConfigProvider& cfg, size_t maxPoints) const;
};

std::vector<Path> JoinPaths(std::vector<Path> const& paths, double maxError);
void ImproveDirsLocally(std::vector<Path>& paths);
std::vector<Path> RouteOptimized(std::vector<Path> const& paths);
bool ImproveOrderLocally(std::vector<Path>& paths);
std::vector<Path> ImproveStartPositions(std::vector<Path> const& paths);

}
