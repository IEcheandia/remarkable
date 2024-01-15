#include "deviceKeyValueChangeEntry.h"

namespace precitec
{
namespace gui
{

DeviceKeyValueChangeEntry::DeviceKeyValueChangeEntry(QObject *parent)
    : components::userLog::Change(parent)
{
}

DeviceKeyValueChangeEntry::DeviceKeyValueChangeEntry(const QUuid &device, const precitec::interface::SmpKeyValue &kv, QObject *parent)
    : components::userLog::Change(parent)
    , m_device(device)
    , m_key(QString::fromStdString(kv->key()))
{
    setMessage(tr("Device key value changed"));
    switch (kv->type())
    {
    case TChar:
        m_value = kv->value<char>();
        break;
    case TByte:
        m_value = kv->value<byte>();
        break;
    case TInt:
        m_value = kv->value<int>();
        break;
    case TUInt:
        m_value = kv->value<uint>();
        break;
    case TBool:
        m_value = kv->value<bool>();
        break;
    case TFloat:
        m_value = kv->value<float>();
        break;
    case TDouble:
        m_value = kv->value<double>();
        break;
    case TString:
        m_value = QString::fromStdString(kv->value<std::string>());
        break;
    default:
        m_value = QStringLiteral("undefined");
    }
}

DeviceKeyValueChangeEntry::~DeviceKeyValueChangeEntry() = default;

QJsonObject DeviceKeyValueChangeEntry::data() const
{
    return {{
        qMakePair(QStringLiteral("device"), m_device.toString()),
        qMakePair(QStringLiteral("key"), m_key),
        qMakePair(QStringLiteral("value"), QJsonValue::fromVariant(m_value))
    }};
}

void DeviceKeyValueChangeEntry::initFromJson(const QJsonObject &data)
{
    auto it = data.find(QStringLiteral("device"));
    if (it != data.end())
    {
        m_device = QUuid{(*it).toString()};
    }
    it = data.find(QStringLiteral("key"));
    if (it != data.end())
    {
        m_key = (*it).toString();
    }
    it = data.find(QStringLiteral("value"));
    if (it != data.end())
    {
        m_value = (*it).toVariant();
    }
}

QUrl DeviceKeyValueChangeEntry::detailVisualization() const
{
    return QStringLiteral("qrc:///resources/qml/userLog/DeviceKeyValueChangeEntry.qml");
}

}
}
