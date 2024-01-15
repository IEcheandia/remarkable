#pragma once

#include <fstream>
#include <iomanip>

#include "editorDataTypes.h"
#include "json.hpp"

using json = nlohmann::ordered_json;

namespace RTC6
{

namespace wobbleFigure
{
namespace command
{
    void to_json(json &jsonObject, const Order &order)
    {
        jsonObject = {
                {"EndPosition", order.endPosition},
                {"Power", order.power},
                {"RingPower", order.ringPower}
            };
    }
    void from_json(const json &jsonObject, Order &order)
    {
        jsonObject.at("EndPosition").get_to(order.endPosition);
        jsonObject.at("Power").get_to(order.power);
        if (jsonObject.contains("RingPower"))
        {
            jsonObject.at("RingPower").get_to(order.ringPower);
        }
        else
        {
            order.ringPower = 0.0;
        }
    }
}
    void to_json(json &jsonObject, const Figure &figure)
    {
        jsonObject = json{
            {"Name", figure.name},
            {"ID", figure.ID},
            {"Description", figure.description},
            {"MicroVectorFactor", figure.microVectorFactor},
            {"PowerModulationMode", figure.powerModulationMode},
            {"Figure", figure.figure}
        };
    }
    void from_json(const json &jsonObject, Figure &figure)
    {
        jsonObject.at("Name").get_to(figure.name);
        jsonObject.at("ID").get_to(figure.ID);
        jsonObject.at("Description").get_to(figure.description);
        jsonObject.at("MicroVectorFactor").get_to(figure.microVectorFactor);
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
}

namespace seamFigure
{
namespace command
{
    void to_json(json &jsonObject, const Order &order)
    {
        jsonObject = {
            {"EndPosition", order.endPosition},
            {"Power", order.power},
            {"RingPower", order.ringPower},
            {"Velocity", order.velocity}
        };
    }
    void from_json(const json &jsonObject, Order &order)
    {
        jsonObject.at("EndPosition").get_to(order.endPosition);
        jsonObject.at("Power").get_to(order.power);
        jsonObject.contains("RingPower") ? jsonObject.at("RingPower").get_to(order.ringPower) : order.ringPower = -1.0;
        jsonObject.contains("Velocity") ? jsonObject.at("Velocity").get_to(order.velocity) : order.velocity = -1.0;
    }
}

    void to_json(json &jsonObject, const Ramp &ramp)
    {
        jsonObject = json{
            {"StartID", ramp.startPointID},
            {"Length", ramp.length},
            {"StartPower", ramp.startPower},
            {"EndPower", ramp.endPower},
            {"StartPowerRing", ramp.startPowerRing},
            {"EndPowerRing", ramp.endPowerRing}
        };
    }
    void from_json(const json &jsonObject, Ramp &ramp)
    {
        jsonObject.at("StartID").get_to(ramp.startPointID);
        jsonObject.at("Length").get_to(ramp.length);
        jsonObject.at("StartPower").get_to(ramp.startPower);
        jsonObject.at("EndPower").get_to(ramp.endPower);
        jsonObject.at("StartPowerRing").get_to(ramp.startPowerRing);
        jsonObject.at("EndPowerRing").get_to(ramp.endPowerRing);
    }

    void to_json(json &jsonObject, const SeamFigure &figure)
    {
        jsonObject = json{
            {"Name", figure.name},
            {"ID", figure.ID},
            {"Description", figure.description},
            {"Ramps", figure.ramps},
            {"Figure", figure.figure}
        };
    }
    void from_json(const json &jsonObject, SeamFigure &figure)
    {
        jsonObject.at("Name").get_to(figure.name);
        jsonObject.at("ID").get_to(figure.ID);
        jsonObject.at("Description").get_to(figure.description);
        if (jsonObject.contains("Ramps"))
        {
            jsonObject.at("Ramps").get_to(figure.ramps);
        }
        jsonObject.at("Figure").get_to(figure.figure);
    }
}

namespace function
{
    void to_json(json &jsonObject, const OverlayFunction &figure)
    {
        jsonObject = json{
            {"Name", figure.name},
            {"ID", figure.ID},
            {"Description", figure.description},
            {"FunctionValues", figure.functionValues}
        };
    }
    void from_json(const json &jsonObject, OverlayFunction &figure)
    {
        jsonObject.at("Name").get_to(figure.name);
        jsonObject.at("ID").get_to(figure.ID);
        jsonObject.at("Description").get_to(figure.description);
        jsonObject.at("FunctionValues").get_to(figure.functionValues);
    }
}

    bool printToFile(const json &jsonObject, const std::string &filename)
    {
        std::ofstream file;
        file.open(filename);
        if (!file.is_open())
        {
            return false;
        }

        try
        {
            file << std::setw(2) << jsonObject << std::endl;
        }
        catch(...)
        {
            file.close();
            return false;
        }

        file.close();
        return true;
    }

    bool readFromFile(json &jsonObject, const std::string &filename)
    {
        std::ifstream file;
        file.open(filename);
        if (!file.is_open())
        {
            return false;
        }

        try
        {
            file >> jsonObject;
        }
        catch (...)
        {
            file.close();
            return false;
        }

        file.close();
        return true;
    }
}
