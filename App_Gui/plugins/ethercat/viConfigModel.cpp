#include "viConfigModel.h"
#include "gatewayModel.h"
#include "../../src/serviceToGuiServer.h"
#include "event/ethercatDefines.h"

#include <precitec/dataSet.h>

#include <QDateTime>
#include <QDir>
#include <QFutureWatcher>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QMutex>
#include <QSaveFile>
#include <QtConcurrentRun>
#include <QTimer>
#include <bitset>
#include <chrono>

using precitec::gui::components::plotter::DataSet;
using namespace std::chrono_literals;

namespace precitec
{
namespace gui
{
namespace components
{
namespace ethercat
{

ViConfigModel::ViConfigModel(QObject *parent)
    : QAbstractListModel(parent)
    , m_fetchDataMutex(std::make_unique<QMutex>())
{
    connect(this, &ViConfigModel::viConfigChanged, this, &ViConfigModel::initSignals);
    connect(this, &ViConfigModel::modelReset, this, &ViConfigModel::dataSetsChanged);
    connect(this, &ViConfigModel::dataChanged, this,
        [this] (const QModelIndex &topLeft, const QModelIndex & bottomLeft, const QVector<int> &roles)
        {
            Q_UNUSED(topLeft)
            Q_UNUSED(bottomLeft)
            if (roles.empty() || std::find(roles.begin(), roles.end(), Qt::UserRole) != roles.end())
            {
                emit dataSetsChanged();
            }
        }
    );
    connect(this, &ViConfigModel::serviceChanged, this, &ViConfigModel::initSlaves);
    connect(this, &ViConfigModel::dataSetsChanged, this, &ViConfigModel::initSlaves);
    connect(this, &ViConfigModel::defaultDataSetColorChanged, this,
        [this]
        {
            if (rowCount() == 0)
            {
                return;
            }
            emit dataChanged(index(0, 0), index(rowCount() - 1, 0), {Qt::UserRole + 1});
        }
    );
    connect(this, &ViConfigModel::samplesChanged, this, &ViConfigModel::addQueuedSamples, Qt::QueuedConnection);
}

ViConfigModel::~ViConfigModel() = default;

QHash<int, QByteArray> ViConfigModel::roleNames() const
{
    return {
        {Qt::DisplayRole, QByteArrayLiteral("name")},
        {Qt::UserRole, QByteArrayLiteral("enabled")},
        {Qt::UserRole + 1, QByteArrayLiteral("color")}
    };
}

int ViConfigModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
    {
        return 0;
    }
    return m_signals.size();
}

QVariant ViConfigModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
    {
        return {};
    }

    const auto &signal = m_signals.at(index.row());
    switch (role)
    {
    case Qt::DisplayRole:
        return GatewayModel::localizeSignal(signal, 0, true);
    case Qt::UserRole:
        return m_dataSets.find(index.row()) != m_dataSets.end();
    case Qt::UserRole + 1:
    {
        auto it = m_dataSets.find(index.row());
        if (it != m_dataSets.end())
        {
            return std::get<0>((*it).second)->color();
        }
        return m_defaultColor;
    }
    case Qt::UserRole + 2:
        return QVariant::fromValue(signal.type);
    }
    return {};
}

void ViConfigModel::setViConfig(ViConfigService *service)
{
    if (m_viConfig == service)
    {
        return;
    }
    m_viConfig = service;
    disconnect(m_viConfigDestroyed);
    disconnect(m_signalsChanged);
    if (m_viConfig)
    {
        m_viConfigDestroyed = connect(m_viConfig, &ViConfigService::destroyed, this, std::bind(&ViConfigModel::setViConfig, this, nullptr));
        m_signalsChanged = connect(m_viConfig, &ViConfigService::signalsChanged, this, &ViConfigModel::initSignals);
    } else
    {
        m_viConfigDestroyed = {};
        m_signalsChanged = {};
    }
    emit viConfigChanged();
}

void ViConfigModel::initSignals()
{
    if (!m_viConfig)
    {
        if (m_signals.empty())
        {
            return;
        }
        beginResetModel();
        {
            QMutexLocker lock{m_fetchDataMutex.get()};
            m_signals.clear();
            deleteDataSets();
        }
        endResetModel();
        return;
    }
    const auto &sigs = m_viConfig->allSignals();
    if (sigs.empty() && m_signals.empty())
    {
        return;
    }
    beginResetModel();
    {
        QMutexLocker lock{m_fetchDataMutex.get()};
        m_signals = sigs;
        deleteDataSets();
    }
    endResetModel();
}

void ViConfigModel::deleteDataSets()
{
    for (auto it = m_dataSets.begin(); it != m_dataSets.end(); it++)
    {
        std::get<0>((*it).second)->deleteLater();
    }
    m_dataSets.clear();
}

void ViConfigModel::toggleEnabled(const QModelIndex &index)
{
    if (!index.isValid())
    {
        return;
    }
    QMutexLocker lock{m_fetchDataMutex.get()};
    auto it = m_dataSets.find(index.row());
    if (it == m_dataSets.end())
    {
        auto emplaced = m_dataSets.emplace(index.row(), std::make_tuple(new DataSet{this}, std::vector<QVector2D>{}, std::vector<QVector2D>{}));
        std::get<0>(emplaced.first->second)->setName(index.data().toString());
        std::get<0>(emplaced.first->second)->setSmooth(false);
        std::get<0>(emplaced.first->second)->setColor(m_defaultColor);
    } else
    {
        std::get<0>((*it).second)->deleteLater();
        m_dataSets.erase(it);
    }
    lock.unlock();
    emit dataChanged(index, index, {Qt::UserRole});
}

QVariantList ViConfigModel::dataSets() const
{
    QVariantList ret;
    std::transform(m_dataSets.begin(), m_dataSets.end(), std::back_inserter(ret), [] (auto element) { return QVariant::fromValue(std::get<0>(element.second)); });
    return ret;
}

void ViConfigModel::setService(ServiceToGuiServer *service)
{
    if (m_service == service)
    {
        return;
    }
    m_service = service;
    disconnect(m_serviceDestroyed);
    disconnect(m_serviceInfo);
    disconnect(m_serviceData);
    if (m_service)
    {
        m_serviceInfo = connect(m_service, &ServiceToGuiServer::slaveInfoChanged, this, &ViConfigModel::initSlaves, Qt::QueuedConnection);
        m_serviceDestroyed = connect(m_service, &QObject::destroyed, this, std::bind(&ViConfigModel::setService, this, nullptr));
        m_serviceData = connect(m_service, &ServiceToGuiServer::processDataChanged, this, &ViConfigModel::fetchData, Qt::DirectConnection);
    } else
    {
        m_serviceDestroyed = {};
        m_serviceInfo = {};
        m_serviceData = {};
    }
    emit serviceChanged();
}

bool operator==(const EC_T_GET_SLAVE_INFO &a, const EC_T_GET_SLAVE_INFO &b)
{
    return a.dwVendorId == b.dwVendorId &&
           a.dwProductCode == b.dwProductCode &&
           a.wCfgPhyAddress == b.wCfgPhyAddress;
}

void ViConfigModel::initSlaves()
{
    QMutexLocker lock{m_fetchDataMutex.get()};
    m_slaveInfos.clear();
    if (!m_service)
    {
        return;
    }
    auto slaves = m_service->slaveInfo();
    for (const auto &data : m_dataSets)
    {
        const auto &signal = m_signals.at(data.first);
        auto it = slaves.begin();
        for (quint32 i = 0; i < signal.instance && it != slaves.end(); i++)
        {
            it = std::find_if(it, slaves.end(), [signal] (auto &slave) {if (slave.dwVendorId == VENDORID_HILSCHER) slave.dwVendorId = VENDORID_HMS;
                                                                        if (slave.dwProductCode == PRODUCTCODE_FIELDBUS) slave.dwProductCode = PRODUCTCODE_ANYBUS_GW;
                                                                        return slave.dwVendorId == signal.vendorId && slave.dwProductCode == signal.productCode; });
            if (i == signal.instance -1)
            {
                break;
            }
            if (it != slaves.end())
            {
                std::advance(it, 1);
            }
        }
        if (it == slaves.end())
        {
            continue;
        }
        auto slaveIt = std::find_if(m_slaveInfos.begin(), m_slaveInfos.end(), [it] (const auto &foo) { return *it == foo.first; });
        if (slaveIt == m_slaveInfos.end())
        {
            m_slaveInfos.emplace_back(*it, std::vector<int>{data.first});
        } else
        {
            slaveIt->second.push_back(data.first);
        }
    }
}

namespace
{
template <typename T>
T sampleFromBytes(const QByteArray &data, quint32 startByte)
{
    T value;
    std::memcpy(&value, data.constData() + startByte, sizeof value);
    return value;
}

void addSamples(const std::vector<QByteArray> &byteData, const ViConfigService::Signal &signal, std::vector<QVector2D> &samples, quint32 timestamp)
{
    samples.reserve(samples.size() + byteData.size());
    for (const auto &data: byteData)
    {
        if (int((signal.startBit + signal.length) / 8) >= data.size())
        {
            continue;
        }
        const quint32 startByte = signal.startBit / 8;
        if (signal.length < 8)
        {
            const auto byte = reinterpret_cast<const uint8_t*>(data.constData())[startByte];

            const std::bitset<8> bits{byte};
            std::bitset<8> result;
            for (std::size_t i = 0; i < signal.length; i++)
            {
                result.set(i, bits.test(signal.startBit - startByte * 8 + i));
            }
            samples.emplace_back(float(timestamp++) / 1000.0f, result.to_ulong());
        } else if (signal.length == 8)
        {
            auto bytes = reinterpret_cast<const uint8_t*>(data.constData())[startByte];
            samples.emplace_back(float(timestamp++) / 1000.0f, bytes);
        } else if (signal.length <= 16)
        {
            samples.emplace_back(float(timestamp++) / 1000.0f, sampleFromBytes<uint16_t>(data, startByte));
        } else if (signal.length <= 32)
        {
            samples.emplace_back(float(timestamp++) / 1000.0f, sampleFromBytes<uint32_t>(data, startByte));
        }
        // ignore any fields larger than 32 bits as we cannot represent it as a simple variable
    }
}
}

void ViConfigModel::fetchData()
{
    QMutexLocker lock{m_fetchDataMutex.get()};
    if (m_slaveInfos.empty() || !m_service)
    {
        return;
    }
    std::size_t updateCounter = 0;
    for (const auto &slaveInfo : m_slaveInfos)
    {
        const auto &inputData = m_service->inputData(slaveInfo.first);
        const auto &outputData = m_service->outputData(slaveInfo.first);
        updateCounter = std::max(updateCounter, inputData.size());
        updateCounter = std::max(updateCounter, outputData.size());

        for (int index : slaveInfo.second)
        {
            const auto &signal = m_signals.at(index);
            auto dataSetIt = m_dataSets.find(index);
            if (dataSetIt == m_dataSets.end())
            {
                continue;
            }

            if (signal.type == ViConfigService::SignalType::Input)
            {
                addSamples(inputData, signal, std::get<1>(dataSetIt->second), m_timestamp);
            } else
            {
                addSamples(outputData, signal, std::get<1>(dataSetIt->second), m_timestamp);
            }

        }
    }
    m_timestamp += updateCounter;
    emit samplesChanged();
}

void ViConfigModel::addQueuedSamples()
{
    QMutexLocker lock{m_fetchDataMutex.get()};
    for (auto &data : m_dataSets)
    {
        auto &samples = std::get<1>(data.second);
        std::get<0>(data.second)->addSamples(samples);
        if (m_recording)
        {
            std::move(samples.begin(), samples.end(), std::back_inserter(std::get<2>(data.second)));
        }
        samples.clear();
    }
}

void ViConfigModel::setColor(const QModelIndex &index, const QColor &color)
{
    auto it = m_dataSets.find(index.row());
    if (it != m_dataSets.end())
    {
        std::get<0>((*it).second)->setColor(color);
        emit dataChanged(index, index, {Qt::UserRole + 1});
    }
}

void ViConfigModel::setDefaultDataSetColor(const QColor &color)
{
    if (m_defaultColor == color)
    {
        return;
    }
    m_defaultColor = color;
    emit defaultDataSetColorChanged();
}

void ViConfigModel::stopRecording()
{
    if (!m_recording)
    {
        return;
    }
    m_recording = false;
    m_processing = true;
    if (m_recordingTimer)
    {
        m_recordingTimer->stop();
    }
    emit processingChanged();
    auto watcher = new QFutureWatcher<void>{this};
    connect(watcher, &QFutureWatcher<void>::finished, this,
        [this]
        {
            m_processing = false;
            emit processingChanged();
            emit recordingChanged();
        }
    );
    watcher->setFuture(QtConcurrent::run(
        [this]
        {
            QJsonArray dataSets;
            std::transform(m_dataSets.begin(), m_dataSets.end(), std::back_inserter(dataSets),
                [] (auto &data)
                {
                    QJsonArray samples;
                    std::transform(std::get<2>(data.second).begin(), std::get<2>(data.second).end(), std::back_inserter(samples),
                        [] (const auto &sample)
                        {
                            return sample.y();
                        }
                    );
                    std::get<2>(data.second).clear();
                    auto dataSet = std::get<0>(data.second);
                    return QJsonObject{{
                        qMakePair(QStringLiteral("name"), dataSet->name()),
                        qMakePair(QStringLiteral("color"), QJsonObject{
                            {QStringLiteral("r"), dataSet->color().red()},
                            {QStringLiteral("g"), dataSet->color().green()},
                            {QStringLiteral("b"), dataSet->color().blue()}
                        }),
                        qMakePair(QStringLiteral("samples"), samples)
                    }};
                });

            QSaveFile file{QDir{m_storageDir}.absoluteFilePath(QStringLiteral("signalAnalyzer.json"))};
            if (file.open(QIODevice::WriteOnly))
            {
                const QJsonObject rootObject{{
                    {QStringLiteral("date"), QDateTime::currentDateTime().toString(Qt::ISODate)},
                    {QStringLiteral("samples"), dataSets}
                }};

//                 qDebug()<<QJsonDocument{rootObject};
//                 qDebug()<<QJsonDocument{rootObject}.toJson();
//                 qDebug()<<QCborValue(QJsonDocument{rootObject}.toJson());
//                 qDebug()<<QCborValue(QJsonDocument{rootObject}.toJson()).toCbor();
//                 qDebug()<<qCompress(QCborValue(QJsonDocument{rootObject}.toJson()).toCbor());
//                 qDebug()<<qUncompress(qCompress(QCborValue(QJsonDocument{rootObject}.toJson()).toCbor()));
//                 qDebug()<<QCborValue::fromCbor(qUncompress(qCompress(QCborValue(QJsonDocument{rootObject}.toJson()).toCbor())));
//                 qDebug()<<QCborValue::fromCbor(qUncompress(qCompress(QCborValue(QJsonDocument{rootObject}.toJson()).toCbor()))).toByteArray();
//                 qDebug()<<QJsonDocument::fromJson(QCborValue::fromCbor(qUncompress(qCompress(QCborValue(QJsonDocument{rootObject}.toJson()).toCbor()))).toByteArray());




                file.write(qCompress(QCborValue(QJsonDocument{rootObject}.toJson()).toCbor()));
                file.commit();
            }
        }));
}

void ViConfigModel::startRecording()
{
    if (m_recording)
    {
        return;
    }
    m_recording = true;
    if (!m_recordingTimer)
    {
        m_recordingTimer = new QTimer{this};
        m_recordingTimer->setSingleShot(true);
        m_recordingTimer->setInterval(10min);
        connect(m_recordingTimer, &QTimer::timeout, this, &ViConfigModel::stopRecording);
    }
    m_recordingTimer->start();
    emit recordingChanged();
}

void ViConfigModel::setStorageDir(const QString &dir)
{
    if (m_storageDir == dir)
    {
        return;
    }
    m_storageDir = dir;
    emit storageDirChanged();
}

void ViConfigModel::clearDataSets()
{
    if (m_recording)
    {
        return;
    }
    m_timestamp = 0;
    for (auto data : m_dataSets)
    {
        std::get<DataSet*>(data.second)->clear();
    }
}

}
}
}
}
