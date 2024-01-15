#include "referenceCurve.h"
#include "abstractMeasureTask.h"
#include "copyMode.h"
#include "jsonSupport.h"

#include <QJsonObject>

namespace precitec
{
namespace storage
{

ReferenceCurve::ReferenceCurve(QObject* parent)
    : QObject(parent)
    , m_uuid(QUuid::createUuid())
    , m_lowerUuid(QUuid::createUuid())
    , m_middleUuid(QUuid::createUuid())
    , m_upperUuid(QUuid::createUuid())
{
    connect(this, &ReferenceCurve::nameChanged, this, &ReferenceCurve::markAsChanged);
    connect(this, &ReferenceCurve::resultTypeChanged, this, &ReferenceCurve::markAsChanged);
    connect(this, &ReferenceCurve::jitterChanged, this, &ReferenceCurve::markAsChanged);
    connect(this, &ReferenceCurve::referenceTypeChanged, this, &ReferenceCurve::markAsChanged);
    connect(this, &ReferenceCurve::resultTypeChanged, this, &ReferenceCurve::markAsChanged);
}

ReferenceCurve::~ReferenceCurve() = default;

void ReferenceCurve::setResultType(int type)
{
    if (m_resultType == type)
    {
        return;
    }
    if (isChangeTracking())
    {
        addChange(std::move(PropertyChange{tr("ResultType"), m_resultType, type}));
    }
    m_resultType = type;
    emit resultTypeChanged();
}

void ReferenceCurve::setName(const QString& name)
{
    if (m_name == name)
    {
        return;
    }
    if (isChangeTracking())
    {
        addChange(std::move(PropertyChange{tr("Name"), m_name, name}));
    }
    m_name = name;
    emit nameChanged();
}

void ReferenceCurve::setJitter(float jitter)
{
    if (m_jitter == jitter)
    {
        return;
    }
    if (isChangeTracking())
    {
        addChange(std::move(PropertyChange{tr("Jitter"), m_jitter, jitter}));
    }
    m_jitter = jitter;
    emit jitterChanged();
}

void ReferenceCurve::setReferenceType(ReferenceType type)
{
    if (m_referenceType == type)
    {
        return;
    }
    if (isChangeTracking())
    {
        addChange(std::move(PropertyChange{tr("Reference Type"), typeToString(m_referenceType), typeToString(type)}));
    }
    m_referenceType = type;
    emit referenceTypeChanged();
}

void ReferenceCurve::setMeasureTask(AbstractMeasureTask* task)
{
    if (m_task == task)
    {
        return;
    }
    m_task = task;

    emit measureTaskChanged();
}

bool ReferenceCurve::isChangeTracking() const
{
    if (m_task)
    {
        return m_task->isChangeTracking();
    }
    return false;
}

void ReferenceCurve::addChange(ChangeTracker&& change)
{
    m_changeTracker.emplace_back(std::move(change));
}

QJsonArray ReferenceCurve::changes() const
{
    QJsonArray changes;
    std::transform(m_changeTracker.begin(), m_changeTracker.end(), std::back_inserter(changes), [] (const ChangeTracker& change) { return change.json(); });
    return changes;
}

ReferenceCurve* ReferenceCurve::fromJson(const QJsonObject& object, AbstractMeasureTask* parent)
{
    if (object.isEmpty())
    {
        return nullptr;
    }

    auto uuid = json::parseUuid(object);
    auto lowerUuid = json::parseLower(object);
    auto middleUuid = json::parseMiddle(object);
    auto upperUuid = json::parseUpper(object);

    if (uuid.isNull())
    {
        uuid = QUuid::createUuid();
    }
    if (lowerUuid.isNull())
    {
        lowerUuid = QUuid::createUuid();
    }
    if (middleUuid.isNull())
    {
        middleUuid = QUuid::createUuid();
    }
    if (upperUuid.isNull())
    {
        upperUuid = QUuid::createUuid();
    }

    ReferenceCurve* curve = new ReferenceCurve{parent};
    curve->m_uuid = uuid;
    curve->m_lowerUuid = lowerUuid;
    curve->m_middleUuid = middleUuid;
    curve->m_upperUuid = upperUuid;
    curve->setResultType(json::parseType(object));
    curve->setJitter(json::parseJitter(object));
    curve->setReferenceType(stringToType(json::parseReferenceType(object)));
    curve->setName(json::parseName(object));
    curve->setMeasureTask(parent);

    return curve;
}

QJsonObject ReferenceCurve::toJson() const
{
    return {{
        json::toJson(m_uuid),
        json::nameToJson(m_name),
        json::typeToJson(m_resultType),
        json::referenceTypeToJson(typeToString(m_referenceType)),
        json::jitterToJson(m_jitter),
        json::lowerToJson(m_lowerUuid),
        json::middleToJson(m_middleUuid),
        json::upperToJson(m_upperUuid)
    }};
}

ReferenceCurve *ReferenceCurve::duplicate(CopyMode mode, AbstractMeasureTask* parent) const
{
    auto curve = new ReferenceCurve{parent};
    curve->m_uuid = duplicateUuid(mode, uuid());
    curve->m_lowerUuid = duplicateUuid(mode, lower());
    curve->m_middleUuid = duplicateUuid(mode, middle());
    curve->m_upperUuid = duplicateUuid(mode, upper());
    curve->setMeasureTask(parent);
    curve->setResultType(resultType());
    curve->setName(name());
    curve->setJitter(jitter());
    curve->setReferenceType(referenceType());

    return curve;
}

QString ReferenceCurve::referenceTypeString() const
{
    return typeToString(m_referenceType);
}

QString ReferenceCurve::typeToString(const ReferenceType type)
{
    switch (type)
    {
        case ReferenceType::Average:
            return QStringLiteral("Average");
        case ReferenceType::Median:
            return QStringLiteral("Median");
        case ReferenceType::MinMax:
            return QStringLiteral("Minimum and Maximum");
        default:
            Q_UNREACHABLE();
    }
}

ReferenceCurve::ReferenceType ReferenceCurve::stringToType(const QString &str)
{
    if (str.compare(QStringLiteral("Median"), Qt::CaseInsensitive) == 0)
    {
        return ReferenceType::Median;
    }
    if (str.compare(QStringLiteral("Minimum and Maximum"), Qt::CaseInsensitive) == 0)
    {
        return ReferenceType::MinMax;
    }
    return ReferenceType::Average;
}

void ReferenceCurve::subscribe(const QUuid& errorId)
{
    if (errorId.isNull() || std::any_of(m_subscriptions.begin(), m_subscriptions.end(), [&errorId] (auto id) { return errorId == id; }))
    {
        return;
    }
    m_subscriptions.push_back(errorId);

    emit usedChanged();
}

void ReferenceCurve::unsubscribe(const QUuid& errorId)
{
    const auto it = std::find(m_subscriptions.begin(), m_subscriptions.end(), errorId);
    if (it == m_subscriptions.end())
    {
        return;
    }
    m_subscriptions.erase(it);

    emit usedChanged();
}

void ReferenceCurve::setSourceOfCurve(const std::vector<QString>& serialNumbers)
{
    m_sourceOfCurve = serialNumbers;
}

bool ReferenceCurve::isSource(const QString &serialNumber) const
{
    return std::any_of(m_sourceOfCurve.begin(), m_sourceOfCurve.end(), [&serialNumber] (const auto& sourceSerialNumber) { return sourceSerialNumber == serialNumber; });
}

}
}
