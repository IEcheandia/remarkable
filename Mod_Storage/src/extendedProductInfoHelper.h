#pragma once

#include <QObject>

#include <optional>

namespace precitec
{
namespace storage
{

/**
 * Helper class to extract serial number and part number from extended product info string.
 **/
class ExtendedProductInfoHelper : public QObject
{
    Q_OBJECT
    /**
     * Whether to take serial number from extended product info.
     * @see serialNumberFromExtendedProductInfoField
     **/
    Q_PROPERTY(bool serialNumberFromExtendedProductInfo READ serialNumberFromExtendedProductInfo WRITE setSerialNumberFromExtendedProductInfo NOTIFY serialNumberFromExtendedProductInfoChanged)
    /**
     * Which field of extended product info is the serial number
     * @see serialNumberFromExtendedProductInfo
     **/
    Q_PROPERTY(uint serialNumberFromExtendedProductInfoField READ serialNumberFromExtendedProductInfoField WRITE setSerialNumberFromExtendedProductInfoField NOTIFY serialNumberFromExtendedProductInfoFieldChanged)

    /**
     * Whether to take part number from extended product info.
     * @see partNumberFromExtendedProductInfoField
     **/
    Q_PROPERTY(bool partNumberFromExtendedProductInfo READ partNumberFromExtendedProductInfo WRITE setPartNumberFromExtendedProductInfo NOTIFY partNumberFromExtendedProductInfoChanged)
    /**
     * Which field of extended product info is the part number
     * @see partNumberFromExtendedProductInfo
     **/
    Q_PROPERTY(uint partNumberFromExtendedProductInfoField READ partNumberFromExtendedProductInfoField WRITE setPartNumberFromExtendedProductInfoField NOTIFY partNumberFromExtendedProductInfoFieldChanged)
public:
    ExtendedProductInfoHelper(QObject *parent = nullptr);
    ~ExtendedProductInfoHelper();

    bool serialNumberFromExtendedProductInfo() const
    {
        return m_serialNumberFromExtendedProductInfo;
    }
    void setSerialNumberFromExtendedProductInfo(bool value);
    uint serialNumberFromExtendedProductInfoField() const
    {
        return m_serialNumberFromExtendedProductInfoField;
    }
    void setSerialNumberFromExtendedProductInfoField(uint value);

    bool partNumberFromExtendedProductInfo() const
    {
        return m_partNumberFromExtendedProductInfo;
    }
    void setPartNumberFromExtendedProductInfo(bool value);
    uint partNumberFromExtendedProductInfoField() const
    {
        return m_partNumberFromExtendedProductInfoField;
    }
    void setPartNumberFromExtendedProductInfoField(uint value);

    /**
     * Extracts the serial number from the @p extendedProductInfo depending on the settings.
     * @returns nullopt in case of empty or feature disabled, otherwise the extracted string.
     **/
    std::optional<QString> serialNumber(QStringView extendedProductInfo) const;
    /**
     * Extracts the part number from the @p extendedProductInfo depending on the settings.
     * @returns nullopt in case of empty or feature disabled, otherwise the extracted string.
     **/
    std::optional<QString> partNumber(QStringView extendedProductInfo) const;

Q_SIGNALS:
    void serialNumberFromExtendedProductInfoChanged();
    void serialNumberFromExtendedProductInfoFieldChanged();
    void partNumberFromExtendedProductInfoChanged();
    void partNumberFromExtendedProductInfoFieldChanged();

private:
    std::optional<QString> splitHelper(QStringView extendedProductInfo, bool use, uint field) const;
    bool m_serialNumberFromExtendedProductInfo = false;
    uint m_serialNumberFromExtendedProductInfoField = 0;
    bool m_partNumberFromExtendedProductInfo = false;
    uint m_partNumberFromExtendedProductInfoField = 0;
};

}
}

Q_DECLARE_METATYPE(precitec::storage::ExtendedProductInfoHelper*)
