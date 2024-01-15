#pragma once
#include <QUuid>
#include <vector>

class QFileInfo;
class QDir;
class QJsonObject;

namespace precitec
{
namespace storage
{

struct Nio {
    int type = -1;
    int count = -1;
    bool valid = true;

    static Nio parse(const QJsonObject& jsonObject);
};

/**
 * Helper class to parse the MetaData file of a seam.
 **/
class SeamMetaData
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
     * Whether the number was parsed successfully.
     **/
    bool isNumberValid() const
    {
        return m_numberValid;
    }
    /**
     * Whether the seam series number was parsed successfully.
     **/
    bool isSeamSeriesValid() const
    {
        return m_seamSeriesValid;
    }
    /**
     * Whether the seam series uuid was parsed successfully.
     **/
    bool isSeamSeriesUuidValid() const
    {
        return m_seamSeriesUuidValid;
    }
    /**
     * Whether the length was parsed successfully.
     **/
    bool isLengthValid() const
    {
        return m_lengthValid;
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
     * Whether linkTo was parsed successfully.
     **/
    bool isLinkToValid() const
    {
        return m_linkToValid;
    }

    const QUuid &uuid() const
    {
        return m_uuid;
    }
    int number() const
    {
        return m_number;
    }
    int seamSeries() const
    {
        return m_seamSeries;
    }
    const QUuid& seamSeriesUuid() const
    {
        return m_seamSeriesUuid;
    }
    int length() const
    {
        return m_length;
    }
    bool nio() const
    {
        return m_nio;
    }
    bool nioSwitchedOff() const
    {
        return m_nioSwitchedOff;
    }
    const QUuid& linkTo() const
    {
        return m_linkTo;
    }
    const std::vector<Nio>& nios() const
    {
        return m_nios;
    }

    /**
     * Parses the metadata file for the seam with @p number in @p seamSeries.
     **/
    static SeamMetaData parse(const QFileInfo &seamSeries, int number);
    /**
     * Parses the metadata file found in @p seamDir, which must point to a directory.
     **/
    static SeamMetaData parse(const QFileInfo &seamDir);
    /**
     * Parses the metadata file found in @p seamDir.
     **/
    static SeamMetaData parse(const QDir &seamDir);
    /**
     * Parses the json object @p jsonObject.
     **/
    static SeamMetaData parse(const QJsonObject& jsonObject);

private:
    static SeamMetaData parse(const QString &absoluteFilePath);

    QUuid m_uuid;
    int m_number = 0;
    int m_seamSeries = 0;
    QUuid m_seamSeriesUuid;
    int m_length = -1;
    bool m_nio = false;
    bool m_nioSwitchedOff = false;
    QUuid m_linkTo;
    std::vector<Nio> m_nios;

    bool m_uuidValid = false;
    bool m_numberValid = false;
    bool m_seamSeriesValid = false;
    bool m_seamSeriesUuidValid = false;
    bool m_lengthValid = false;
    bool m_nioValid = false;
    bool m_nioSwitchedOffValid = false;
    bool m_linkToValid = false;
};

}
}
