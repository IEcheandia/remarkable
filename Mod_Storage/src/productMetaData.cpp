#include "productMetaData.h"
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

ProductMetaData ProductMetaData::parse(const QFileInfo &productInstanceDir)
{
    return parse(QDir{productInstanceDir.absoluteFilePath()});
}

ProductMetaData ProductMetaData::parse(const QDir &productInstanceDir)
{
    return parse(productInstanceDir.absoluteFilePath(QStringLiteral("metadata.json")));
}

ProductMetaData ProductMetaData::parse(const QString &absoluteFilePath)
{
    QFile metaDataFile(absoluteFilePath);
    if (!metaDataFile.open(QIODevice::ReadOnly))
    {
        return {};
    }
    ProductMetaData metaData;
    metaData.m_filePath = absoluteFilePath;

    const auto jsonObject = QJsonDocument::fromJson(metaDataFile.readAll()).object();
    auto it = jsonObject.find(QLatin1String("uuid"));
    if (it != jsonObject.end())
    {
        metaData.m_uuid = QUuid::fromString(it.value().toString());
        metaData.m_uuidValid = true;
    }
    it = jsonObject.find(QLatin1String("serialNumber"));
    if (it != jsonObject.end())
    {
        metaData.m_number = it.value().toInt();
        metaData.m_numberValid = true;
    }
    it = jsonObject.find(QLatin1String("extendedProductInfo"));
    if (it != jsonObject.end())
    {
        metaData.m_extendedProductInfo = it.value().toString();
    }
    it = jsonObject.find(QLatin1String("date"));
    if (it != jsonObject.end())
    {
        metaData.m_date = QDateTime::fromString(it.value().toString(), Qt::ISODateWithMs);
        metaData.m_dateValid = metaData.m_date.isValid();
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

    it = jsonObject.find(QLatin1String("processedSeamSeries"));
    if (it != jsonObject.end())
    {
        const auto jsonArray = it.value().toArray();
        metaData.m_seamSeries.reserve(jsonArray.size());
        std::transform(jsonArray.begin(), jsonArray.end(), std::back_inserter(metaData.m_seamSeries), [] (const auto& value) { return SeamSeriesMetaData::parse(value.toObject()); });
        metaData.m_seamSeriesValid = true;
    }

    it = jsonObject.find(QLatin1String("processedSeams"));
    if (it != jsonObject.end())
    {
        const auto jsonArray = it.value().toArray();
        metaData.m_seams.reserve(jsonArray.size());
        std::transform(jsonArray.begin(), jsonArray.end(), std::back_inserter(metaData.m_seams), [] (const auto& value) { return SeamMetaData::parse(value.toObject()); });
        metaData.m_seamsValid = true;
    }

    it = jsonObject.find(QLatin1String("productUuid"));
    if (it != jsonObject.end())
    {
        if (const auto id = QUuid::fromString(it.value().toString()); !id.isNull())
        {
            metaData.m_productUuid = id;
        }
    }

    it = jsonObject.find(QLatin1String("productName"));
    if (it != jsonObject.end())
    {
        metaData.m_productName = it.value().toString();
    }

    it = jsonObject.find(QLatin1String("productType"));
    if (it != jsonObject.end())
    {
        metaData.m_productType = it.value().toInt();
    }

    return metaData;
}

}
}
