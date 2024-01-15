#pragma once

#include "seamMetaData.h"

#include <QUuid>
#include <vector>

class QFileInfo;
class QDir;

namespace precitec
{
namespace storage
{

/**
 * Helper class to parse the MetaData file of a seam series.
 **/
class SeamMetaData;

class SeamSeriesMetaData
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
    int number() const
    {
        return m_number;
    }
    bool nio() const
    {
        return m_nio;
    }
    bool nioSwitchedOff() const
    {
        return m_nioSwitchedOff;
    }
    const std::vector<SeamMetaData>& seams() const
    {
        return m_seams;
    }
    const std::vector<Nio>& nios() const
    {
        return m_nios;
    }

    /**
     * Parses the metadata file for the seam series with @p number in @p productInstance.
     **/
    static SeamSeriesMetaData parse(const QFileInfo &productInstance, int number);
    /**
     * Parses the metadata file found in @p seamSeriesDir, which must point to a directory.
     **/
    static SeamSeriesMetaData parse(const QFileInfo &seamSeriesDir);
    /**
     * Parses the metadata file found in @p seamSeriesDir.
     **/
    static SeamSeriesMetaData parse(const QDir &seamSeriesDir);
    /**
     * Parses the json object @p jsonObject.
     **/
    static SeamSeriesMetaData parse(const QJsonObject& jsonObject);

private:
    static SeamSeriesMetaData parse(const QString &absoluteFilePath);

    QUuid m_uuid;
    int m_number = 0;
    bool m_nio = false;
    bool m_nioSwitchedOff = false;
    std::vector<SeamMetaData> m_seams;
    std::vector<Nio> m_nios;

    bool m_uuidValid = false;
    bool m_numberValid = false;
    bool m_nioValid = false;
    bool m_nioSwitchedOffValid = false;
    bool m_seamsValid = false;
};

}
}
