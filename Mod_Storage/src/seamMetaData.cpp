#include "seamMetaData.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

namespace precitec
{
namespace storage
{

Nio Nio::parse(const QJsonObject& jsonObject)
{
    Nio nio;

    auto it = jsonObject.find(QLatin1String("type"));
    if (it != jsonObject.end())
    {
        nio.type = it.value().toInt();
    } else
    {
        nio.valid = false;
    }

    it = jsonObject.find(QLatin1String("count"));
    if (it != jsonObject.end())
    {
        nio.count = it.value().toInt();
    } else
    {
        nio.valid = false;
    }

    return nio;
}

SeamMetaData SeamMetaData::parse(const QFileInfo &seamSeries, int number)
{
    if (!seamSeries.exists() || !seamSeries.isDir())
    {
        return {};
    }

    const QDir dir{seamSeries.absoluteFilePath()};
    const QDir seamDir{dir.absoluteFilePath(QStringLiteral("seam%1").arg(number, 4, 10, QLatin1Char('0')))};
    if (!seamDir.exists())
    {
        return {};
    }
    return parse(seamDir);
}

SeamMetaData SeamMetaData::parse(const QFileInfo &seamDir)
{
    return parse(QDir{seamDir.absoluteFilePath()});
}

SeamMetaData SeamMetaData::parse(const QDir &seamDir)
{
    return parse(seamDir.absoluteFilePath(QStringLiteral("metadata.json")));
}

SeamMetaData SeamMetaData::parse(const QString &absoluteFilePath)
{
    QFile metaDataFile(absoluteFilePath);
    if (!metaDataFile.open(QIODevice::ReadOnly))
    {
        return {};
    }

    return parse(QJsonDocument::fromJson(metaDataFile.readAll()).object());
}

SeamMetaData SeamMetaData::parse(const QJsonObject& jsonObject)
{
    SeamMetaData metaData;

    auto it = jsonObject.find(QLatin1String("uuid"));
    if (it != jsonObject.end())
    {
        metaData.m_uuid = QUuid::fromString(it.value().toString());
        metaData.m_uuidValid = true;
    }
    it = jsonObject.find(QLatin1String("number"));
    if (it != jsonObject.end())
    {
        metaData.m_number = it.value().toInt();
        metaData.m_numberValid = true;
    }
    it = jsonObject.find(QLatin1String("seamSeries"));
    if (it != jsonObject.end())
    {
        metaData.m_seamSeries = it.value().toInt();
        metaData.m_seamSeriesValid = true;
    }
    it = jsonObject.find(QLatin1String("seamSeriesUuid"));
    if (it != jsonObject.end())
    {
        metaData.m_seamSeriesUuid = QUuid::fromString(it.value().toString());
        metaData.m_seamSeriesUuidValid = true;
    }
    it = jsonObject.find(QLatin1String("linkTo"));
    if (it != jsonObject.end())
    {
        metaData.m_linkTo = QUuid::fromString(it.value().toString());
        metaData.m_linkToValid = true;
    }
    it = jsonObject.find(QLatin1String("length"));
    if (it != jsonObject.end())
    {
        metaData.m_length = it.value().toInt();
        metaData.m_lengthValid = true;
    }
    it = jsonObject.find(QLatin1String("nio"));
    if (it != jsonObject.end())
    {
        if (it.value().type() == QJsonValue::Array)
        {
            const auto jsonArray = it.value().toArray();
            metaData.m_nios.reserve(jsonArray.size());
            std::transform(jsonArray.begin(), jsonArray.end(), std::back_inserter(metaData.m_nios), [] (const auto& value) { return Nio::parse(value.toObject()); });
            metaData.m_nio = !metaData.m_nios.empty();
            metaData.m_nioValid = std::all_of(metaData.m_nios.begin(), metaData.m_nios.end(), [] (const auto& nio) { return nio.valid; });
        } else
        {
            metaData.m_nio = it.value().toBool();
            metaData.m_nioValid = true;
        }
    }
    it = jsonObject.find(QLatin1String("nioSwitchedOff"));
    if (it != jsonObject.end())
    {
        metaData.m_nioSwitchedOff = it.value().toBool();
        metaData.m_nioSwitchedOffValid = true;
    }

    return metaData;
}

}
}

