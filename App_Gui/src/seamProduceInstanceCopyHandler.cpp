#include "seamProduceInstanceCopyHandler.h"

#include <QDir>
#include <QFutureWatcher>
#include <QtConcurrent>
#include <QXmlStreamWriter>
#include <QFile>
#include "videoRecorder/literal.h"

using namespace precitec::vdr;
namespace precitec::gui
{

SeamProduceInstanceCopyHandler::SeamProduceInstanceCopyHandler(QObject *parent) : QObject(parent)
{
}

void SeamProduceInstanceCopyHandler::setSource(const SeamProduceInstanceCopyHandler::SeamProduceInstanceInfo &source)
{
    m_source = source;
}

void SeamProduceInstanceCopyHandler::setTarget(const SeamProduceInstanceCopyHandler::SeamProduceInstanceInfo &target)
{
    m_target = target;
}

void SeamProduceInstanceCopyHandler::copy()
{
    const auto sourceSeamProductInstancePath = checkedSourceSeamProductInstanceDirectory();
    const auto targetSeamProductInstancePath = checkedTargetSeamProductInstanceDirectory();
    if (sourceSeamProductInstancePath.isEmpty() || targetSeamProductInstancePath.isEmpty())
    {
        return;
    }
    copySeamProductInstance(sourceSeamProductInstancePath, targetSeamProductInstancePath);
}

QString SeamProduceInstanceCopyHandler::checkedSourceSeamProductInstanceDirectory()
{
    if (!QDir(m_source.productInstanceDirectory).exists())
    {
        return {};
    }

    if (QString::number(m_source.seamSeriesNumber).length() > 4 ||
        QString::number(m_source.seamNumber).length() > 4)
    {
        return {};
    }
    const QDir sourceSeamProductInstanceDirectory{QStringLiteral("%1/%2%3/%4%5/")
                       .arg(QDir(m_source.productInstanceDirectory).path())
                       .arg(QString::fromStdString(g_oSeamSeriesDirName))
                       .arg(m_source.seamSeriesNumber, 4, 10, QLatin1Char('0'))
                       .arg(QString::fromStdString(g_oSeamDirName))
                       .arg(m_source.seamNumber, 4, 10, QLatin1Char('0'))};

    if (!sourceSeamProductInstanceDirectory.exists())
    {
        return {};
    }
    auto metaFileName = QString::fromStdString(g_oSequenceInfoFile) + "." + QString::fromStdString(g_oXmlExtension);
    if (!QFile::exists(sourceSeamProductInstanceDirectory.path() + "/" + metaFileName))
    {
        return {};
    }

    return sourceSeamProductInstanceDirectory.path();
}

QString SeamProduceInstanceCopyHandler::checkedTargetSeamProductInstanceDirectory()
{
    if (!QDir(m_target.productInstanceDirectory).exists())
    {
        return {};
    }

    if (QString::number(m_target.seamSeriesNumber).length() > 4 ||
        QString::number(m_target.seamNumber).length() > 4)
    {
        return {};
    }

    return QStringLiteral("%1/%2%3/%4%5").arg(QDir(m_target.productInstanceDirectory).path())
        .arg(QString::fromStdString(g_oSeamSeriesDirName))
        .arg(m_target.seamSeriesNumber, 4, 10, QLatin1Char('0'))
        .arg(QString::fromStdString(g_oSeamDirName))
        .arg(m_target.seamNumber, 4, 10, QLatin1Char('0'));
}

void SeamProduceInstanceCopyHandler::copySeamProductInstance(const QString sourceSeamProductInstancePath,
                                                             const QString targetSeamProductInstancePath)
{
    if (sourceSeamProductInstancePath == targetSeamProductInstancePath)
    {
        return;
    }

    if (!removeDirectory(targetSeamProductInstancePath))
    {
        return;
    }

    QDir parentTargetDir(QFileInfo(targetSeamProductInstancePath).path());
    if (!parentTargetDir.mkpath(QFileInfo(targetSeamProductInstancePath).fileName()))
    {
        return;
    }

    QDir sourceDir(sourceSeamProductInstancePath);
    foreach (const QFileInfo &info, sourceDir.entryInfoList({"*." + QString::fromStdString(g_oImageExtension)}, QDir::Files | QDir::NoDotAndDotDot))
    {
        QString sourceItemPath = sourceSeamProductInstancePath + "/" + info.fileName();
        QString targetItemPath = targetSeamProductInstancePath + "/" + info.fileName();
        if (info.isFile())
        {
            if (!QFile::copy(sourceItemPath, targetItemPath))
            {
                return;
            }
        }
    }
    createTargetMetaFileFromSource(sourceSeamProductInstancePath, targetSeamProductInstancePath);
}

bool SeamProduceInstanceCopyHandler::removeDirectory(const QString &directoryPath)
{
    QDir dir(directoryPath);
    if (!dir.exists())
    {
        return true;
    }

    foreach (const QFileInfo &info, dir.entryInfoList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot))
    {
        if (info.isDir())
        {
            if (!removeDirectory(info.filePath()))
            {
                return false;
            }
        }
        else
        {
            if (!dir.remove(info.fileName()))
            {
                return false;
            }
        }
    }
    QDir parentDir(QFileInfo(directoryPath).path());
    return parentDir.rmdir(QFileInfo(directoryPath).fileName());
}

void SeamProduceInstanceCopyHandler::createTargetMetaFileFromSource(const QString &sourcePath,
                                                                    const QString &targetPath)
{
    auto metaFileName = QString::fromStdString(g_oSequenceInfoFile) + "." + QString::fromStdString(g_oXmlExtension);
    QFile sourceMetaFile(sourcePath + "/" + metaFileName);
    QFile targetMetaFile(targetPath + "/" + metaFileName);

    if (!sourceMetaFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        return;
    }

    if (targetMetaFile.exists())
    {
        return;
    }

    if (!targetMetaFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        return;
    }

    QXmlStreamReader xmlReader;
    QXmlStreamWriter xmlWriter;
    xmlReader.setDevice(&sourceMetaFile);
    xmlWriter.setDevice(&targetMetaFile);
    xmlReader.readNextStartElement();

    if (auto videoRecorder = QString::fromStdString(g_oConfigFileName);
        xmlReader.name().compare(videoRecorder))
    {
        xmlWriter.setAutoFormatting(true);
        xmlWriter.writeStartElement(videoRecorder);
    }
    else
    {
        return;
    }

    while (!xmlReader.atEnd())
    {
        xmlReader.readNext();
        if (xmlReader.isStartElement())
        {
            if (auto dataTime = QString::fromStdString(g_oTagDateTime);
                xmlReader.name().compare(dataTime))
            {
                xmlWriter.writeTextElement(dataTime, xmlReader.readElementText());
                continue;
            }

            if (auto isLiveMode= QString::fromStdString(g_oTagIsLiveMode);
                xmlReader.name().compare(isLiveMode))
            {
                xmlWriter.writeTextElement(isLiveMode, xmlReader.readElementText());
                continue;
            }

            if (auto productName = QString::fromStdString(g_oTagProductName);
                xmlReader.name().compare(productName))
            {
                xmlWriter.writeTextElement(productName, m_target.productName);
                continue;
            }

            if (auto productInstance = QString::fromStdString(g_oTagProductInst); xmlReader.name().compare(productInstance))
            {
                xmlWriter.writeTextElement(productInstance,
                                           m_target.productInstanceUuid.toString(QUuid::WithoutBraces));
                continue;
            }

            if (auto productUuid = QString::fromStdString(g_oTagProductUUID);
                xmlReader.name().compare(productUuid))
            {
                xmlWriter.writeTextElement(productUuid, m_target.productUuid.toString(QUuid::WithoutBraces));
                continue;
            }

            if (auto productType = QString::fromStdString(g_oTagProductType);
                xmlReader.name().compare(productType))
            {
                xmlWriter.writeTextElement(productType, QString::number(m_target.productType));
                continue;
            }

            if (auto productNumber = QString::fromStdString(g_oTagProductNumber); xmlReader.name().compare(productNumber))
            {
                // the name looks strange ... but it preserves compatability
                xmlWriter.writeTextElement(productNumber, QString::number(m_target.serialNumber));
                continue;
            }

            if (auto seamSeries = QString::fromStdString(g_oTagSeamSeries); xmlReader.name().compare(seamSeries))
            {
                xmlWriter.writeTextElement(seamSeries, QString::number(m_target.seamSeriesNumber));
                continue;
            }

            if (auto seam = QString::fromStdString(g_oTagSeam); xmlReader.name().compare(seam))
            {
                xmlWriter.writeTextElement(seam, QString::number(m_target.seamNumber));
                continue;
            }

            if (auto triggerDelta = QString::fromStdString(g_oTagTriggerDelta);
                xmlReader.name().compare(triggerDelta))
            {
                xmlWriter.writeTextElement(triggerDelta, xmlReader.readElementText());
                continue;
            }

            if (auto nbImages = QString::fromStdString(g_oTagNbImages);
                xmlReader.name().compare(nbImages))
            {
                xmlWriter.writeTextElement(nbImages, xmlReader.readElementText());
                continue;
            }

            if (auto nbSamples = QString::fromStdString(g_oTagNbSamples);
                xmlReader.name().compare(nbSamples))
            {
                xmlWriter.writeTextElement(nbSamples, xmlReader.readElementText());
                continue;
            }
        }
    }

    xmlWriter.writeEndElement(); // Close tag "video_recorder"
    sourceMetaFile.close();
    targetMetaFile.close();
}

SeamProduceInstanceCopyHandler::~SeamProduceInstanceCopyHandler()
{
}

}