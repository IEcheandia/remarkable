#pragma once

class QColor;
class QString;

namespace precitec
{
namespace interface
{

class ResultArgs;

}

/**
 * @returns A name for the given @p result, if the result is unknown it returns the type number converted to string.
 **/
QString nameForResult(const precitec::interface::ResultArgs &result);
QString nameForResult(int type);

/**
 * @returns A color for the given @p result, @c Qt::black if the result type is unknonw.
 **/
QColor colorForResult(const precitec::interface::ResultArgs &result);
QColor colorForResult(int type);

QString nameForQualityError(int enumType);

}

