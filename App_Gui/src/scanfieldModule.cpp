#include "scanfieldModule.h"
#include "deviceProxyWrapper.h"
#include "weldmasterPaths.h"
#include "scanfieldImageToJpgConverter.h"

#include "util/ScanFieldImageParameters.h"

#include <QDir>
#include <QRect>
#include <QPoint>
#include <QSettings>
#include <QFutureWatcher>
#include <QtConcurrentRun>

using precitec::interface::Configuration;
using precitec::calibration::ScanFieldImageParameters;

namespace precitec
{
namespace gui
{
namespace
{

template <typename T>
T getValue(const Configuration& configuration, const std::string& key, T defaultValue)
{
    auto it = std::find_if(configuration.begin(), configuration.end(), [key] (auto kv) { return kv->key() == key; });
    if (it == configuration.end())
    {
        return defaultValue;
    }
    return (*it)->template value<T>();
}
}

ScanfieldModule::ScanfieldModule(QObject* parent)
    : QObject(parent)
{
    connect(this, &ScanfieldModule::seriesChanged, this, &ScanfieldModule::sourceImageDirChanged);
    connect(this, &ScanfieldModule::sourceImageDirChanged, this, &ScanfieldModule::loadCalibration);

    connect(this, &ScanfieldModule::grabberDeviceProxyChanged, this,
        [this]
        {
            if (!m_grabberDeviceProxy)
            {
                setCameraSize({1280, 1024});
                return;
            }
            setLoading(true);
            auto grabberDeviceProxy = m_grabberDeviceProxy;
            QFutureWatcher<Configuration>* watcher = new QFutureWatcher<Configuration>(this);
            connect(watcher, &QFutureWatcher<Configuration>::finished, this,
                [this, watcher]
                {
                    watcher->deleteLater();
                    auto configuration = watcher->result();

                    setCameraSize({getValue(configuration, std::string("Window.WMax"), 1280), getValue(configuration, std::string("Window.HMax"), 1024)});
                    setLoading(false);
                }
            );
            watcher->setFuture(QtConcurrent::run(
                [grabberDeviceProxy]
                {
                    if (!grabberDeviceProxy)
                    {
                        return Configuration{};
                    }
                    return grabberDeviceProxy->deviceProxy()->get();
                })
            );
        }
    );
}

ScanfieldModule::~ScanfieldModule() = default;

void ScanfieldModule::setCalibrationCoordinatesRequestProxy(const CalibrationCoordinatesRequestProxy& proxy)
{
    if (m_calibrationCoordinatesRequestProxy == proxy)
    {
        return;
    }
    m_calibrationCoordinatesRequestProxy = proxy;
    emit calibrationCoordinatesRequestProxyChanged();
}

void ScanfieldModule::setGrabberDeviceProxy(DeviceProxyWrapper* device)
{
    if (m_grabberDeviceProxy == device)
    {
        return;
    }

    disconnect(m_grabberDeviceDestroyConnection);

    m_grabberDeviceProxy = device;

    if (m_grabberDeviceProxy)
    {
        m_grabberDeviceDestroyConnection = connect(m_grabberDeviceProxy, &QObject::destroyed, this, std::bind(&ScanfieldModule::setGrabberDeviceProxy, this, nullptr));
    } else
    {
        m_grabberDeviceDestroyConnection = {};
    }

    emit grabberDeviceProxyChanged();
}

void ScanfieldModule::setImageSize(const QSize& size)
{
    if (m_imageSize == size)
    {
        return;
    }
    m_imageSize = size;
    emit imageSizeChanged();
}

bool ScanfieldModule::centerValid(const QPointF& point) const
{
    return point.x() >= 0.5 * m_cameraSize.width()
        && point.y() >= 0.5 * m_cameraSize.height()
        && point.x() <= m_imageSize.width() - 0.5 * m_cameraSize.width()
        && point.y() <= m_imageSize.height() - 0.5 * m_cameraSize.height();
}

QRectF ScanfieldModule::cameraRect(const QPointF& point) const
{
    return {toValidCameraCenter(point) - QPointF(0.5 * m_cameraSize.width(), 0.5 * m_cameraSize.height()), m_cameraSize};
}

void ScanfieldModule::setSeries(const QUuid& id)
{
    if (m_series == id)
    {
        return;
    }
    m_series = id;
    emit seriesChanged();
}

QString ScanfieldModule::sourceImageDir() const
{
    if (m_series.isNull())
    {
        return QLatin1String("");
    }
    return WeldmasterPaths::instance()->scanFieldDir() + (m_series.toString(QUuid::WithoutBraces).append(QStringLiteral("/ScanFieldImage.jpg")));
}

void ScanfieldModule::loadCalibration()
{
    if (m_series.isNull())
    {
        clearConfiguration();
        return;
    }
    loadCalibrationFile(WeldmasterPaths::instance()->scanFieldDir() + (m_series.toString(QUuid::WithoutBraces)));
}

void ScanfieldModule::loadCalibrationFile(const QString& pathToSource)
{
    if (pathToSource.isEmpty())
    {
        clearConfiguration();
        return;
    }
    const auto& sourceDir = QDir{pathToSource};
    const auto& fileName = QStringLiteral("ScanFieldImage.ini");
    m_calibrationConfig.clear();
    if (sourceDir.exists(fileName))
    {
        QSettings uiSettings{sourceDir.absoluteFilePath(fileName), QSettings::IniFormat};
        setImageSize(QSize{uiSettings.value(QStringLiteral("ScanFieldImageWidth"), 0).toInt(), uiSettings.value(QStringLiteral("ScanFieldImageHeight"), 0).toInt()});

        ScanFieldImageParameters defaultScanFieldImageParameters;
        for (auto keyValue : defaultScanFieldImageParameters.makeConfiguration())
        {
            auto key = QString::fromStdString(keyValue->key());
            if (!uiSettings.contains(key))
            {
                // incomplete configuration
                m_calibrationConfig.clear();
                break;
            }
            auto value = uiSettings.value(key);
            switch (keyValue->type())
            {
                case TInt:
                    keyValue->setValue<int>(value.toInt());
                    break;
                case TUInt:
                    keyValue->setValue<uint>(value.toUInt());
                    break;
                case TBool:
                    keyValue->setValue<bool>(value.toBool());
                    break;
                case TFloat:
                    keyValue->setValue<float>(value.toFloat());
                    break;
                case TDouble:
                    keyValue->setValue<double>(value.toDouble());
                    break;
                case TString:
                    keyValue->setValue<std::string>(value.toString().toStdString());
                    break;
                case TChar:
                case TByte:
                default:
                    break;

            }
            m_calibrationConfig.push_back(keyValue);
        }
        ScanfieldImageToJpgConverter{sourceDir}();
    }
    emit configurationValidChanged();
}

QRectF ScanfieldModule::mirrorRect(const QRectF& rect) const
{
    //recompute a ROI (expressed as relative coordinate inside an image of size cameraSize) according to mirroring parameters
    auto mirroredRoi = rect;

    auto mirrorX = false;
    auto mirrorY = false;

    if (configurationValid())
    {
        const auto& data = ScanFieldImageParameters::loadConfiguration(m_calibrationConfig);
        mirrorX = data.m_ScanMasterData.m_mirrorX;
        mirrorY = data.m_ScanMasterData.m_mirrorY;
    }

    if (mirrorX)
    {
        mirroredRoi.moveRight(m_cameraSize.width() - rect.left());
    }

    if (mirrorY)
    {
        mirroredRoi.moveBottom(m_cameraSize.height() - rect.top());
    }

    return mirroredRoi.normalized();
}

QPointF ScanfieldModule::scannerToImageCoordinates(double x, double y) const
{
    if (m_calibrationCoordinatesRequestProxy && configurationValid())
    {
        const auto& coords = m_calibrationCoordinatesRequestProxy->getScanFieldImagePositionFromScannerPosition(x, y, m_calibrationConfig);
        return {coords.x, coords.y};
    }
    return {-1, -1};
}

QPointF ScanfieldModule::imageToScannerCoordinates(double x, double y) const
{
    if (m_calibrationCoordinatesRequestProxy && configurationValid())
    {
        const auto& coords = m_calibrationCoordinatesRequestProxy->getScannerPositionFromScanFieldImage(x, y, m_calibrationConfig);
        return {coords.x, coords.y};
    }
    return {-1, -1};
}

QPointF ScanfieldModule::toValidCameraCenter(const QPointF& point) const
{
    return {
        qBound(0.0 + 0.5 * m_cameraSize.width(), point.x(), m_imageSize.width() - 0.5 * m_cameraSize.width()),
        qBound(0.0 + 0.5 * m_cameraSize.height(), point.y(), m_imageSize.height() - 0.5 * m_cameraSize.height())
    };
}

void ScanfieldModule::setCameraSize(const QSize& size)
{
    if (m_cameraSize == size)
    {
        return;
    }
    m_cameraSize = size;
    emit cameraSizeChanged();
}

void ScanfieldModule::clearConfiguration()
{
    if (configurationValid())
    {
        m_calibrationConfig.clear();
        emit configurationValidChanged();
    }
}

void ScanfieldModule::setLoading(bool loading)
{
    if (m_loading == loading)
    {
        return;
    }
    m_loading = loading;
    emit loadingChanged();
}

void ScanfieldModule::copyFromOtherSeries(const QUuid& otherSeries)
{
    if (m_series.isNull() || otherSeries.isNull())
    {
        return;
    }
    const QDir dir{WeldmasterPaths::instance()->scanFieldDir() + (otherSeries.toString(QUuid::WithoutBraces))};
    const QDir targetDir{WeldmasterPaths::instance()->scanFieldDir() + (m_series.toString(QUuid::WithoutBraces))};
    if (!targetDir.mkpath(targetDir.absolutePath()))
    {
        return;
    }

    const auto& files = dir.entryInfoList(QDir::Files);
    for (const auto& file : files)
    {
        QFile::copy(file.absoluteFilePath(), targetDir.absoluteFilePath(file.fileName()));
    }
    loadCalibration();
}

}
}
