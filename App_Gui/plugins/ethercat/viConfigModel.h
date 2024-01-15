#pragma once

#include "viConfigService.h"

#include <QAbstractListModel>
#include <QColor>
#include <QVector2D>
#include <common/ethercat.h>

class QMutex;
class QTimer;

namespace precitec
{
namespace gui
{
class ServiceToGuiServer;

namespace components
{

namespace plotter
{
class DataSet;
}

namespace ethercat
{

/**
 * Model containing all signals extracted from ViConfig.
 * In addition it can create DataSets for enabled signals and fills them with data from ServiceToGuiServer.
 * Use @link{toggleEnabled} to add or remove a DataSet for a signal.
 *
 * The model provides the following roles:
 * @li name
 * @li enabled (whether there is a DataSet in dataSets for this row)
 * @li color (the color of the DataSet or defaultDataSetColor if the row is not enabled)
 **/
class ViConfigModel : public QAbstractListModel
{
    Q_OBJECT
    /**
     * ViConfigService needed to extract the signals
     **/
    Q_PROPERTY(precitec::gui::components::ethercat::ViConfigService *viConfig READ viConfig WRITE setViConfig NOTIFY viConfigChanged)
    /**
     * ServiceToGuiServer needed to get the binary data to fill the DataSets
     **/
    Q_PROPERTY(precitec::gui::ServiceToGuiServer *service READ service WRITE setService NOTIFY serviceChanged)
    /**
     * The currently enabled datasets
     **/
    Q_PROPERTY(QVariantList dataSets READ dataSets NOTIFY dataSetsChanged)
    /**
     * Whether items are enabled, that is dataSets contain elements
     **/
    Q_PROPERTY(bool itemsEnabled READ areItemsEnabled NOTIFY dataSetsChanged)
    /**
     * The default color for a newly enabled DataSet.
     **/
    Q_PROPERTY(QColor defaultDataSetColor READ defaultDataSetColor WRITE setDefaultDataSetColor NOTIFY defaultDataSetColorChanged)
    /**
     * Whether the ViConfigModel is recording data for the enabled signals
     **/
    Q_PROPERTY(bool recording READ isRecording NOTIFY recordingChanged)
    /**
     * Whether the system is currently processing the recorded data. As this is heavily I/O bound the UI should be blocked to not
     * interact with the ViConfigModel while processing.
     **/
    Q_PROPERTY(bool processing READ isProcessing NOTIFY processingChanged)
    /**
     * Directory for storing the recording.
     **/
    Q_PROPERTY(QString storageDir READ storageDir WRITE setStorageDir NOTIFY storageDirChanged)
public:
    ViConfigModel(QObject *parent = nullptr);
    ~ViConfigModel() override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = {}) const override;
    QHash<int, QByteArray> roleNames() const override;

    ViConfigService *viConfig() const
    {
        return m_viConfig;
    }
    void setViConfig(ViConfigService *service);

    ServiceToGuiServer *service() const
    {
        return m_service;
    }
    void setService(ServiceToGuiServer *service);

    QColor defaultDataSetColor() const
    {
        return m_defaultColor;
    }
    void setDefaultDataSetColor(const QColor &color);

    Q_INVOKABLE void toggleEnabled(const QModelIndex &index);
    /**
     * Sets the @p color for @p index. Requires a DataSet
     **/
    Q_INVOKABLE void setColor(const QModelIndex &index, const QColor &color);

    QVariantList dataSets() const;

    bool isRecording() const
    {
        return m_recording;
    }
    Q_INVOKABLE void startRecording();
    Q_INVOKABLE void stopRecording();

    bool areItemsEnabled() const
    {
        return !m_dataSets.empty();
    }

    bool isProcessing() const
    {
        return m_processing;
    }

    QString storageDir() const
    {
        return m_storageDir;
    }
    void setStorageDir(const QString &dir);

    /**
     * Clears the DataSets and resets the timestamp counter.
     * Non-functional if currently recording.
     **/
    Q_INVOKABLE void clearDataSets();

Q_SIGNALS:
    void viConfigChanged();
    void dataSetsChanged();
    void serviceChanged();
    void defaultDataSetColorChanged();
    void recordingChanged();
    void processingChanged();
    void storageDirChanged();
    /**
     * internal signal
     **/
    void samplesChanged();

private:
    void initSignals();
    void deleteDataSets();
    void initSlaves();
    void fetchData();
    void addQueuedSamples();
    ViConfigService *m_viConfig = nullptr;
    QMetaObject::Connection m_viConfigDestroyed;
    QMetaObject::Connection m_signalsChanged;
    std::vector<ViConfigService::Signal> m_signals;
    std::map<int, std::tuple<plotter::DataSet*, std::vector<QVector2D>, std::vector<QVector2D>>> m_dataSets;
    ServiceToGuiServer *m_service = nullptr;
    QMetaObject::Connection m_serviceDestroyed;
    QMetaObject::Connection m_serviceInfo;
    QMetaObject::Connection m_serviceData;
    std::vector<std::pair<EC_T_GET_SLAVE_INFO, std::vector<int>>> m_slaveInfos;
    quint32 m_timestamp = 0;
    QColor m_defaultColor{Qt::black};
    std::unique_ptr<QMutex> m_fetchDataMutex;
    bool m_recording = false;
    bool m_processing = false;
    QString m_storageDir;
    QTimer *m_recordingTimer = nullptr;
};

}
}
}
}

Q_DECLARE_METATYPE(precitec::gui::components::ethercat::ViConfigModel*)
