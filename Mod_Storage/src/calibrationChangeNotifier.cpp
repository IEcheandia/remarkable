#include "calibrationChangeNotifier.h"
#include "common/calibrationConfiguration.h"
#include "math/calibrationCommon.h"  //sensorId

#include <QFileSystemWatcher>
#include <iostream>

namespace precitec
{
namespace storage
{
    
CalibrationChangeNotifier::CalibrationChangeNotifier(QObject* parent)
: QObject(parent)
, m_monitor(new QFileSystemWatcher{this})
{
    connect(m_monitor, &QFileSystemWatcher::directoryChanged, this, &CalibrationChangeNotifier::calibrationData0Changed);
    connect(m_monitor, &QFileSystemWatcher::fileChanged, this, &CalibrationChangeNotifier::calibrationData0Changed);
    
    connect(m_monitor, &QFileSystemWatcher::directoryChanged, this, [] () {std::cout << "Calibration Directory changed" << std::endl;});
    connect(m_monitor, &QFileSystemWatcher::fileChanged, this,[] () {std::cout << "Calibration File changed" << std::endl;});
}


void CalibrationChangeNotifier::updateMonitor()
{
    // first remove all existing paths
    const auto directories = m_monitor->directories();
    if (!directories.isEmpty())
    {
        m_monitor->removePaths(directories);
    }
    const auto files = m_monitor->files();
    if (!directories.isEmpty())
    {
        m_monitor->removePaths(files);
    }
    
    if (m_directory.isEmpty())
    {
        return;
    }
    
    //see files used for calibration in CalibrationManager::getOSCalibrationDataFromHW, CalibrateIbOpticalSystem::getCamGridDataFromCamera 
    coordinates::CalibrationConfiguration oFilenamesConfiguration(math::eSensorId0, m_directory.toStdString());
    //monitor the calibration data file
    m_monitor->addPath(QString::fromStdString(oFilenamesConfiguration.getConfigFilename()));
    //monitor the cache file, but since it does not always exist, we need to monitor the containing folder
    //NB: the cache file is in the folder calib, because it is not meant for backup
    QString oCalibFolder = QString::fromStdString(oFilenamesConfiguration.getCalibFolder());
    QDir{}.mkpath(oCalibFolder);
    m_monitor->addPath(oCalibFolder);
    assert(QString::fromStdString(oFilenamesConfiguration.getCamGridDataBinaryFilename()).startsWith(oCalibFolder) && "File will not be monitored");
    //no need to monitor SystemConfig.xml for a change is Type_of_Sensor,  because when it's modified a reboot is expected
    //the fallback file it's not monitored neither, because it's not modified after the system startup (reboot after changes in system configuration 
    // or backup restore)
}

    
void CalibrationChangeNotifier::setDirectory ( const QString& directory )
{
    if ( m_directory != directory ) 
    {
        m_directory = directory;
        updateMonitor();
        emit directoryChanged();
    }
}

}
}
