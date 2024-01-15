/**
* @file
* @copyright    Precitec Vision GmbH & Co. KG
* @author       MM
* @date         2021
* @brief        Some helper structs/classes for PoorPenetrationChecker and PoorPenetrationCheckerTriple
*/

#include "poorPenetrationPaint.h"

namespace precitec
{
namespace filter
{
InfoLine::InfoLine()
{
    _number = 0;
    _value = 0;
    _color = image::Color::Black();
}

InfoLine::InfoLine(int number, int value, image::Color color)
{
    _number = number;
    _value = value;
    _color = color;
}

std::string InfoLine::getLine()
{
    switch (_number)
    {
    case 0:
        return "Unit.None:Unit.None:0";
        break;
    case 1:
        return "Length:Unit.Pixels:" + convertIntToString(_value);
        break;
    case 2:
        return "DX:Unit.Pixels:" + convertIntToString(_value);
        break;
    case 3:
        return "Gradient:Unit.None:" + convertIntToString(_value);
        break;
    case 4:
        return "GreyvalGap:Unit.None:" + convertIntToString(_value);
        break;
    case 5:
        return "GreyvalInner:Unit.None:" + convertIntToString(_value);
        break;
    case 6:
        return "GreyvalOuter:Unit.None:" + convertIntToString(_value);
        break;
    case 7:
        return "StdDev:Unit.None:" + convertIntToString(_value);
        break;
    case 8:
        return "DevLengthL:Unit.None:" + convertIntToString(_value);
        break;
    case 9:
        return "DevLengthR:Unit.None:" + convertIntToString(_value);
        break;
    case 10:
        return "Result:Unit.None:" + convertIntToString(_value);
        break;
    }
    return "Unit.None:Unit.None:0";
}

std::string InfoLine::spaces(int i)
{
    std::ostringstream strstream;
    for (int j = 0; j < i; j++)
    {
        strstream << ".";
    }
    return strstream.str();
}

std::string InfoLine::convertIntToString(int i)
{
    std::ostringstream strstream;
    strstream << i;
    return strstream.str();
}

PoorPenetrationOverlay::PoorPenetrationOverlay()
{
    reset();
}

void PoorPenetrationOverlay::reset()
{
    m_pointContainer.clear();
    m_lineContainer.clear();
    m_rectangleContainer.clear();
}

const std::vector<PoorPenetrationPoint> & PoorPenetrationOverlay::getPointContainer() const
{
    return m_pointContainer;
}

const std::vector<PoorPenetrationLine> & PoorPenetrationOverlay::getLineContainer() const
{
    return m_lineContainer;
}

const std::vector<PoorPenetrationRectangle> & PoorPenetrationOverlay::getRectangleContainer() const
{
    return m_rectangleContainer;
}

void PoorPenetrationOverlay::addPoint(int x, int y, image::Color color)
{
    m_pointContainer.push_back(PoorPenetrationPoint(x, y, color));
}

void PoorPenetrationOverlay::addLine(int x1, int y1, int x2, int y2, image::Color color)
{
    m_lineContainer.push_back(PoorPenetrationLine(x1, y1, x2, y2, color));
}

void PoorPenetrationOverlay::addRectangle(int x, int y, int width, int height, image::Color color)
{
    m_rectangleContainer.push_back(PoorPenetrationRectangle(x, y, width, height, color));
}

PoorPenetrationPoint::PoorPenetrationPoint()
{
    x = 0;
    y = 0;
    color = image::Color::Black();
}

PoorPenetrationPoint::PoorPenetrationPoint(int x, int y, image::Color color)
{
    this->x = x;
    this->y = y;
    this->color = color;
}

PoorPenetrationLine::PoorPenetrationLine()
{
    x1 = 0;
    y1 = 0;
    x2 = 0;
    y2 = 0;
    color = image::Color::Black();
}

PoorPenetrationLine::PoorPenetrationLine(int x1, int y1, int x2, int y2, image::Color color)
{
    this->x1 = x1;
    this->y1 = y1;
    this->x2 = x2;
    this->y2 = y2;
    this->color = color;
}

PoorPenetrationRectangle::PoorPenetrationRectangle()
{
    x = 0;
    y = 0;
    width = 0;
    height = 0;
    color = image::Color::Black();
}

PoorPenetrationRectangle::PoorPenetrationRectangle(int x, int y, int width, int height, image::Color color)
{
    this->x = x;
    this->y = y;
    this->width = width;
    this->height = height;
    this->color = color;
}
}
}
