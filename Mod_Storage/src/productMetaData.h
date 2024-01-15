#pragma once

#include "seamSeriesMetaData.h"

#include <QDateTime>
#include <QFileInfo>
#include <QUuid>

#include <optional>

class QDir;

namespace precitec
{
namespace storage
{

/**
 * Helper class to parse the MetaData file of a product instance.
 **/
class ProductMetaData
{
public:
    /**
     * Whether the uuid was parsed successfully.
     **/
    bool isUuidValid() const
    {
        return m_uuidValid;
    }
    /**
     * Whether the serial number was parsed successfully.
     **/
    bool isNumberValid() const
    {
        return m_numberValid;
    }
    /**
     * Whether nios were parsed successfully.
     **/
    bool isNioValid() const
    {
        return m_nioValid;
    }
    /**
     * Whether nioSwitchedOff was parsed successfully.
     **/
    bool isNioSwitchedOffValid() const
    {
        return m_nioSwitchedOffValid;
    }
    /**
     * Whether date was parsed successfully.
     **/
    bool isDateValid() const
    {
        return m_dateValid;
    }
    /**
     * Whether seam seriess were parsed successfully.
     **/
    bool isSeamSeriesValid() const
    {
        return m_seamSeriesValid;
    }
    /**
     * Whether seams were parsed successfully.
     **/
    bool isSeamsValid() const
    {
        return m_seamsValid;
    }

    const QUuid &uuid() const
    {
        return m_uuid;
    }
    std::uint32_t number() const
    {
        return m_number;
    }
    const QString &extendedProductInfo() const
    {
        return m_extendedProductInfo;
    }
    bool nio() const
    {
        return m_nio;
    }
    bool nioSwitchedOff() const
    {
        return m_nioSwitchedOff;
    }
    const QDateTime &date() const
    {
        return m_date;
    }
    const std::vector<SeamSeriesMetaData>& seamSeries() const
    {
        return m_seamSeries;
    }
    const std::vector<SeamMetaData>& seams() const
    {
        return m_seams;
    }
    const std::vector<Nio>& nios() const
    {
        return m_nios;
    }
    std::optional<QUuid> productUuid() const
    {
        return m_productUuid;
    }
    std::optional<QString> productName() const
    {
        return m_productName;
    }
    std::optional<int> productType() const
    {
        return m_productType;
    }


    /**
     * The path to the Metadata file
     **/
    const QFileInfo &filePath() const
    {
        return m_filePath;
    }

    /**
     * Parses the metadata file found in @p productInstanceDir, which must point to a directory.
     **/
    static ProductMetaData parse(const QFileInfo &productInstanceDir);
    /**
     * Parses the metadata file found in @p productInstanceDir.
     **/
    static ProductMetaData parse(const QDir &productInstanceDir);

private:
    static ProductMetaData parse(const QString &absoluteFilePath);

    std::optional<QUuid> m_productUuid;
    std::optional<int> m_productType;
    std::optional<QString> m_productName;

    QUuid m_uuid;
    std::uint32_t m_number = 0;
    QString m_extendedProductInfo;
    bool m_nio = false;
    bool m_nioSwitchedOff = false;
    QDateTime m_date;
    std::vector<SeamSeriesMetaData> m_seamSeries;
    std::vector<SeamMetaData> m_seams;
    std::vector<Nio> m_nios;

    bool m_uuidValid = false;
    bool m_numberValid = false;
    bool m_nioValid = false;
    bool m_nioSwitchedOffValid = false;
    bool m_dateValid = false;
    bool m_seamSeriesValid = false;
    bool m_seamsValid = false;

    QFileInfo m_filePath;
};

}
}
