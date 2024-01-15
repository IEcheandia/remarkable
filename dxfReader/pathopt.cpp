#include "dxfreader.h"

#include "frnn.h"
#include <algorithm>
#include <array>

#ifndef NDEBUG
#define _DEBUG
#endif

namespace nsDxfReader
{

/*
Randbedingungen:

Reihenfolge der Pfade soll möglichst erhalten bleiben. Pfade die nicht zusammengeführt wurden
sollen ihre Ordnung behalten. Zusammengeführte orientieren sich am Start-Pfad.

Ablauf:

1. Nachbarpunkte finden:

- Alle Start- und Endpunkte T von jedem Pfad P registerieren
- Für jeden T nach einem Partner im Radius suchen.
  - Überspringen falls T als behandelt markiert ist
  - Kandidaten ignorieren wenn er zum gleichen Pfad gehört wie T
  - Kandidaten ignorieren wenn sie als "behandelt" markiert sind
  - alle Kandidaten als "behandelt" markieren
  - Wenn es mehr als einen Kandidaten gibt dann als Fehler protokollieren (Position als Info)
  - Bei Erfolg:
    - Nachbar-Zeiger der beiden Punkte setzen

2. Pfade markieren ob sie in einer Zusammenführung landen

- Pfade der Reihe nach durchgehen
  - Falls bereits Teil einer Kette: überspringen
  - Falls kein Ende einen Nachbarn hat: Überspringen weil nicht Teil einer Kette
  - Falls beide Enden einen Nachbarn haben: Überspringen weil nicht am Start/Ende einer Kette
  - alle benachbarten Pfadelemente als "Teil einer Kette" markieren
  - merken welcher Pfad das Start-Segment sein soll

3. Ausgabe erzeugen

- Pfade der Reihe nach durchgehen
  - Falls bereits in Ausgabe: Überspringen
  - Falls nicht Teil einer Kette: In Ausgabe kopieren
  - Falls Start-Segment einer Kette: Kette zu einem Pfad machen und in Ausgabe kopieren
*/
std::vector<Path> JoinPaths(std::vector<Path> const& paths, double maxError)
{
    struct PathInfo;

    // Auxillary information about the start or end of a path
    struct PointInfo
    {
        Point2 p;
        PathInfo* pathInfo;
        bool atStart;
        bool pairingDone = false;
        PointInfo* neighbor = nullptr;
    };

    // Auxillary information about a path
    struct PathInfo
    {
        PointInfo start, end;
        Path const* path;
        bool inChain = false;
        bool isChainStart = false;
        bool inOutput = false;
        bool cyclic = false;
    };

    // Applies a function for every path in a chain (PathInfo::start/end.neighbor). Also provides a flag if the path is used forward (otherwise its points must visited in reverse order).
    // f(PathInfo & pathInfo, bool usedForward)
    auto IterateChain = [](PathInfo& first, auto f) -> void
    {
        assert(!first.start.neighbor != !first.end.neighbor); // this function should only be used with the first/last path of a chain
        f(first, !first.start.neighbor);

        for (PointInfo* it = first.start.neighbor ? first.start.neighbor : first.end.neighbor; it;
             it = it->atStart ? it->pathInfo->end.neighbor : it->pathInfo->start.neighbor)
            f(*it->pathInfo, it->atStart);
    };

    // initialize auxillary per-path data structures
    std::vector<PathInfo> pis(paths.size());
    for (size_t i = 0; i < paths.size(); ++i)
    {
        Path const& p = paths[i];
        PathInfo& pi = pis[i];

        pi.path = &p;

        pi.start.p = p.StartPoint();
        pi.start.pathInfo = &pi;
        pi.start.atStart = true;

        pi.end.p = p.EndPoint();
        pi.end.pathInfo = &pi;
        pi.end.atStart = false;
    }

    // TODO: actually report them in a sensible way
    std::vector<Point2> ambiguousPositions;
    std::vector<Point2> ambiguousDirections;

    { // find neighbors
        // build helper structure to efficiently find points in neighbor range
        Frnn2D<double, PointInfo*> frnn(maxError);
        for (PathInfo& pi : pis)
        {
            if (pi.path->cyclic)
                continue;

            for (PointInfo* i : {&pi.start, &pi.end})
                frnn.Insert(i->p.x, i->p.y, i);
        }

        // pair the neighbors
        for (PathInfo& pi : pis)
        {
            if (pi.path->cyclic)
                continue;

            for (PointInfo* cur : {&pi.start, &pi.end})
            {
                bool posReported = false;
                frnn.QueryCandidates(cur->p.x, cur->p.y, [&](PointInfo* cand)
                                     {
                if (cand->pathInfo == &pi)
                    return; // skip if the other point belongs to the same path

                if (Distance(cur->p, cand->p) > maxError)
                    return; // points are not close enough

                if (cand->pairingDone)
                    return; // skip if this point was already handled

                cand->pairingDone = true;
                if (cur->pairingDone)
                {
                    if (!posReported)
                    {
                        ambiguousPositions.push_back(cur->p);
                        posReported = true;
                    }
                    return;
                }

                cur->pairingDone = true;

                assert(!cur->neighbor && !cand->neighbor);
                cur->neighbor = cand;
                cand->neighbor = cur; });
            }
        }
    }

#ifdef _DEBUG
    for (PathInfo const& pi : pis)
    {
        for (PointInfo const* p : {&pi.start, &pi.end})
        {
            assert(p->pathInfo == &pi);
            if (!p->neighbor)
                continue;

            assert(p->neighbor->neighbor == p);
            assert(Distance(p->p, p->neighbor->p) <= maxError);
        }
    }
#endif

    // mark paths as parts of chain
    for (PathInfo& pi : pis)
    {
        assert(pi.start.atStart);
        assert(!pi.end.atStart);

        if (pi.inChain)
            continue;

        if (pi.start.neighbor && pi.end.neighbor)
            continue; // the path is not the start or end of a chain

        if (!pi.start.neighbor && !pi.end.neighbor)
            continue; // the path is not connected to other paths

        PathInfo* last;
        IterateChain(pi, [&](PathInfo& it, bool)
                     {
            assert(!it.inChain);
            it.inChain = true;
            last = &it; });

        PathInfo* begin = &pi;
        PathInfo* end = last;

        if (begin->start.neighbor)
            std::swap(begin, end);

        begin->isChainStart = true;

        if (end->end.neighbor)
            ambiguousDirections.push_back(end->end.p);
    }

    // handle cyclic chains by using the first path as start
    // TODO: this approach is quite arbitrary, find a good solution!
    for (PathInfo& pi : pis)
    {
        if (pi.inChain || !pi.start.neighbor)
            continue;

        // break the link from the last to the first path
        pi.start.neighbor->neighbor = nullptr;
        pi.start.neighbor = nullptr;
        pi.isChainStart = true;

        IterateChain(pi, [&](PathInfo& it, bool)
                     {
            assert(!it.inChain);
            it.inChain = true;
            it.cyclic = true; });
    }

    // move paths to output (as chain if they are part of one)
    std::vector<Path> ret;
    for (PathInfo& pi : pis)
    {
        if (pi.inOutput)
            continue;

        if (!pi.inChain)
        {
            ret.push_back(*pi.path);
            continue;
        }

        assert(pi.inChain);
        if (!pi.isChainStart)
            continue;

#ifdef _DEBUG
        {
            PointInfo* prevEnd = nullptr;
            IterateChain(pi, [&](PathInfo& it, bool forward)
                         {
                    PointInfo* curStart = forward ? &it.start : &it.end;
            if (prevEnd)
            {
                double const d = Distance(prevEnd->p, curStart->p);
                assert(d <= maxError);
            }
            prevEnd = forward ? &it.end : &it.start; });
        }
#endif

        size_t n = 1; // NOTE: we skip the first point for all but the first path of the chain
        IterateChain(pi, [&](PathInfo& it, bool)
                     {
            n += it.path->points.size() - 1; /* NOTE: dont count the first point*/ });

        ret.emplace_back();
        Path& path = ret.back();
        path.cyclic = pi.cyclic;
        path.optimizeStart = pi.cyclic;
        path.directed = !pi.cyclic;
        auto& points = path.points;
        points.resize(n);
        auto dst = points.begin();
        int skip = 0; // used to skip the first point of all but the  first path of the chain
        IterateChain(pi, [&](PathInfo& it, bool forward)
                     {
            assert(!it.inOutput && it.inChain);
            it.inOutput = true;

            if (it.path->curved)
                path.curved = true;

            for (auto idx : it.path->sourcePathIndices)
                path.sourcePathIndices.insert(idx);

            auto const& src = it.path->points;
            if (forward)
                dst = std::copy(src.begin() + skip, src.end(), dst);
            else
                dst = std::copy(src.rbegin() + skip, src.rend(), dst);
            skip = 1; });
        assert(dst - points.begin() == static_cast<ptrdiff_t>(n)); // we should have copied exactly as many points as we reserved
    }

    return ret;
}

/**
Optimizes the start positions where allowed by rotating them to (locally) reduce overhead path length.
*/
std::vector<Path> ImproveStartPositions(std::vector<Path> const& paths)
{
    if (paths.size() < 2)
        return paths;

    std::vector<size_t> rotations(paths.size());

    size_t const n = paths.size();
    auto length = [&](size_t i, size_t rotation)
    {
        size_t const iLeft = (i + n - 1) % n;
        size_t const iRight = (i + 1) % n;
        Path const& left = paths[iLeft];
        Path const& mid = paths[i];
        Path const& right = paths[iRight];

        assert(mid.cyclic && mid.optimizeStart);
        assert(rotation < mid.points.size());

        Point2 const& a = left.points[left.cyclic ? rotations[iLeft] : (rotations[iLeft] + 1) % left.points.size()];
        Point2 const& b = mid.points[rotation];
        Point2 const& c = right.points[rotations[iRight]];

        return Distance(a, b) + Distance(b, c);
    };

    for (;;)
    {
        bool madeProgress = false;

        for (size_t i = 0; i < paths.size(); ++i)
        {
            Path const& path = paths[i];
            size_t const n = path.points.size();

            if (!path.cyclic || !path.optimizeStart)
                continue;

            double const orgLen = length(i, rotations[i]);

            for (size_t rot : {(rotations[i] + n - 1) % n, (rotations[i] + 1) % n})
            {
                if (length(i, rot) < orgLen)
                {
                    rotations[i] = rot;
                    madeProgress = true;
                    break;
                }
            }
        }

        if (!madeProgress)
            break;
    }

    std::vector<Path> ret = paths;

    for (size_t i = 0; i < ret.size(); ++i)
    {
        size_t const rot = rotations[i];

        Path const& path = paths[i];
        Path& optPath = ret[i];
        auto dst = std::copy(path.points.begin() + rot, path.points.end(), optPath.points.begin());
        std::copy(path.points.begin(), path.points.begin() + rot, dst);
    }

    return ret;
}

/**
Swaps a path with its successor if that decreases the sum of distance that has to be traveled to visit the paths.
*/
bool ImproveOrderLocally(std::vector<Path>& paths)
{
    if (paths.size() < 3)
        return false;

    size_t const n = paths.size();

    // summs up the distance between the end- and start-points of four paths
    auto overhead = [&](std::array<size_t, 4> const& idxs)
    {
        double sum = 0;
        for (size_t i = 1; i < idxs.size(); ++i)
        {
            Path const& a = paths[idxs[i - 1]];
            Path const& b = paths[idxs[i]];
            sum += Distance(a.EndPoint(), b.StartPoint());
        }
        return sum;
    };

    bool wasUseful = false;
    for (size_t i = 0; i < n; ++i)
    {
        std::array a{i, (i + 1) % n, (i + 2) % n, (i + 3) % n};
        std::array b{i, (i + 2) % n, (i + 1) % n, (i + 3) % n};

        if (overhead(a) > overhead(b))
        {
            std::swap(paths[a[1]], paths[a[2]]);
            wasUseful = true;
        }
    }

    return wasUseful;
}

/**
Reverses the direction of each path with @ref Path::optimizeStart if that decreases the distance to its successor and predecessor.
*/
void ImproveDirsLocally(std::vector<Path>& paths)
{
    if (paths.size() < 2)
        return;

    size_t const n = paths.size();
    for (size_t i = 0; i < n; ++i)
    {
        Path& cur = paths[i];
        if (!cur.optimizeStart || cur.cyclic || cur.StartPoint() == cur.EndPoint())
            continue;

        Path const& prev = paths[(i + n - 1) % n];
        Path const& next = paths[(i + 1) % n];

        double a = Distance(prev.EndPoint(), cur.StartPoint()) + Distance(cur.EndPoint(), next.StartPoint());
        double b = Distance(prev.EndPoint(), cur.EndPoint()) + Distance(cur.StartPoint(), next.StartPoint());

        if (b < a)
            std::reverse(cur.points.begin(), cur.points.end());
    }
}

namespace
{

typedef uint64_t serpinsky_index_t;

// https://www2.isye.gatech.edu/~jjb/research/mow/mow.pdf
serpinsky_index_t SerpinskyIndex(Vec2 const& v)
{
    serpinsky_index_t const maxInput = 1u << 30;

    assert(v.x >= 0 && v.x <= 1);
    assert(v.y >= 0 && v.y <= 1);

    double x = v.x * maxInput;
    double y = v.y * maxInput;

    serpinsky_index_t loopIndex = maxInput;
    serpinsky_index_t result = 0;

    if (x > y)
    {
        ++result;
        x = maxInput - x;
        y = maxInput - y;
    }

    while (loopIndex > 0)
    {
        result *= 2;

        if (x + y > maxInput)
        {
            ++result;
            double oldx = x;
            x = maxInput - y;
            y = oldx;
        }

        x *= 2;
        y *= 2;

        result *= 2;

        if (y > maxInput)
        {
            ++result;
            double oldx = x;
            x = y - maxInput;
            y = maxInput - oldx;
        }

        loopIndex /= 2;
    }

    return result;
}

} // namespace

/**
Returns the paths in optimized order and with optimized start position where allowed.
*/
std::vector<Path> RouteOptimized(std::vector<Path> const& paths)
{
    struct PathInfo
    {
        Path const* path;
        bool used = false;
    };

    struct PointInfo
    {
        PathInfo* pathInfo;
        size_t idx;
        serpinsky_index_t serpinskyIdx;
    };

    if (paths.empty())
        return paths;

    Point2 min = paths.front().StartPoint();
    Point2 max = min;

    for (auto const& path : paths)
    {
        for (Point2 const& p : path.points)
        {
            min.Minimize(p);
            max.Maximize(p);
        }
    }

    Vec2 const size = max - min;
    double const d = std::max(size.x, size.y);
    if (!d)
        throw DxfReadError("Paths have a total length of zero.");
    double const s = 1. / d;

    auto serp = [&](Point2 p)
    {
        return SerpinskyIndex((p - min) * s);
    };

    std::vector<PointInfo> pointInfos;
    std::vector<PathInfo> pathInfos(paths.size());
    for (size_t i = 0; i < paths.size(); ++i)
    {
        Path const& path = paths[i];
        PathInfo& pi = pathInfos[i];
        pi.path = &path;

        if (path.cyclic)
        {
            for (size_t j = 0; j < path.points.size(); ++j)
            {
                PointInfo pti;
                pti.pathInfo = &pi;
                pti.idx = j;
                pti.serpinskyIdx = serp(path.points[j]);
                pointInfos.push_back(pti);

                if (!path.optimizeStart)
                    break;
            }
        }
        else
        {
            PointInfo pti;
            pti.pathInfo = &pi;
            {
                pti.idx = 0;
                pti.serpinskyIdx = serp(path.StartPoint());
                pointInfos.push_back(pti);
            }

            if (pi.path->optimizeStart)
            {
                pti.idx = path.points.size() - 1;
                pti.serpinskyIdx = serp(path.EndPoint());
                pointInfos.push_back(pti);
            }
        }
    }

    std::sort(pointInfos.begin(), pointInfos.end(), [&](PointInfo const& a, PointInfo const& b)
              { return a.serpinskyIdx < b.serpinskyIdx; });

    std::vector<Path> ret;
    ret.reserve(paths.size());
    for (PointInfo& pti : pointInfos)
    {
        PathInfo& pi = *pti.pathInfo;
        if (pi.used || (!pi.path->optimizeStart && pti.idx))
            continue;

        pi.used = true;

        Path const& path = *pi.path;

        if (path.optimizeStart)
        {
            if (path.cyclic)
            {
                Path optPath = path;
                auto dst = std::copy(path.points.begin() + pti.idx, path.points.end(), optPath.points.begin());
                std::copy(path.points.begin(), path.points.begin() + pti.idx, dst);
                ret.push_back(optPath);
                continue;
            }
        }

        ret.push_back(path);
    }
    assert(paths.size() == ret.size());

    return ret;
}

} // nsDxfReader
