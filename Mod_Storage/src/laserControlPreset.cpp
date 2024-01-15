#include "laserControlPreset.h"
#include "jsonSupport.h"
#include "precitec/dataSet.h"

#include <QJsonObject>
#include <QJsonDocument>
#include <QDir>
#include <QPointF>

using precitec::gui::components::plotter::DataSet;

namespace precitec
{
namespace storage
{

LaserControlPreset::LaserControlPreset(QObject *parent)
    : QObject(parent)
    , m_uuid(QUuid::createUuid())
    , m_power(std::vector<int>(m_numberOfSamples, 0))
    , m_offset(std::vector<int>(m_numberOfSamples, 0))
    , m_channel1Power(new DataSet(this))
    , m_channel2Power(new DataSet(this))
{
    m_channel1Power->setColor("magenta");
    m_channel1Power->setDrawingMode(DataSet::DrawingMode::Line);
    m_channel1Power->setDrawingOrder(DataSet::DrawingOrder::OnBottom);

    m_channel2Power->setColor("limegreen");
    m_channel2Power->setDrawingMode(DataSet::DrawingMode::Line);
    m_channel2Power->setDrawingOrder(DataSet::DrawingOrder::OnBottom);

    connect(this, &LaserControlPreset::powerChanged, [this] (int powerIndex) {
        if (powerIndex >= 0 && powerIndex < 4)
        {
            computeChannel1Samples();
        }
        if (powerIndex >= 4 && powerIndex < 8)
        {
            computeChannel2Samples();
        }
    });

    computeChannel1Samples();
    computeChannel2Samples();
}

LaserControlPreset::~LaserControlPreset() = default;

void LaserControlPreset::setName(const QString& name)
{
    if (m_name == name)
    {
        return;
    }
    m_name = name;
    setChange(true);
    emit nameChanged();
}

void LaserControlPreset::setPower(uint index, int power)
{
    power = qBound(0, power, 100);
    if (m_power.at(index) == power)
    {
        return;
    }
    m_power.at(index) = power;
    setChange(true);
    emit powerChanged(index);
}

void LaserControlPreset::setPower(const std::vector<int>& power)
{
    for (auto i = 0u; i < m_numberOfSamples; i++)
    {
        setPower(i, i < power.size() ? power.at(i) : 0);
    }
}

void LaserControlPreset::setOffset(uint index, int offset)
{
    offset = qBound(0, offset, 100);
    if (m_offset.at(index) == offset)
    {
        return;
    }
    m_offset.at(index) = offset;
    setChange(true);
    emit offsetChanged(index);
}

void LaserControlPreset::setOffset(const std::vector<int>& offset)
{
    for (auto i = 0u; i < m_numberOfSamples; i++)
    {
        setOffset(i, i < offset.size() ? offset.at(i) : 0);
    }
}

void LaserControlPreset::setEnabled(int enabled)
{
    if (m_enabled == enabled)
    {
        return;
    }
    if (m_state != State::Default)
    {
        return;
    }
    m_enabled = enabled;

    m_channel1Power->setColor(m_enabled ? "magenta" : "gray");
    m_channel2Power->setColor(m_enabled ? "limegreen" : "gray");

    emit enabledChanged();
}

void LaserControlPreset::setChange(const bool set)
{
    if (m_hasChanges == set)
    {
        return;
    }
    m_hasChanges = set;
    emit markAsChanged();
}

void LaserControlPreset::setState(State state)
{
    if (m_state == state)
    {
        return;
    }

    if (m_state != State::Default)
    {
        emit editStopped();
    }

    m_state = state;
    emit stateChanged();

    if (m_state != State::Default)
    {
        emit editStarted();
    }
}

void LaserControlPreset::setFilePath(const QString& path)
{
    m_filePath = path;
}

int LaserControlPreset::getValue(Key key) const
{
    switch (key)
    {
        case Key::LC_Parameter_No3:
        case Key::LC_Parameter_No4:
        case Key::LC_Parameter_No5:
        case Key::LC_Parameter_No6:
            return m_power.at(int(key));
        case Key::LC_Parameter_No11:
        case Key::LC_Parameter_No12:
        case Key::LC_Parameter_No13:
        case Key::LC_Parameter_No14:
            return m_offset.at(int(key) - 4);
        case Key::LC_Parameter_No16:
        case Key::LC_Parameter_No17:
        case Key::LC_Parameter_No18:
        case Key::LC_Parameter_No19:
            return m_power.at(int(key) - 4);
        case Key::LC_Parameter_No24:
        case Key::LC_Parameter_No25:
        case Key::LC_Parameter_No26:
        case Key::LC_Parameter_No27:
            return m_offset.at(int(key) - 8);
        default:
            Q_UNREACHABLE();
            break;
    }
}

void LaserControlPreset::setValue(Key key, int value)
{
    switch (key)
    {
        case Key::LC_Parameter_No3:
        case Key::LC_Parameter_No4:
        case Key::LC_Parameter_No5:
        case Key::LC_Parameter_No6:
            setPower(int(key), value);
            break;
        case Key::LC_Parameter_No11:
        case Key::LC_Parameter_No12:
        case Key::LC_Parameter_No13:
        case Key::LC_Parameter_No14:
            setOffset(int(key) - 4, value);
            break;
        case Key::LC_Parameter_No16:
        case Key::LC_Parameter_No17:
        case Key::LC_Parameter_No18:
        case Key::LC_Parameter_No19:
            setPower(int(key) - 4, value);
            break;
        case Key::LC_Parameter_No24:
        case Key::LC_Parameter_No25:
        case Key::LC_Parameter_No26:
        case Key::LC_Parameter_No27:
            setOffset(int(key) - 8, value);
            break;
        default:
            Q_UNREACHABLE();
            break;
    }
}

LaserControlPreset *LaserControlPreset::load(const QString &filePath, QObject* parent)
{
    QFile presetFile(filePath);
    if (!presetFile.exists())
    {
        return nullptr;
    }
    if (!presetFile.open(QIODevice::ReadOnly))
    {
        return nullptr;
    }

    const QByteArray data = presetFile.readAll();
    if (data.isEmpty())
    {
        return nullptr;
    }

    const auto document = QJsonDocument::fromJson(data);
    if (document.isNull())
    {
        return nullptr;
    }

    const auto object = document.object();
    if (object.isEmpty())
    {
        return nullptr;
    }

    auto uuid = json::parseUuid(object);
    if (uuid.isNull())
    {
        uuid = QUuid::createUuid();
    }

    LaserControlPreset *preset = new LaserControlPreset(parent);
    preset->m_uuid = uuid;
    preset->m_filePath = filePath;
    preset->setName(json::parseName(object));
    preset->setPower(json::parsePower(object));
    preset->setOffset(json::parseOffset(object));
    preset->setChange(false);

    return preset;
}

bool LaserControlPreset::deleteFile(const QString& filePath)
{
    QFile presetFile(filePath);

    return presetFile.remove();
}

void LaserControlPreset::save()
{
    QFile presetFile(m_filePath);
    if (!presetFile.open(QIODevice::WriteOnly))
    {
        return;
    }

    const auto object = QJsonObject{
        {
            json::toJson(m_uuid),
            json::nameToJson(m_name),
            json::powerToJson(m_power),
            json::offsetToJson(m_offset)
        }
    };

    presetFile.write(QJsonDocument{object}.toJson());

    setChange(false);
    setState(State::Default);
}

void LaserControlPreset::save(const QString &filePath)
{
    m_filePath = filePath;
    save();
}

void LaserControlPreset::restore()
{
    QFile presetFile(m_filePath);
    if (!presetFile.exists())
    {
        return;
    }
    if (!presetFile.open(QIODevice::ReadOnly))
    {
        return;
    }

    const QByteArray data = presetFile.readAll();
    if (data.isEmpty())
    {
        return;
    }

    const auto document = QJsonDocument::fromJson(data);
    if (document.isNull())
    {
        return;
    }

    const auto object = document.object();
    if (object.isEmpty())
    {
        return;
    }

    setName(json::parseName(object));
    setPower(json::parsePower(object));
    setOffset(json::parseOffset(object));

    setChange(false);
    setState(State::Default);
}

void LaserControlPreset::computeChannel1Samples()
{
    m_channel1Power->clear();
    m_channel1Power->addSample(QVector2D{0.0f, 0.01f * (m_power.at(3) * 2.0f) -1.0f});
    m_channel1Power->addSample(QVector2D{25.0f, 0.01f * (m_power.at(0) * 2.0f) -1.0f});
    m_channel1Power->addSample(QVector2D{50.0f, 0.01f * (m_power.at(1) * 2.0f) -1.0f});
    m_channel1Power->addSample(QVector2D{75.0f, 0.01f * (m_power.at(2) * 2.0f) -1.0f});
    m_channel1Power->addSample(QVector2D{100.0f, 0.01f * (m_power.at(3) * 2.0f) -1.0f});
    m_channel1Power->addSample(QVector2D{125.0f, 0.01f * (m_power.at(0) * 2.0f) -1.0f});
    m_channel1Power->addSample(QVector2D{150.0f, 0.01f * (m_power.at(1) * 2.0f) -1.0f});

    emit channel1SamplesChanged();
}

void LaserControlPreset::computeChannel2Samples()
{
    m_channel2Power->clear();
    m_channel2Power->addSample(QVector2D{0.0f,   0.01f * (m_power.at(7) * 2.0f) -1.0f});
    m_channel2Power->addSample(QVector2D{25.0f,  0.01f * (m_power.at(4) * 2.0f) -1.0f});
    m_channel2Power->addSample(QVector2D{50.0f,  0.01f * (m_power.at(5) * 2.0f) -1.0f});
    m_channel2Power->addSample(QVector2D{75.0f,  0.01f * (m_power.at(6) * 2.0f) -1.0f});
    m_channel2Power->addSample(QVector2D{100.0f, 0.01f * (m_power.at(7) * 2.0f) -1.0f});
    m_channel2Power->addSample(QVector2D{125.0f, 0.01f * (m_power.at(4) * 2.0f) -1.0f});
    m_channel2Power->addSample(QVector2D{150.0f, 0.01f * (m_power.at(5) * 2.0f) -1.0f});

    emit channel2SamplesChanged();
}

}
}
