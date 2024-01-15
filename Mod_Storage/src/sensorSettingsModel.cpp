#include "sensorSettingsModel.h"
#include "event/sensor.h"
#include "jsonSupport.h"

#include <QDir>
#include <QJsonObject>
#include <QJsonDocument>
#include <QSaveFile>
#include <QColor>

namespace precitec
{
namespace storage
{

static const std::map<int, std::tuple<std::string, QColor>> s_sensors {
    {precitec::interface::eWeldHeadAxisXPos, {QT_TR_NOOP("X Axis position"), QColor{QStringLiteral("darkslateblue")}}},
    {precitec::interface::eWeldHeadAxisYPos, {QT_TR_NOOP("Y Axis position"), QColor{QStringLiteral("darkslategray")}}},
    {precitec::interface::eWeldHeadAxisZPos, {QT_TR_NOOP("Z Axis position"), QColor{QStringLiteral("darkturquoise")}}},
    {precitec::interface::eGlasNotPresent, {QT_TR_NOOP("Glas Not Present"), QColor{QStringLiteral("blueviolet")}}},
    {precitec::interface::eGlasDirty, {QT_TR_NOOP("Glas Dirty"), QColor{QStringLiteral("cadetblue")}}},
    {precitec::interface::eTempGlasFail, {QT_TR_NOOP("Temp Glas Fail"), QColor{QStringLiteral("darksalmon")}}},
    {precitec::interface::eTempHeadFail, {QT_TR_NOOP("Temp Head Fail"), QColor{QStringLiteral("darkseagreen")}}},
    {precitec::interface::eLaserPowerSignal, {QT_TR_NOOP("Laser Power"), QColor{QStringLiteral("darkcyan")}}},
    {precitec::interface::eEncoderInput1, {QT_TR_NOOP("Encoder 1"), QColor{QStringLiteral("blue")}}},
    {precitec::interface::eEncoderInput2, {QT_TR_NOOP("Encoder 2"), QColor{QStringLiteral("aqua")}}},
    {precitec::interface::eRobotTrackSpeed, {QT_TR_NOOP("Robot tracking speed"), QColor{QStringLiteral("darkred")}}},
    {precitec::interface::eOversamplingSignal1, {QT_TR_NOOP("Oversampling signal 1"), QColor{QStringLiteral("darkgoldenrod")}}},
    {precitec::interface::eOversamplingSignal2, {QT_TR_NOOP("Oversampling signal 2"), QColor{QStringLiteral("darkgreen")}}},
    {precitec::interface::eOversamplingSignal3, {QT_TR_NOOP("Oversampling signal 3"), QColor{QStringLiteral("darkmagenta")}}},
    {precitec::interface::eOversamplingSignal4, {QT_TR_NOOP("Oversampling signal 4"), QColor{QStringLiteral("darkolivegreen")}}},
    {precitec::interface::eOversamplingSignal5, {QT_TR_NOOP("Oversampling signal 5"), QColor{QStringLiteral("darkorange")}}},
    {precitec::interface::eOversamplingSignal6, {QT_TR_NOOP("Oversampling signal 6"), QColor{QStringLiteral("darkorchid")}}},
    {precitec::interface::eOversamplingSignal7, {QT_TR_NOOP("Oversampling signal 7"), QColor{255, 30, 30}}},
    {precitec::interface::eOversamplingSignal8, {QT_TR_NOOP("Oversampling signal 8"), QColor{255, 50, 50}}},
    {precitec::interface::eGenPurposeDigIn1, {QT_TR_NOOP("General purpose digital in 1"), QColor{255, 70, 70}}},
    {precitec::interface::eGenPurposeDigIn2, {QT_TR_NOOP("General purpose digital in 2"), QColor{245, 80, 80}}},
    {precitec::interface::eGenPurposeDigIn3, {QT_TR_NOOP("General purpose digital in 3"), QColor{235, 90, 90}}},
    {precitec::interface::eGenPurposeDigIn4, {QT_TR_NOOP("General purpose digital in 4"), QColor{225, 100, 100}}},
    {precitec::interface::eGenPurposeDigIn5, {QT_TR_NOOP("General purpose digital in 5"), QColor{215, 110, 110}}},
    {precitec::interface::eGenPurposeDigIn6, {QT_TR_NOOP("General purpose digital in 6"), QColor{205, 120, 120}}},
    {precitec::interface::eGenPurposeDigIn7, {QT_TR_NOOP("General purpose digital in 7"), QColor{195, 130, 130}}},
    {precitec::interface::eGenPurposeDigIn8, {QT_TR_NOOP("General purpose digital in 8"), QColor{185, 140, 140}}},
    {precitec::interface::eGenPurposeAnalogIn1, {QT_TR_NOOP("General purpose analog in 1"), QColor{QStringLiteral("darkturquoise")}}},
    {precitec::interface::eIDMWeldingDepth, {QT_TR_NOOP("IDM welding depth"), QColor{29, 62, 121}}},
    {precitec::interface::eIDMTrackingLine, {QT_TR_NOOP("IDM tracking line"), QColor{29, 62, 121}}},
    {precitec::interface::eIDMOverlayImage, {QT_TR_NOOP("IDM overlay image"), QColor{29, 62, 121}}},
    {precitec::interface::eIDMSpectrumLine, {QT_TR_NOOP("IDM spectrum line"), QColor{29, 62, 121}}},
    {precitec::interface::eScannerXPosition, {QT_TR_NOOP("Scanner X Position"), QColor{20, 120, 160}}},
    {precitec::interface::eScannerYPosition, {QT_TR_NOOP("Scanner Y Position"), QColor{30, 60, 255}}},
    {precitec::interface::eLWM40_1_Plasma, {QT_TR_NOOP("LWM Plasma"), QColor{QStringLiteral("mediumblue")}}},
    {precitec::interface::eLWM40_1_Temperature, {QT_TR_NOOP("LWM Temperature"), QColor{QStringLiteral("mediumorchid")}}},
    {precitec::interface::eLWM40_1_BackReflection, {QT_TR_NOOP("LWM Back Reflection"), QColor{QStringLiteral("mediumseagreen")}}},
    {precitec::interface::eLWM40_1_AnalogInput, {QT_TR_NOOP("LWM Laser Power Monitor"),  QColor{QStringLiteral("mediumturquoise")}}},
    {precitec::interface::eGenPurposeAnalogIn2, {QT_TR_NOOP("General purpose analog in 2"), QColor{245, 80, 80}}},
    {precitec::interface::eGenPurposeAnalogIn3, {QT_TR_NOOP("General purpose analog in 3"), QColor{235, 90, 90}}},
    {precitec::interface::eGenPurposeAnalogIn4, {QT_TR_NOOP("General purpose analog in 4"), QColor{225, 100, 100}}},
    {precitec::interface::eGenPurposeAnalogIn5, {QT_TR_NOOP("General purpose analog in 5"), QColor{215, 110, 110}}},
    {precitec::interface::eGenPurposeAnalogIn6, {QT_TR_NOOP("General purpose analog in 6"), QColor{205, 120, 120}}},
    {precitec::interface::eGenPurposeAnalogIn7, {QT_TR_NOOP("General purpose analog in 7"), QColor{195, 130, 130}}},
    {precitec::interface::eGenPurposeAnalogIn8, {QT_TR_NOOP("General purpose analog in 8"), QColor{185, 140, 140}}},
    {precitec::interface::eS6K_Leading_Result1, {QT_TR_NOOP("S6K Result 1 of Leading PostInspect"), QColor{245, 80, 80}}},
    {precitec::interface::eS6K_Leading_Result2, {QT_TR_NOOP("S6K Result 2 of Leading PostInspect"), QColor{235, 90, 90}}},
    {precitec::interface::eS6K_Leading_Result3, {QT_TR_NOOP("S6K Result 3 of Leading PostInspect"), QColor{225, 100, 100}}},
    {precitec::interface::eS6K_Leading_Result4, {QT_TR_NOOP("S6K Result 4 of Leading PostInspect"), QColor{215, 110, 110}}},
    {precitec::interface::eZCPositionDigV1, {QT_TR_NOOP("Z-Collimator Position"), QColor{255, 70, 70}}},
    {precitec::interface::eScannerWeldingFinished, {QT_TR_NOOP("Welding with scanner finished"), QColor{245, 80, 80}}},
    {precitec::interface::eFiberSwitchPosition, {QT_TR_NOOP("Fiber switch position"), QColor{0, 255, 255}}},
    {precitec::interface::eProfileLines, {QT_TR_NOOP("Profile Line (pseudo image)"), QColor{235, 60, 60}}},
    {precitec::interface::eIDMQualityPeak1, {QT_TR_NOOP("IDM quality peak 1"), QColor{QStringLiteral("mediumseagreen")}}},
    {precitec::interface::eContourPrepared, {QT_TR_NOOP("Contour prepared"), QColor{0, 255, 255}}},

};

SensorSettingsModel::SensorSettingsModel(QObject *parent)
    : QAbstractListModel(parent)
{
    connect(this, &SensorSettingsModel::configurationDirectoryChanged, this, &SensorSettingsModel::load);
    connect(this, &SensorSettingsModel::modelReset, this, &SensorSettingsModel::save);
    connect(this, &SensorSettingsModel::rowsInserted, this, &SensorSettingsModel::save);
    connect(this, &SensorSettingsModel::dataChanged, this, &SensorSettingsModel::save);
}

SensorSettingsModel::~SensorSettingsModel()
{
}

int SensorSettingsModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
    {
        return 0;
    }
    return m_sensorItems.size();
}

QVariant SensorSettingsModel::data(const QModelIndex &index, int role) const
{
    int row = index.row();
    if (row < 0 || row >= (int)m_sensorItems.size())
    {
        return {};
    }

    const auto &sensor = m_sensorItems.at(index.row());
    switch (role)
    {
        case Qt::DisplayRole:
            return sensor->enumType();
        case Qt::UserRole:
            return sensor->uuid();
        case Qt::UserRole + 1:
            return sensor->name();
        case Qt::UserRole + 2:
            return sensor->plotterNumber();
        case Qt::UserRole + 3:
            return sensor->plottable();
        case Qt::UserRole + 4:
            return sensor->min();
        case Qt::UserRole + 5:
            return sensor->max();
        case Qt::UserRole + 6:
            return sensor->lineColor();
        case Qt::UserRole + 7:
            return sensor->visibleItem();
        case Qt::UserRole + 8:
            return QVariant::fromValue(sensor->visualization());
        case Qt::UserRole + 9:
            return sensor->disabled();
        case Qt::UserRole + 10:
            return QColor(sensor->lineColor()).hslHueF();
        case Qt::UserRole + 11:
            return QColor(sensor->lineColor()).hslSaturationF();
        case Qt::UserRole + 12:
            return QColor(sensor->lineColor()).lightnessF();
    }
    return {};
}

QHash<int, QByteArray> SensorSettingsModel::roleNames() const
{
    return QHash<int, QByteArray>{
        {Qt::DisplayRole, QByteArrayLiteral("enumType")},
        {Qt::UserRole, QByteArrayLiteral("uuid")},
        {Qt::UserRole + 1, QByteArrayLiteral("name")},
        {Qt::UserRole + 2, QByteArrayLiteral("plotterNumber")},
        {Qt::UserRole + 3, QByteArrayLiteral("plottable")},
        {Qt::UserRole + 4, QByteArrayLiteral("min")},
        {Qt::UserRole + 5, QByteArrayLiteral("max")},
        {Qt::UserRole + 6, QByteArrayLiteral("lineColor")},
        {Qt::UserRole + 7, QByteArrayLiteral("visibleItem")},
        {Qt::UserRole + 8, QByteArrayLiteral("visualization")},
        {Qt::UserRole + 9, QByteArrayLiteral("disabled")},
        {Qt::UserRole + 10, QByteArrayLiteral("hue")},
        {Qt::UserRole + 11, QByteArrayLiteral("saturation")},
        {Qt::UserRole + 12, QByteArrayLiteral("lightness")}
    };
}

void SensorSettingsModel::setConfigurationDirectory(const QString& dir)
{
    if (m_configurationDirectory.compare(dir) == 0)
    {
        return;
    }
    m_configurationDirectory = dir;
    emit configurationDirectoryChanged();
}

void SensorSettingsModel::load()
{
    const auto dir = QDir{m_configurationDirectory};

    if (!dir.exists())
    {
        return;
    }

    const auto filePath = dir.filePath(QStringLiteral("sensorConfig.json"));

    if (!QFileInfo::exists(filePath))
    {
        createDefaultSettings();
        return;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly))
    {
        return;
    }

    const auto data = file.readAll();
    file.close();
    if (data.isEmpty())
    {
       return;
    }

    QJsonParseError error;
    const auto document = QJsonDocument::fromJson(data, &error);
    if (document.isNull())
    {
        return;
    }

    beginResetModel();
    qDeleteAll(m_sensorItems);
    m_sensorItems = json::parseResultItems(document.object(), this);
    endResetModel();

    addMissingItems();
}

void SensorSettingsModel::createDefaultSettings()
{
    beginResetModel();

    for (auto sensor : s_sensors)
    {
        const auto type = sensor.first;
        const auto name = QString::fromStdString(std::get<std::string>(sensor.second));
        const auto color = std::get<QColor>(sensor.second);

        addItem(type, name, color);
    }

    endResetModel();
}

void SensorSettingsModel::save()
{
    const auto filePath = QDir{m_configurationDirectory}.filePath(QStringLiteral("sensorConfig.json"));

    QSaveFile file(filePath);
    if (!file.open(QIODevice::WriteOnly))
    {
        return;
    }

    const QJsonDocument document({
        json::toJson(m_sensorItems)
    });

    file.write(document.toJson());
    file.commit();
}

void SensorSettingsModel::updateValue(const QModelIndex& modelIndex, const QVariant &data, ResultSetting::Type target)
{
    int index = modelIndex.row();
    if (std::size_t(index) >= m_sensorItems.size())
    {
        return;
    }
    const auto &sensor = m_sensorItems.at(index);
    if (sensor)
    {
        sensor->updateValue(data, target);
        emit dataChanged(modelIndex, modelIndex, {});
    }
}

QModelIndex SensorSettingsModel::indexForResultType(int enumType) const
{
    auto it = std::find_if(m_sensorItems.begin(), m_sensorItems.end(), [enumType] (auto item) { return item->enumType() == enumType; });
    if (it == m_sensorItems.end())
    {
        return {};
    }
    return index(std::distance(m_sensorItems.begin(), it), 0);
}

ResultSetting *SensorSettingsModel::getItem(int enumType) const
{
    for (auto it = m_sensorItems.begin(); it != m_sensorItems.end(); ++it)
    {
        if ((*it)->enumType() == enumType)
        {
            return (*it);
        }
    }
    return nullptr;
}

ResultSetting *SensorSettingsModel::checkAndAddItem(int enumType, const QString& name, const QColor& color)
{
    for (auto it = m_sensorItems.begin(); it != m_sensorItems.end(); ++it)
    {
        if ((*it)->enumType() == enumType)
        {
            return (*it);
        }
    }

    beginInsertRows(QModelIndex(), m_sensorItems.size(), m_sensorItems.size());
    auto newValue = addItem(enumType, name, color);
    endInsertRows();

    return newValue;
}

ResultSetting *SensorSettingsModel::addItem(int enumType, const QString& name, const QColor& color)
{
    auto newValue = new ResultSetting(QUuid::createUuid(), enumType, this);
    newValue->setName(name);
    newValue->setMin(-1000);
    newValue->setMax(1000);
    newValue->setPlottable(1);
    newValue->setPlotterNumber(1);
    newValue->setLineColor(color.name());
    newValue->setVisibleItem(1);

    m_sensorItems.push_back(newValue);

    return newValue;
}

void SensorSettingsModel::addMissingItems()
{
    for (auto sensor : s_sensors)
    {
        const auto type = sensor.first;
        const auto name = QString::fromStdString(std::get<std::string>(sensor.second));
        const auto color = std::get<QColor>(sensor.second);

        checkAndAddItem(type, name, color);
    }
}

QString SensorSettingsModel::sensorName(int sensorId)
{
    auto it = s_sensors.find(sensorId);
    if (it != s_sensors.end())
    {
        return QString::fromStdString(std::get<std::string>(it->second));
    } else
    {
        return QString::number(sensorId);
    }
}

QColor SensorSettingsModel::sensorColor(int sensorId)
{
    auto it = s_sensors.find(sensorId);
    if (it != s_sensors.end())
    {
        return std::get<QColor>(it->second);
    } else
    {
        return {29, 62, 121};
    }
}

void SensorSettingsModel::ensureItemExists(int enumType)
{
    checkAndAddItem(enumType, QString::number(enumType), Qt::black);
}

}
}
