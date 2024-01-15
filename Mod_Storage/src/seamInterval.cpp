#include "seamInterval.h"
#include "copyMode.h"
#include "jsonSupport.h"
#include "seam.h"

#include <QJsonObject>
#include <QXmlStreamReader>

namespace precitec
{
namespace storage
{

SeamInterval::SeamInterval(const QUuid &uuid, Seam *parent)
    : AbstractMeasureTask(uuid, parent)
    , m_seam(parent)
{
}

SeamInterval::~SeamInterval() = default;

SeamInterval *SeamInterval::duplicate(CopyMode mode, Seam *parent) const
{
    auto newUuid = duplicateUuid(mode, uuid());
    auto interval = new SeamInterval{newUuid, parent};
    interval->setNumber(number());
    interval->setName(name());
    interval->setLength(length());
    interval->setLevel(level());
    return interval;
}

void SeamInterval::setLength(qint64 length)
{
    if (m_length == length)
    {
        return;
    }
    if (isChangeTracking())
    {
        addChange(std::move(PropertyChange{QStringLiteral("length"), m_length, length}));
    }
    m_length = length;
    emit lengthChanged();
}

void SeamInterval::setLevel(int level)
{
    if (m_level == level)
    {
        return;
    }
    if (isChangeTracking())
    {
        addChange(std::move(PropertyChange{QStringLiteral("level"), m_level, level}));
    }
    m_level = level;
    emit levelChanged();
}

SeamInterval *SeamInterval::fromJson(const QJsonObject &object, Seam *parent)
{
    if (object.isEmpty())
    {
        return nullptr;
    }
    auto uuid = json::parseUuid(object);
    if (uuid.isNull())
    {
        uuid = QUuid::createUuid();
    }
    auto interval = new SeamInterval(uuid, parent);
    interval->setNumber(json::parseNumber(object));
    interval->setName(json::parseName(object));
    interval->setLength(json::parseLength(object));
    interval->setLevel(json::parseLevel(object));

    // legacy, set filter graph parameters to seam
    if (interval->seam())
    {
        const auto subGraphs = json::parseSubGraphs(object);
        if (!subGraphs.empty())
        {
            interval->seam()->setSubGraphs(subGraphs);
        }

        const auto graphId = json::parseGraph(object);
        if (!graphId.isNull())
        {
            interval->seam()->setGraph(graphId);
        }

        const auto filterSetId = json::parseGraphParamSet(object);
        if (!filterSetId.isNull())
        {
            interval->seam()->setGraphParamSet(filterSetId);
        }
    }

    return interval;
}

SeamInterval *SeamInterval::fromXml(QXmlStreamReader &xml, Seam *parent)
{
    if (xml.qualifiedName().compare(QLatin1String("Messaufgabe")) != 0)
    {
        return nullptr;
    }
    SeamInterval *interval = nullptr;
    while (!xml.atEnd())
    {
        if (!xml.readNextStartElement())
        {
            if (xml.qualifiedName().compare(QLatin1String("Messaufgabe")) == 0)
            {
                break;
            }
            continue;
        }
        if (xml.qualifiedName().compare(QLatin1String("MessaufgabeID")) == 0)
        {
            const QUuid uuid = QUuid::fromString(xml.readElementText());
            if (!uuid.isNull() && !interval)
            {
                interval = new SeamInterval(uuid, parent);
            }
            continue;
        }
        if (!interval)
        {
            continue;
        }
        if (xml.qualifiedName().compare(QLatin1String("GraphID")) == 0)
        {
            if (const auto seam = interval->seam())
            {
                seam->setGraph(QUuid::fromString(xml.readElementText()));
            }
            continue;
        }
        if (xml.qualifiedName().compare(QLatin1String("Name")) == 0)
        {
            interval->setName(xml.readElementText());
            continue;
        }
        if (xml.qualifiedName().compare(QLatin1String("MessaufgabeLevels")) == 0)
        {
            const auto attributes = xml.attributes();
            interval->setNumber(attributes.value(QLatin1String("seaminterval")).toInt());
            continue;
        }
        if (xml.qualifiedName().compare(QLatin1String("MessaufgabeGeometry")) == 0)
        {
            const auto attributes = xml.attributes();
            interval->setLength(attributes.value(QLatin1String("Laenge")).toInt());
            continue;
        }
        if (xml.qualifiedName().compare(QLatin1String("SumErrorsParameters")) == 0)
        {
            // skip for the moment
            while (!xml.atEnd())
            {
                if (!xml.readNextStartElement())
                {
                    if (xml.qualifiedName().compare(QLatin1String("SumErrorsParameters")) == 0)
                    {
                        break;
                    }
                    continue;
                }
            }
        }
        if (xml.qualifiedName().compare(QLatin1String("HardwareParameters")) == 0)
        {
            // hardware parameters are skipped as the uuids do not match
            while (!xml.atEnd())
            {
                if (!xml.readNextStartElement())
                {
                    if (xml.qualifiedName().compare(QLatin1String("HardwareParameters")) == 0)
                    {
                        break;
                    }
                    continue;
                }
            }
        }
    }
    return interval;
}

QJsonObject SeamInterval::toJson() const
{
    return {
        json::nameToJson(name()),
        json::toJson(uuid()),
        json::numberToJson(number()),
        json::lengthToJson(m_length),
        json::levelToJson(int(m_level))
    };
}

bool SeamInterval::isChangeTracking() const
{
    return m_seam ? m_seam->isChangeTracking() : false;
}

QColor SeamInterval::color() const
{
    return AbstractMeasureTask::levelColor(m_level);
}

QUuid SeamInterval::graph() const
{
    if (!m_seam)
    {
        return AbstractMeasureTask::graph();
    }
    return m_seam->graph();
}

const GraphReference& SeamInterval::graphReference() const
{
    if (!m_seam)
    {
        return AbstractMeasureTask::graphReference();
    }
    return m_seam->graphReference();
}

QUuid SeamInterval::graphParamSet() const
{
    if (!m_seam)
    {
        return AbstractMeasureTask::graphParamSet();
    }
    return m_seam->graphParamSet();
}

const std::vector<QUuid> &SeamInterval::subGraphs() const
{
    if (!m_seam)
    {
        return AbstractMeasureTask::subGraphs();
    }
    return m_seam->subGraphs();
}

bool SeamInterval::usesSubGraph() const
{
    if (!m_seam)
    {
        return AbstractMeasureTask::usesSubGraph();
    }
    return m_seam->usesSubGraph();
}

ParameterSet *SeamInterval::hardwareParameters() const
{
    if (!m_seam)
    {
        return AbstractMeasureTask::hardwareParameters();
    }
    return m_seam->hardwareParameters();
}

Product* SeamInterval::product() const
{
    if (m_seam)
    {
        return m_seam->product();
    }
    return nullptr;
}

}
}
