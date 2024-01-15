/**
* @file
* @copyright    Precitec Vision GmbH & Co. KG
* @author       MM
* @date         2021
* @brief        Some helper structs/classes for PoorPenetrationChecker and PoorPenetrationCheckerTriple
*/

#pragma once

#include "overlay/overlayCanvas.h"

namespace precitec
{
namespace filter
{
struct PoorPenetrationPoint
{
    PoorPenetrationPoint();
    PoorPenetrationPoint(int x, int y, image::Color color);
    int x;
    int y;
    image::Color color;
};

struct PoorPenetrationLine
{
    PoorPenetrationLine();
    PoorPenetrationLine(int x1, int y1, int x2, int y2, image::Color color);
    int x1;
    int y1;
    int x2;
    int y2;
    image::Color color;
};

struct PoorPenetrationRectangle
{
    PoorPenetrationRectangle();
    PoorPenetrationRectangle(int x, int y, int width, int height, image::Color color);
    int x;
    int y;
    int width;
    int height;
    image::Color color;
};

class PoorPenetrationOverlay
{
public:
    PoorPenetrationOverlay();

    void reset();

    const std::vector<PoorPenetrationPoint> & getPointContainer() const;
    const std::vector<PoorPenetrationLine> & getLineContainer() const;
    const std::vector<PoorPenetrationRectangle> & getRectangleContainer() const;

    void addPoint(int x, int y, image::Color color);
    void addLine(int x1, int y1, int x2, int y2, image::Color color);
    void addRectangle(int x, int y, int width, int height, image::Color color);

private:
    std::vector<PoorPenetrationPoint> m_pointContainer;
    std::vector<PoorPenetrationLine> m_lineContainer;
    std::vector<PoorPenetrationRectangle> m_rectangleContainer;
};

/**
 * @brief Holds one line of an overlay-textbox-output
 */
class InfoLine
{
public:
    InfoLine();
    InfoLine(int number, int value, image::Color color);
    std::string getLine();

    image::Color _color;

    int _number, _value;
private:
    std::string spaces(int i);
    std::string convertIntToString(int i);
};
}
}
