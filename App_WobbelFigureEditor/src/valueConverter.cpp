#include "valueConverter.h"

#include <QLocale>

using precitec::scanmaster::components::wobbleFigureEditor::ValueConverter;

namespace precitec
{
namespace scanmaster
{
namespace components
{
namespace wobbleFigureEditor
{

ValueConverter::ValueConverter(QObject* parent) : QObject(parent)
{ }

ValueConverter::~ValueConverter() = default;

ValueConverter* ValueConverter::instance()
{
    static ValueConverter s_instance;
    return &s_instance;
}

double ValueConverter::convertDoubleToPercent(double value) const
{
    return value * 100.0;
}

double ValueConverter::convertFromPercentDouble(double value) const
{
    return value * 0.01;
}

}
}
}
}
