#pragma once

#include <QObject>
#include <QDateTime>
#include <QPointer>
#include <QUuid>
#include <QTemporaryDir>

#include <list>
#include <map>
#include <memory>

class QFileSystemWatcher;

namespace precitec
{

namespace storage
{

/**
 * The CalibrationModel holds the current calibration configuration
 **/
class CalibrationChangeNotifier : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString directory READ directory WRITE setDirectory NOTIFY directoryChanged)
    

public:
    explicit CalibrationChangeNotifier (QObject *parent = nullptr);
    ~CalibrationChangeNotifier() override = default;

    /**
     * Sets the @p directory where results are stored
     **/
    void setDirectory(const QString &directory);

    /**
     * @returns the directory where results are stored.
     **/
    QString directory() const
    {
        return m_directory;
    }
    

Q_SIGNALS:
    void directoryChanged();
    void calibrationData0Changed();

private:
    void updateMonitor();

    QString m_directory;
    QFileSystemWatcher *m_monitor;
    
};

}
}
