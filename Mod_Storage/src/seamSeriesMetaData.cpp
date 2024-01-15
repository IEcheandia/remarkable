#include "seamSeriesMetaData.h"
#include "seamMetaData.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

namespace precitec
{
namespace storage
{

SeamSeriesMetaData SeamSeriesMetaData::parse(const QFileInfo &productInstance, int number)
{
    if (!productInstance.exists() || !productInstance.isDir())
    {
        return {};
    }

    const QDir dir{productInstance.absoluteFilePath()};
    const QDir seamSeriesDir{dir.absoluteFilePath(QStringLiteral("seam_series%1").arg(number, 4, 10, QLatin1Char('0')))};
    if (!seamSeriesDir.exists())
    {
        return {};
    }
    return parse(seamSeriesDir);
}

SeamSeriesMetaData SeamSeriesMetaData::parse(const QFileInfo &seamSeriesDir)
{
    return parse(QDir{seamSeriesDir.absoluteFilePath()});
}

SeamSeriesMetaData SeamSeriesMetaData::parse(const QDir &seamSeriesDir)
{
    return parse(seamSeriesDir.absoluteFilePath(QStringLiteral("metadata.json")));
}

SeamSeriesMetaData SeamSeriesMetaData::parse(const QString &absoluteFilePath)
{
    QFile metaDataFile(absoluteFilePath);
    if (!metaDataFile.open(QIODevice::ReadOnly))
    {
        return {};
    }

    return parse(QJsonDocument::fromJson(metaDataFile.readAll()).object());
}

SeamSeriesMetaData SeamSeriesMetaData::parse(const QJsonObject& jsonObject)
{
    SeamSeriesMetaData metaData;

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

    it = jsonObject.find(QLatin1String("processedSeams"));
    if (it != jsonObject.end())
    {
        const auto jsonArray = it.value().toArray();
        metaData.m_seams.reserve(jsonArray.size());
        std::transform(jsonArray.begin(), jsonArray.end(), std::back_inserter(metaData.m_seams), [] (const auto& value) { return SeamMetaData::parse(value.toObject()); });
        metaData.m_seamsValid = true;
    }

    return metaData;
}

}
}
