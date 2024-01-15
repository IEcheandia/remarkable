#pragma once

#include <filter/algoArray.h> ///< algorithmic interface for class TArray
#include "module/moduleLogger.h"

namespace precitec
{
namespace filter
{
    enum class MultipleIndicesStrategy
    {
        UseFirstIndexes,
        ExtractShortestLine,
        ExtractLongestLineWithPadding,
        ExtractLongestLineWithoutPadding
    };

    inline double chooseStartIndex(const geo2d::Doublearray& start, const std::string& filterName, const MultipleIndicesStrategy& strategy)
    {
        double minStart;
        double maxStart;
        double validStart = start.getRank().front();
        std::tie(validStart, minStart, maxStart) = min_maxValidArrayElement(start);

        double globalStart = start.getData().front();
        if (!validStart)
        {
            wmLog(eDebug, "%s: No valid start index, taking the first one anyway \n", filterName);
        }
        else
        {
            switch (strategy)
            {
            case MultipleIndicesStrategy::UseFirstIndexes:
                assert(globalStart == start.getData().front());
                break;
            case MultipleIndicesStrategy::ExtractShortestLine:
                globalStart = maxStart;
                break;
            case MultipleIndicesStrategy::ExtractLongestLineWithPadding:
                globalStart = minStart;
                break;
            case MultipleIndicesStrategy::ExtractLongestLineWithoutPadding:
                globalStart = minStart;
                break;
            }
        }

        return globalStart;
    };

    inline double chooseEndIndex(const geo2d::Doublearray& end, const std::string& filterName, const MultipleIndicesStrategy& strategy)
    {
        double minEnd;
        double maxEnd;
        double validEnd = end.getRank().front();
        std::tie(validEnd, minEnd, maxEnd) = min_maxValidArrayElement(end);

        double globalEnd = end.getData().front();
        if (!validEnd)
        {
            wmLog(eDebug, "%s: No valid end index, taking the first one anyway \n", filterName);
        }
        else
        {
            switch (strategy)
            {
            case MultipleIndicesStrategy::UseFirstIndexes:
                assert(globalEnd == end.getData().front());
                break;
            case MultipleIndicesStrategy::ExtractShortestLine:
                globalEnd = minEnd;
                break;
            case MultipleIndicesStrategy::ExtractLongestLineWithPadding:
                globalEnd = maxEnd;
                break;
            case MultipleIndicesStrategy::ExtractLongestLineWithoutPadding:
                globalEnd = maxEnd;
                break;
            }
        }

        return globalEnd;
    };
}
}
