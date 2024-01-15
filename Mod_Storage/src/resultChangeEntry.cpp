#include "resultChangeEntry.h"

using precitec::gui::components::userLog::Change;

namespace precitec
{
namespace storage
{

ResultChangeEntry::ResultChangeEntry(QObject *parent)
    : Change(parent)
{
}

ResultChangeEntry::ResultChangeEntry(ResultSetting *setting, const ResultSetting::Type &change, const QVariant &oldValue, const QVariant &newValue, QObject *parent)
    : Change(parent)
    , m_result(setting)
    , m_change(change)
    , m_oldValue(oldValue)
    , m_newValue(newValue)
{
    setMessage(tr("Result configuration changed"));
}

ResultChangeEntry::~ResultChangeEntry() = default;

QJsonObject ResultChangeEntry::data() const
{
    return {
        qMakePair(QStringLiteral("result"), m_result->toJson()),
        qMakePair(QStringLiteral("change"), int(m_change)),
        qMakePair(QStringLiteral("oldValue"), QJsonValue::fromVariant(m_oldValue)),
        qMakePair(QStringLiteral("newValue"), QJsonValue::fromVariant(m_newValue))
    };
}

void ResultChangeEntry::initFromJson(const QJsonObject &data)
{
    auto it = data.find(QStringLiteral("result"));
    if (it != data.end())
    {
        m_result = ResultSetting::fromJson(it.value().toObject(), this);
    }
    it = data.find(QStringLiteral("change"));
    if (it != data.end())
    {
        m_change = ResultSetting::Type(it.value().toInt());
    }
    it = data.find(QStringLiteral("oldValue"));
    if (it != data.end())
    {
        m_oldValue = it.value().toVariant();
    }
    it = data.find(QStringLiteral("newValue"));
    if (it != data.end())
    {
        m_newValue = it.value().toVariant();
    }
}

QUrl ResultChangeEntry::detailVisualization() const
{
    return QStringLiteral("qrc:///resources/qml/userLog/ResultChangeEntry.qml");
}

}
}
