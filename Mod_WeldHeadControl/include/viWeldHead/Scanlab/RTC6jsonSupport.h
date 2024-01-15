#pragma once

#include "RTC6DataTypes.h"
#include <fstream>
#include <iomanip>
#include <iostream>
#include "json.hpp"

using json = nlohmann::ordered_json;

namespace RTC6
{
namespace command
{
    void to_json(json& jsonObject, const Order& order)
    {
        jsonObject = {
                {"EndPosition", order.endPosition},
                {"Power", order.relativePower},
                {"RingPower", order.relativeRingPower},
                {"Velocity", order.velocity},
            };
    }
    void from_json(const json &jsonObject, Order& order)
    {
        jsonObject.at("EndPosition").get_to(order.endPosition);
        jsonObject.at("Power").get_to(order.relativePower);
        if (jsonObject.contains("RingPower"))
        {
            jsonObject.at("RingPower").get_to(order.relativeRingPower);
        }
        else
        {
            order.relativeRingPower = 0.0;
        }
        if (jsonObject.contains("Velocity"))
        {
            jsonObject.at("Velocity").get_to(order.velocity);
        }
        else
        {
            order.velocity = -1.0;
        }
    }
}
    void to_json(json& jsonObject, const Figure& figure)
    {
        jsonObject = json{
            {"Name", figure.name},
            {"ID", figure.ID},
            {"Description", figure.description},
            {"MicroVectorFactor", figure.microVectorFactor},
            {"PowerModulationMode", figure.powerModulationMode},
            {"Figure", figure.figure},
        };
    }
    void from_json(const json &jsonObject, Figure& figure)
    {
        jsonObject.at("Name").get_to(figure.name);
        jsonObject.at("ID").get_to(figure.ID);
        jsonObject.at("Description").get_to(figure.description);
        if (jsonObject.contains("MicroVectorFactor"))
        {
            jsonObject.at("MicroVectorFactor").get_to(figure.microVectorFactor);
        }
        else
        {
            figure.microVectorFactor = 1;
        }
        if (jsonObject.contains("PowerModulationMode"))
        {
            jsonObject.at("PowerModulationMode").get_to(figure.powerModulationMode);
        }
        else
        {
            figure.powerModulationMode = 1;
        }
        jsonObject.at("Figure").get_to(figure.figure);
    }

    void readFromFile(json &j, const std::string& filename)
    {
        std::ifstream file;
        file.open(filename);
        if (file.is_open())
        {
            file >> j;
        }
        file.close();
    }
}

