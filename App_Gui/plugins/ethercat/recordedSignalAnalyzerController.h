#pragma once

#include <QDateTime>
#include <QObject>
#include <QVariant>

#include <vector>

class QJsonDocument;

namespace precitec
{
namespace gui
{
namespace components
{

namespace plotter
{
class DataSet;
}

namespace ethercat
{

/**
 * This controller can load the data stored by ViConfigModel and provide the loaded
 * data as DataSets.
 **/
class RecordedSignalAnalyzerController : public QObject
{
    Q_OBJECT
    /**
     * Directory for reading the recording.
     **/
    Q_PROPERTY(QString storageDir READ storageDir WRITE setStorageDir NOTIFY storageDirChanged)
    /**
     * The date of the recording hold by this controller.
     **/
    Q_PROPERTY(QDateTime recordingDate READ recordingDate NOTIFY dataChanged)
    /**
     * List of data sets
     **/
    Q_PROPERTY(QVariantList dataSets READ dataSets NOTIFY dataChanged)
    /**
     * Whether the controller contains data, that is dataSets is not empty
     **/
    Q_PROPERTY(bool hasData READ hasData NOTIFY dataChanged)
    /**
     * Whether the controller is currently loading data
     **/
    Q_PROPERTY(bool loading READ isLoading NOTIFY loadingChanged)
    /**
     * The recommended range of the values in x direction
     **/
    Q_PROPERTY(qreal xRange READ xRange NOTIFY dataChanged)
public:
    RecordedSignalAnalyzerController(QObject *parent = nullptr);
    ~RecordedSignalAnalyzerController();

    QString storageDir() const
    {
        return m_storageDir;
    }
    void setStorageDir(const QString &storageDir);

    QVariantList dataSets() const;

    QDateTime recordingDate() const
    {
        return m_recordingDate;
    }

    bool isLoading() const
    {
        return m_loading;
    }

    bool hasData() const
    {
        return !m_dataSets.empty();
    }

    float xRange() const;

    /**
     * Loads the data. This can be used to reload the data asynchronously when other parts (e.g. ViConfigModel)
     * know that the storage data changed.
     **/
    Q_INVOKABLE void load();

Q_SIGNALS:
    void storageDirChanged();
    /**
     * Signal emitted whenever the date hold by this controller changes
     **/
    void dataChanged();
    void loadingChanged();

private:
    void parseJson(const QJsonDocument &document);
    QString m_storageDir;
    QDateTime m_recordingDate;
    std::vector<precitec::gui::components::plotter::DataSet*> m_dataSets;
    bool m_loading = false;
};

}
}
}
}
