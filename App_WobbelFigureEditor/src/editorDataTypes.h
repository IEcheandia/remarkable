#pragma once
#include <string>
#include <vector>
#include <QMetaType>

namespace RTC6
{
    namespace wobbleFigure
    {
        namespace command
        {
            struct Order
            {
                std::pair<double, double> endPosition;
                double power;
                double ringPower;
            };
        }
        struct Figure
        {
            std::string name;
            std::string ID;
            std::string description;
            unsigned int microVectorFactor;
            unsigned int powerModulationMode;
            std::vector<RTC6::wobbleFigure::command::Order> figure;
        };
    }

    namespace seamFigure
    {
        namespace command
        {
            struct Order
            {
                std::pair<double, double> endPosition;
                double power;
                double ringPower;
                double velocity;

                bool operator==(const Order& compare) const
                {
                    return std::get<0>(this->endPosition) == std::get<0>(compare.endPosition) && std::get<1>(this->endPosition) == std::get<1>(compare.endPosition) && this->power == compare.power && this->ringPower == compare.ringPower && this->velocity == compare.velocity;
                }
            };
        }
        struct Ramp
        {
            std::size_t startPointID;
            double length;
            double startPower;
            double endPower;
            double startPowerRing;
            double endPowerRing;

            bool operator==(const Ramp& compare) const
            {
                return this->startPointID == compare.startPointID && this->length == compare.length && this->startPower == compare.startPower && this->endPower == compare.endPower && this->startPowerRing == compare.startPowerRing && this->endPowerRing == compare.endPowerRing;
            }
        };

        struct SeamFigure
        {
            std::string name;
            std::string ID;
            std::string description;
            std::vector<Ramp> ramps;
            std::vector<RTC6::seamFigure::command::Order> figure;
        };
    }

    namespace function
    {
        struct OverlayFunction
        {
            std::string name;
            std::string ID;
            std::string description;
            std::vector<std::pair<double, double>> functionValues;
        };
    }
}

Q_DECLARE_METATYPE(RTC6::seamFigure::Ramp)
