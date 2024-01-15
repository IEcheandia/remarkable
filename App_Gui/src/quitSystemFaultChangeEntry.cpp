#include "quitSystemFaultChangeEntry.h"

namespace precitec
{
namespace gui
{

QuitSystemFaultChangeEntry::QuitSystemFaultChangeEntry(QObject *parent)
    : components::userLog::Change(parent)
{
}


QuitSystemFaultChangeEntry::QuitSystemFaultChangeEntry(const QString &station, QObject *parent)
    : components::userLog::Change(parent)
    , m_station(station)
{
    setMessage(tr("Quit system fault"));
}

QuitSystemFaultChangeEntry::~QuitSystemFaultChangeEntry() = default;

QJsonObject QuitSystemFaultChangeEntry::data() const
{
    return {qMakePair(QStringLiteral("station"), m_station)};
}

void QuitSystemFaultChangeEntry::initFromJson(const QJsonObject &data)
{
    auto it = data.find(QStringLiteral("station"));
    if (it != data.end())
    {
        m_station = (*it).toString();
    }
}

QUrl QuitSystemFaultChangeEntry::detailVisualization() const
{
    return QStringLiteral("qrc:///resources/qml/userLog/QuitSystemFaultChangeEntry.qml");
}

}
}
