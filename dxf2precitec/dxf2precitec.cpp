#include "dxfreader.h"
#include <fstream>
#include <array>
#include <iostream>
#include <tuple>
#include <sstream>
#include <optional>
#include <list>

/*
TODO:
- removal of (nearly) duplicate points
- finish and cleanup output (svg and json)
- HATCH?
*/

using namespace nsDxfReader;

// ----------------------------------------------------------------------------

/**
Returns the sum of "spare" movement that is required to trace the paths in the given order and returning to the start of the first path.
*/
double Overhead(std::vector<Path> const& paths)
{
    if (paths.empty())
        return 0;

    double overhead = 0;
    Path const* prev = &paths.back();
    for (Path const& path : paths)
    {
        overhead += Distance(prev->EndPoint(), path.StartPoint());
        prev = &path;
    }

    return overhead;
}

// ----------------------------------------------------------------------------

// see https://en.cppreference.com/w/cpp/error/throw_with_nested
void print_exception(const std::exception& e, int level = 0)
{
    std::cerr << std::string(level, ' ') << "exception: " << e.what() << '\n';
    try
    {
        std::rethrow_if_nested(e);
    }
    catch (const std::exception& nestedException)
    {
        print_exception(nestedException, level + 1);
    }
    catch (...)
    {
    }
}

// quotes and escapes a string value for json
std::string jstr(std::string str)
{
    std::ostringstream s;
    s << "\"";
    for (char c : str)
    {
        switch (c)
        {
        case '"':
            s << "\\\"";
            break;
        case '\\':
            s << "\\\\";
            break;
        default:
            s << c;
        }
    }
    s << "\"";

    return s.str();
}

int main(int /*argc*/, char const** argv)
{
    char const* usage = R"(usage: <dxfpath> <jsonpath> [-a <name> <value>] ..

dxfpath        Path to the .dxf input file
jsonpath       Path to the .json precitec output file
-a name value  Adds a json attribute with the given name and value to the top
               of the json file. This option is optional and can be used
               multiple times.
-e maxError    Maximum error for curved elements. Points along line segments
               that approximate a curved elements will not be further away
               from their ideal path than this value. Unit: mm, default: 0.5
-d maxDist     Maximum distance between successive points on curved elements.
-optdir        Allow the optimizer to reverse direction of non-cyclic paths.
-optstart      Allow the optimizer to choose any vertex of a cyclic path as
               starting point. The starting position for circles and ellipses
               is always chosen by the optimizer.
-unit name     Sets or overrides the physical unit of the imported geometry.
               By default the unit is taken from the DXF file and must be set.
               See -lu for supported units.
-lu            List known units and exit.
-svg           Path to a .svg output file. Intended for testing and debugging.
-v             Print version and exit
--             Terminates options. All following arguments are interpreted as
               positional, even if they start with a dash.
)";

    std::string dxfPath;
    std::string jsonPath;
    std::optional<std::string> unitArg;
    std::optional<std::string> svgPath;
    std::list<std::tuple<std::string, std::string>> attrs;
    double maxError = 0.5;
    std::optional<double> maxDist;
    bool optdir = false;
    bool optstart = false;

    { // commandline parsing
        std::array<std::string*, 2> posArgs = {&dxfPath, &jsonPath};
        auto posIt = posArgs.begin();
        bool optionsDone = false;

        for (char const** parg = argv + 1; *parg; ++parg)
        {
            std::string const arg = *parg;
            if (!optionsDone)
            {
                if (arg == "-v")
                {
                    std::cout << "Preview version 3" << std::endl;
                    return 0;
                }

                if (arg == "-a")
                {
                    if (parg[1] && parg[2])
                        attrs.emplace_back(parg[1], parg[2]);
                    else
                    {
                        std::cerr << "invalid usage of -a, expecting name and value" << std::endl;
                        return -1;
                    }

                    parg += 2;
                    continue;
                }

                if (arg == "-h" || arg == "--help")
                {
                    std::cout << usage << std::endl;
                    return 0;
                }

                if (arg == "-e")
                {
                    ++parg;
                    if (!*parg)
                    {
                        std::cerr << "invalid usage of -e, expecting value afterwards" << std::endl;
                        return -1;
                    }

                    std::istringstream s(*parg);
                    s.imbue(std::locale("C"));
                    s >> maxError;
                    if (!s)
                    {
                        std::cerr << "could not parse value for -e" << std::endl;
                        return -1;
                    }

                    if (maxError <= 0)
                    {
                        std::cerr << "value for -e must be positive" << std::endl;
                        return -1;
                    }

                    continue;
                }

                if (arg == "-d")
                {
                    ++parg;
                    if (!*parg)
                    {
                        std::cerr << "invalid usage of -d, expecting value afterwards" << std::endl;
                        return -1;
                    }

                    std::istringstream s(*parg);
                    s.imbue(std::locale("C"));
                    s >> maxError;
                    if (!s)
                    {
                        std::cerr << "could not parse value for -d" << std::endl;
                        return -1;
                    }

                    if (maxError <= 0)
                    {
                        std::cerr << "value for -d must be positive" << std::endl;
                        return -1;
                    }

                    continue;
                }

                if (arg == "-unit")
                {
                    ++parg;
                    if (!*parg)
                    {
                        std::cerr << "invalid usage of -unit, expecting value afterwards" << std::endl;
                        return -1;
                    }

                    unitArg = *parg;
                    continue;
                }

                if (arg == "-lu")
                {
                    std::cout << "Known units for option -unit:\n  ";
                    Unit const* prev = nullptr;
                    for (auto const& [s, u] : KnownUnits())
                    {
                        if (prev)
                        {
                            if (*prev != u)
                                std::cout << "\n  ";
                            else
                                std::cout << ", ";
                        }

                        prev = &u;
                        std::cout << s;
                    }
                    std::cout << std::endl;
                    return 0;
                }

                if (arg == "-optdir")
                {
                    optdir = true;
                    continue;
                }

                if (arg == "-optstart")
                {
                    optstart = true;
                    continue;
                }

                if (arg == "-svg")
                {
                    ++parg;
                    if (!*parg)
                    {
                        std::cerr << "invalid usage of -svg, expecting path afterwards" << std::endl;
                        return -1;
                    }

                    svgPath = *parg;
                    continue;
                }

                if (arg == "--")
                {
                    optionsDone = true;
                    continue;
                }

                if (!arg.empty() && arg.front() == '-')
                {
                    std::cerr << "invalid option: " << arg << std::endl;
                    return -1;
                }
            }

            if (posIt == posArgs.end())
            {
                std::cerr << "invalid commandline" << std::endl;
                return -1;
            }

            **posIt = arg;
            ++posIt;
        }

        if (posIt != posArgs.end())
        {
            std::cerr << "missing positional argument" << std::endl;
            return -1;
        }
    }

    try
    {
        std::optional<Unit> unit;
        if (unitArg)
            unit = String2Unit(*unitArg);

        class PathConfigProvider : public IPathConfigProvider
        {
            std::optional<double> const mMaxDist;
            double const mMaxError;

        public:
            PathConfigProvider(std::optional<double> maxDist, double maxError)
                : mMaxDist(maxDist)
                , mMaxError(maxError)
            {
            }

            double GetCircleStartAngle(size_t /*pathIdx*/, CircularDesc const& /*circleDesc*/) override { return 0; }
            std::optional<double> GetMaxDist(size_t /*pathIdx*/) override { return mMaxDist; }
            double GetMaxError(size_t /*pathIdx*/) override { return mMaxError; }
        };

        DxfData data(dxfPath);
        auto cfg = PathConfigProvider{maxDist, maxError};
        std::vector<Path> paths = data.CreatePaths(unit, cfg, 100 * 1000);

        std::ofstream json(jsonPath);

        if (!json)
        {
            std::ostringstream s;
            s << "Failed to open output file: " << jsonPath;
            throw DxfReadError(s.str());
        }

        json.exceptions(json.badbit | json.failbit);
        json.imbue(std::locale("C"));

        std::string const ind = "    ";

        json << "{\n";
        for (auto const& attr : attrs)
        {
            auto [name, val] = attr;
            json << ind << jstr(name) << " : " << jstr(val) << ",\n";
        }

        json << ind << "\"Figure\" :\n";
        json << ind << "[\n";

        paths = JoinPaths(paths, maxError);
        paths = RouteOptimized(paths);

        for (Path& path : paths)
            path.optimizeStart = optdir || (path.cyclic && optstart);

        double overhead = Overhead(paths);

        for (int iters = 0; iters < 100; ++iters)
        {
            std::vector<Path> opt = optstart ? ImproveStartPositions(paths) : paths;
            ImproveOrderLocally(opt);
            ImproveDirsLocally(opt);
            double x = Overhead(opt);
            if (x >= overhead)
                break;
            swap(opt, paths);
            overhead = x;
        }

        for (Path const& path : paths)
        {
            for (Point2 const& p : path.points)
            {
                bool const isLast = (&path == &paths.back()) && (&p == &path.points.back());
                char const* sep = isLast ? "" : ",";

                json << ind << ind << "{\n";
                json << ind << ind << ind << "\"EndPosition\" : [" << p.x << ", " << p.y << "],\n";
                json << ind << ind << ind << "\"Power\" : -1,\n";
                json << ind << ind << ind << "\"RingPower\" : -1,\n";
                json << ind << ind << ind << "\"Velocity\" : -1\n";
                json << ind << ind << "}" << sep << "\n";
            }
        }
        json << ind << "]\n";
        json << "}\n";

        if (svgPath)
        {
            std::ofstream svg(*svgPath);

            if (!svg)
            {
                std::ostringstream s;
                s << "Failed to open SVG output file: " << *svgPath;
                throw DxfReadError(s.str());
            }

            svg.exceptions(svg.badbit | svg.failbit);
            svg.imbue(std::locale("C"));

            // NOTE: SVG uses a negative Y-axis (relative to DXF)
            Point2 min, max;

            bool first = true;
            for (Path const& path : paths)
            {
                for (Point2 const& p : path.points)
                {
                    if (first)
                    {
                        min = max = Point2{p.x, -p.y};
                        first = false;
                    }
                    else
                    {
                        min.x = std::min(min.x, p.x);
                        min.y = std::min(min.y, -p.y);
                        max.x = std::max(max.x, p.x);
                        max.y = std::max(max.y, -p.y);
                    }
                }
            }

            double const width = max.x - min.x;
            double const height = max.y - min.y;
            //double const scale = r.ScaleToMeter().value_or(1000.) / 1000.;
            double const scale = 100. / std::max(width, height);

            svg << "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\n";
            svg << "<svg\n";
            svg << "  version=\"1.1\"\n";
            svg << "  width=\"" << width * scale << "mm\" height=\"" << height * scale << "mm\"\n";
            svg << "  viewBox=\"" << min.x << " " << min.y << " " << width << " " << height << "\"";
            svg << R"x(
  xmlns:inkscape="http://www.inkscape.org/namespaces/inkscape"
  xmlns:sodipodi="http://sodipodi.sourceforge.net/DTD/sodipodi-0.dtd"
  xmlns="http://www.w3.org/2000/svg"
  xmlns:svg="http://www.w3.org/2000/svg">
  <defs
     id="defs98666">
    <marker
       style="overflow:visible"
       id="Arrow1"
       refX="0"
       refY="0"
       orient="auto-start-reverse"
       inkscape:stockid="Arrow1"
       markerWidth="4.0606604"
       markerHeight="6.7071066"
       viewBox="0 0 4.0606602 6.7071068"
       inkscape:isstock="true"
       inkscape:collect="always"
       preserveAspectRatio="xMidYMid">
      <path
         style="fill:none;stroke:context-stroke;stroke-width:1;stroke-linecap:butt"
         d="M 3,-3 0,0 3,3"
         id="path5057"
         transform="rotate(180,0.125,0)"
         sodipodi:nodetypes="ccc" />
    </marker>
    <marker
       style="overflow:visible"
       id="TriangleStart"
       refX="0"
       refY="0"
       orient="auto-start-reverse"
       inkscape:stockid="TriangleStart"
       markerWidth="5.3244081"
       markerHeight="6.155385"
       viewBox="0 0 5.3244081 6.1553851"
       inkscape:isstock="true"
       inkscape:collect="always"
       preserveAspectRatio="xMidYMid">
      <path
         transform="scale(0.5)"
         style="fill:context-stroke;fill-rule:evenodd;stroke:context-stroke;stroke-width:1pt"
         d="M 5.77,0 -2.88,5 V -5 Z"
         id="path135" />
    </marker>
    <marker
       style="overflow:visible"
       id="Stop"
       refX="0"
       refY="0"
       orient="auto"
       inkscape:stockid="Stop"
       markerWidth="1"
       markerHeight="8"
       viewBox="0 0 1 8"
       inkscape:isstock="true"
       inkscape:collect="always"
       preserveAspectRatio="xMidYMid">
      <path
         style="fill:none;stroke:context-stroke;stroke-width:1"
         d="M 0,4 V -4"
         id="path171" />
    </marker>
  </defs>
)x";

            for (Path const& path : paths)
            {
                Point2 const& p0 = path.points.front();
                svg << "<polyline fill=\"none\" stroke=\"black\"\n";
                svg << "style=\"opacity:0.1;stroke-width:" << 0.3 / scale << ";marker-start:url(#Stop);marker-mid:url(#Arrow1);marker-end:url(#TriangleStart)\"\n";
                svg << "points=\"" << p0.x << "," << -p0.y;
                for (auto it = path.points.begin() + 1; it != path.points.end(); ++it)
                {
                    Point2 const& p = *it;
                    svg << "\n"
                        << p.x << "," << -p.y;
                }

                if (path.cyclic)
                {
                    Point2 const& p = path.points.front();
                    svg << "\n"
                        << p.x << "," << -p.y;
                }

                svg << "\"/>\n";
            }

            if (!paths.empty())
            {
                Point2 const* prev = &paths.back().EndPoint();
                for (Path const& path : paths)
                {
                    Point2 const& p0 = path.points.front();
                    if (prev && prev != &p0)
                    {
                        svg << "<polyline fill=\"none\" stroke=\"green\"\n";
                        svg << "style=\"stroke-width:" << 0.1 / scale << ";marker-start:url(#Stop);marker-mid:url(#Arrow1);marker-end:url(#TriangleStart)\"\n";
                        svg << "points=\"" << prev->x << "," << -prev->y << " " << p0.x << "," << -p0.y << "\"/>\n";
                    }

                    //svg << "<polyline fill=\"none\" stroke=\"red\"\n";
                    //svg << "style=\"stroke-width:" << 0.1 / scale << ";marker-start:url(#Stop);marker-mid:url(#Arrow1);marker-end:url(#TriangleStart)\"\n";
                    //svg << "points=\"" << p0.x << "," << -p0.y;
                    //for (auto it = path.points.begin() + 1; it != path.points.end(); ++it)
                    //{
                    //	Point const& p = *it;
                    //	svg << "\n" << p.x << "," << -p.y;
                    //}

                    if (path.cyclic)
                    {
                        Point2 const& p = path.StartPoint();
                        //svg << "\n" << p.x << "," << -p.y;
                        prev = &p;
                    }
                    else
                        prev = &path.EndPoint();
                    //svg << "\"/>\n";
                }
            }

            svg << "</svg>\n";
        }
    }
    catch (std::exception& e)
    {
        print_exception(e);
        return -1;
    }

    return 0;
}
