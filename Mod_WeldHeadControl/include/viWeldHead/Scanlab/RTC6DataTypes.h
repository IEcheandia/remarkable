#pragma once
#include <string>
#include <vector>

namespace RTC6
{
namespace command
{
    struct Order
    {
        std::pair<double, double> endPosition;
        double relativePower;
        double relativeRingPower;
        double velocity;
    };
}
    struct Figure
    {
        std::string name;
        std::string ID;
        std::string description;
        unsigned int microVectorFactor;
        unsigned int powerModulationMode;
        std::vector<RTC6::command::Order> figure;
    };
}

