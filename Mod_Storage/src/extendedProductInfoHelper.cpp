#include "extendedProductInfoHelper.h"

namespace precitec
{
namespace storage
{

ExtendedProductInfoHelper::ExtendedProductInfoHelper(QObject *parent)
    : QObject(parent)
{
}

ExtendedProductInfoHelper::~ExtendedProductInfoHelper() = default;

void ExtendedProductInfoHelper::setSerialNumberFromExtendedProductInfo(bool value)
{
    if (m_serialNumberFromExtendedProductInfo == value)
    {
        return;
    }
    m_serialNumberFromExtendedProductInfo = value;
    emit serialNumberFromExtendedProductInfoChanged();
}

void ExtendedProductInfoHelper::setSerialNumberFromExtendedProductInfoField(uint value)
{
    if (m_serialNumberFromExtendedProductInfoField == value)
    {
        return;
    }
    m_serialNumberFromExtendedProductInfoField = value;
    emit serialNumberFromExtendedProductInfoFieldChanged();
}

void ExtendedProductInfoHelper::setPartNumberFromExtendedProductInfo(bool value)
{
    if (m_partNumberFromExtendedProductInfo == value)
    {
        return;
    }
    m_partNumberFromExtendedProductInfo = value;
    emit partNumberFromExtendedProductInfoChanged();
}

void ExtendedProductInfoHelper::setPartNumberFromExtendedProductInfoField(uint value)
{
    if (m_partNumberFromExtendedProductInfoField == value)
    {
        return;
    }
    m_partNumberFromExtendedProductInfoField = value;
    emit partNumberFromExtendedProductInfoFieldChanged();
}

std::optional<QString> ExtendedProductInfoHelper::serialNumber(QStringView extendedProductInfo) const
{
    return splitHelper(extendedProductInfo, m_serialNumberFromExtendedProductInfo, m_serialNumberFromExtendedProductInfoField);
}

std::optional<QString> ExtendedProductInfoHelper::partNumber(QStringView extendedProductInfo) const
{
    return splitHelper(extendedProductInfo, m_partNumberFromExtendedProductInfo, m_partNumberFromExtendedProductInfoField);
}

std::optional<QString> ExtendedProductInfoHelper::splitHelper(QStringView extendedProductInfo, bool use, uint field) const
{
    if (!use)
    {
        return {};
    }
    const auto parts = extendedProductInfo.split(QLatin1Char('\n'));
    if (parts.size() > int(field))
    {
        if (const auto &part = parts.at(field); !part.isEmpty())
        {
            return part.toString();
        }
    }
    return {};
}

}
}
