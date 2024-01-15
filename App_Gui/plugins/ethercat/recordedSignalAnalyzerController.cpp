#include "recordedSignalAnalyzerController.h"

#include <QDir>
#include <QFutureWatcher>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QPointF>
#include <QtConcurrentRun>
#include <QCborValue>

#include <precitec/dataSet.h>

using precitec::gui::components::plotter::DataSet;

namespace precitec
{
namespace gui
{
namespace components
{
namespace ethercat
{

RecordedSignalAnalyzerController::RecordedSignalAnalyzerController(QObject *parent)
    : QObject(parent)
{
    connect(this, &RecordedSignalAnalyzerController::storageDirChanged, this, &RecordedSignalAnalyzerController::load);
}

RecordedSignalAnalyzerController::~RecordedSignalAnalyzerController() = default;

QVariantList RecordedSignalAnalyzerController::dataSets() const
{
    QVariantList ret;
    std::transform(m_dataSets.begin(), m_dataSets.end(), std::back_inserter(ret), [] (auto element) { return QVariant::fromValue(element); });
    return ret;
}

void RecordedSignalAnalyzerController::setStorageDir(const QString &storageDir)
{
    if (m_storageDir == storageDir)
    {
        return;
    }
    m_storageDir = storageDir;
    emit storageDirChanged();
}

void RecordedSignalAnalyzerController::load()
{
    if (m_loading)
    {
        return;
    }
    m_loading = true;
    emit loadingChanged();
    const auto path{QDir{m_storageDir}.filePath(QStringLiteral("signalAnalyzer.json"))};

    auto watcher = new QFutureWatcher<QJsonDocument>{this};
    connect(watcher, &QFutureWatcher<QJsonDocument>::finished, this,
        [watcher, this]
        {
            watcher->deleteLater();
            parseJson(watcher->result());
        }
    );
    watcher->setFuture(QtConcurrent::run([=] () -> QJsonDocument
        {
            QFile file{path};
            if (!file.open(QIODevice::ReadOnly))
            {
                return {};
            }
            return QJsonDocument::fromJson(QCborValue::fromCbor(qUncompress(file.readAll())).toByteArray());
        }));
}

void RecordedSignalAnalyzerController::parseJson(const QJsonDocument &document)
{
    for (auto dataSet : m_dataSets)
    {
        dataSet->deleteLater();
    }
    m_dataSets.clear();
    const auto &rootObject = document.object();
    m_recordingDate = QDateTime::fromString(rootObject.value(QLatin1String("date")).toString(), Qt::ISODate);

    const auto &dataSets = rootObject.value(QLatin1String("samples")).toArray();
    for (const auto &jsonDatSet : dataSets)
    {
        const auto &object = jsonDatSet.toObject();
        auto dataSet{new DataSet{this}};
        dataSet->setSmooth(false);
        dataSet->setName(object.value(QLatin1String("name")).toString());
        const auto &color = object.value(QLatin1String("color")).toObject();
        const int r = color.value(QLatin1String("r")).toInt();
        const int g = color.value(QLatin1String("g")).toInt();
        const int b = color.value(QLatin1String("b")).toInt();
        dataSet->setColor({r, g, b});
        // parse samples
        auto jsonSamples = object.value(QLatin1String("samples")).toArray();
        std::vector<QVector2D> samples;
        int timestamp = 0;
        std::transform(jsonSamples.begin(), jsonSamples.end(), std::back_inserter(samples), [&timestamp] (const auto &value) { return QVector2D{float(timestamp++) / 1000.0f, float(value.toDouble())}; });
        dataSet->setProperty("maxElements", jsonSamples.size());
        dataSet->addSamples(samples);

        m_dataSets.push_back(dataSet);
    }
    m_loading = false;
    emit loadingChanged();
    emit dataChanged();
}

float RecordedSignalAnalyzerController::xRange() const
{
    if (m_dataSets.empty())
    {
        return 10.0;
    }
    return m_dataSets.front()->maxX();
}

}
}
}
}
