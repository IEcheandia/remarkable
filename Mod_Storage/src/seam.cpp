#include "seam.h"
#include "copyMode.h"
#include "intervalError.h"
#include "jsonSupport.h"
#include "linkedSeam.h"
#include "parameterSet.h"
#include "seamError.h"
#include "seamInterval.h"
#include "seamSeries.h"

#include <QThread>
#include <QJsonObject>
#include <QXmlStreamReader>

namespace precitec
{
namespace storage
{

Seam::Seam(const QUuid &uuid, SeamSeries *parent)
    : AbstractMeasureTask(uuid, parent && parent->thread() == QThread::currentThread() ? parent : nullptr)
    , m_seamSeries(parent)
{
}

Seam::~Seam() = default;

Seam *Seam::duplicate(CopyMode mode, SeamSeries *parent) const
{
    auto newSeamUuid = duplicateUuid(mode, uuid());
    auto seam = new Seam{std::move(newSeamUuid), parent};
    seam->setPositionInAssemblyImage(positionInAssemblyImage());
    seam->setRoi(roi());
    seam->copy(mode, this);
    seam->setTriggerDelta(triggerDelta());
    seam->setVelocity(velocity());
    seam->setThicknessLeft(thicknessLeft());
    seam->setThicknessRight(thicknessRight());
    seam->setTargetDifference(targetDifference());
    seam->setMovingDirection(movingDirection());

    if (mode == CopyMode::Identical)
    {
        for (LinkedSeam* linked : m_linkedSeams)
        {
            auto* newLinked = seam->m_linkedSeams.emplace_back(linked->clone(mode, seam));
            seam->connectDestroySignal(newLinked);
        }
    }

    if (parent)
    {
        if (const auto parentProduct = parent->product())
        {
            auto parameterSetCreated = false;
            if (m_seamSeries)
            {
                if (const auto product = m_seamSeries->product())
                {
                    if (const auto parameterSet = product->filterParameterSet(graphParamSet()))
                    {
                        auto duplicateParameterSet = parameterSet->duplicate(mode, parentProduct);
                        parentProduct->addFilterParameterSet(duplicateParameterSet);
                        seam->setGraphParamSet(duplicateParameterSet->uuid());
                        parameterSetCreated = true;
                    }
                }
            }
            if (!parameterSetCreated)
            {
                seam->createFilterParameterSet();
            }
        }
    }
    seam->duplicateSeamIntervals(mode, this);
    seam->duplicateIntervalErrors(mode, this);
    return seam;
}

void Seam::duplicateSeamIntervals(CopyMode mode, const Seam* source)
{
    for (auto interval : m_intervals)
    {
        if (isChangeTracking())
        {
            addChange(std::move(SeamIntervalCreatedChange{interval}));
        }
        for (auto err : m_intervalErrors)
        {
            err->removeInterval(interval);
        }
        disconnect(interval, &SeamInterval::lengthChanged, this, &Seam::lengthChanged);
        interval->deleteLater();
    }
    m_intervals.clear();

    if (source)
    {
        m_intervals.reserve(source->m_intervals.size());
        for (auto interval : source->m_intervals)
        {
            auto newInterval = interval->duplicate(mode, this);

            if (isChangeTracking())
            {
                addChange(std::move(SeamIntervalCreatedChange{interval}));
            }
            connect(newInterval, &SeamInterval::lengthChanged, this, &Seam::lengthChanged);
            m_intervals.push_back(newInterval);

            for (auto intervalError : m_intervalErrors)
            {
                intervalError->addInterval(newInterval);
            }
        }
    }

    emit allSeamIntervalsChanged();
    emit seamIntervalsCountChanged();
    emit lengthChanged();
}

void Seam::duplicateIntervalErrors(CopyMode mode, const Seam* source)
{
    m_intervalErrors.clear();
    m_intervalErrors.reserve(source->m_intervalErrors.size());
    for (auto intervalError : source->m_intervalErrors)
    {
        auto newError = intervalError->duplicate(mode, this);
        m_intervalErrors.push_back(newError);
        for (auto interval : m_intervals)
        {
            newError->addInterval(interval);
        }
    }
}

Seam *Seam::link(const QString &label)
{
    if (findLink(label))
    {
        return nullptr;
    }
    auto *linked = LinkedSeam::create(QUuid::createUuid(), this, label);
    if (linked->seamSeries()->findSeam(linked->number()))
    {
        linked->deleteLater();
        return nullptr;
    }
    m_linkedSeams.push_back(linked);
    connectDestroySignal(linked);
    emit linkedSeamsChanged();
    return linked;
}

Seam *Seam::findLink(const QString &label)
{
    auto it = std::find_if(m_linkedSeams.begin(), m_linkedSeams.end(), [label] (auto *seam) { return seam->label() == label; });
    if (it != m_linkedSeams.end())
    {
        return *it;
    }
    return nullptr;
}

Seam *Seam::fromJson(const QJsonObject &object, SeamSeries *parent)
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
    Seam *seam = new Seam(uuid, parent);
    seam->AbstractMeasureTask::fromJson(object);
    seam->m_intervals = json::parseSeamIntervals(object, seam);
    std::sort(seam->m_intervals.begin(), seam->m_intervals.end(), [] (auto interval1, auto interval2) {
        return interval1->number() < interval2->number();
    });
    seam->m_positionInAssemblyImage = json::parsePositionInAssemblyImage(object);
    seam->m_roi = json::parseRoi(object);
    seam->m_intervalErrors = json::parseIntervalErrors(object, seam);
    seam->setTriggerDelta(json::parseTriggerDelta(object));
    seam->setVelocity(json::parseVelocity(object));
    seam->setThicknessLeft(json::parseThicknessLeft(object));
    seam->setThicknessRight(json::parseThicknessRight(object));
    seam->setTargetDifference(json::parseTargetDifference(object));
    seam->setMovingDirection(MovingDirection(json::parseMovingDirection(object)));
    for (auto i: seam->m_intervals)
    {
        connect(i, &SeamInterval::lengthChanged, seam, &Seam::lengthChanged);
    }
    seam->m_linkedSeams = json::parseLinkedSeams(object, seam);
    for (auto linked : seam->m_linkedSeams)
    {
        seam->connectDestroySignal(linked);
    }
    return seam;
}

Seam *Seam::fromXml(QXmlStreamReader &xml, SeamSeries *parent)
{
    if (xml.qualifiedName().compare(QLatin1String("Messaufgabe")) != 0)
    {
        return nullptr;
    }
    Seam *seam = nullptr;
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
            if (!uuid.isNull() && !seam)
            {
                seam = new Seam(uuid, parent);
            }
            continue;
        }
        if (!seam)
        {
            continue;
        }
        if (xml.qualifiedName().compare(QLatin1String("Name")) == 0)
        {
            seam->setName(xml.readElementText());
            continue;
        }
        if (xml.qualifiedName().compare(QLatin1String("MessaufgabeLevels")) == 0)
        {
            const auto attributes = xml.attributes();
            seam->setNumber(attributes.value(QLatin1String("seam")).toInt());
            continue;
        }
        if (xml.qualifiedName().compare(QLatin1String("MessaufgabeGeometry")) == 0)
        {
            const auto attributes = xml.attributes();
            seam->setTriggerDelta(attributes.value(QLatin1String("triggerdistance")).toInt());
            seam->setVelocity(attributes.value(QLatin1String("Velocity")).toInt());
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
        if (xml.qualifiedName().compare(QLatin1String("Messaufgabe")) == 0)
        {
            if (auto interval = SeamInterval::fromXml(xml, seam))
            {
                seam->m_intervals.push_back(interval);
            }
        }
    }
    for (auto i: seam->m_intervals)
    {
        connect(i, &SeamInterval::lengthChanged, seam, &Seam::lengthChanged);
    }
    return seam;
}

ParameterSet *Seam::findHardwareParameterSet(const QUuid &id) const
{
    if (hardwareParameters() && hardwareParameters()->uuid() == id)
    {
        return hardwareParameters();
    }
    return nullptr;
}

int Seam::length() const
{
    return std::accumulate(m_intervals.begin(), m_intervals.end(), 0, [] (int a, SeamInterval *b) { return a + b->length(); });
}

QJsonObject Seam::toJson() const
{
    auto json = AbstractMeasureTask::toJson();
    const QJsonObject child{{
        json::toJson(m_intervals),
        json::toJson(m_linkedSeams),
        json::positionInAssemblyImageToJson(m_positionInAssemblyImage),
        json::roiToJson(m_roi),
        json::toJson(m_intervalErrors),
        json::triggerDeltaToJson(m_triggerDelta),
        json::velocityToJson(m_velocity),
        json::thicknessLeftToJson(m_thicknessLeft),
        json::thicknessRightToJson(m_thicknessRight),
        json::targetDifferenceToJson(m_targetDifference),
        json::movingDirectionToJson(int(m_movingDirection))
    }};
    for (auto it = child.begin(); it != child.end(); it++)
    {
        json.insert(it.key(), it.value());
    }
    return json;
}

void Seam::setTriggerDelta(int delta)
{
    if (m_triggerDelta == delta)
    {
        return;
    }
    if (isChangeTracking())
    {
        addChange(std::move(PropertyChange{QStringLiteral("triggerDelta"), m_triggerDelta, delta}));
    }
    m_triggerDelta = delta;
    emit triggerDeltaChanged();
}

void Seam::setVelocity(int velocity)
{
    if (m_velocity == velocity)
    {
        return;
    }
    if (isChangeTracking())
    {
        addChange(std::move(PropertyChange{QStringLiteral("velocity"), m_velocity, velocity}));
    }
    m_velocity = velocity;
    emit velocityChanged();
}

void Seam::setThicknessLeft(int thickness)
{
    if (m_thicknessLeft == thickness)
    {
        return;
    }
    if (isChangeTracking())
    {
        addChange(std::move(PropertyChange{QStringLiteral("Left Thickness Changed"), m_thicknessLeft, thickness}));
    }
    m_thicknessLeft = thickness;
    emit thicknessLeftChanged();
}

void Seam::setThicknessRight(int thickness)
{
    if (m_thicknessRight == thickness)
    {
        return;
    }
    if (isChangeTracking())
    {
        addChange(std::move(PropertyChange{QStringLiteral("Right Thickness Changed"), m_thicknessRight, thickness}));
    }
    m_thicknessRight = thickness;
    emit thicknessRightChanged();
}

void Seam::setTargetDifference(int difference)
{
    if (m_targetDifference == difference)
    {
        return;
    }
    if (isChangeTracking())
    {
        addChange(std::move(PropertyChange{QStringLiteral("Target Difference Changed"), m_targetDifference, difference}));
    }
    m_targetDifference = difference;
    emit targetDifferenceChanged();
}

void Seam::setMovingDirection(MovingDirection direction)
{
    if (m_movingDirection == direction)
    {
        return;
    }
    if (isChangeTracking())
    {
        addChange(std::move(PropertyChange{QStringLiteral("movingDirection"), int(m_movingDirection), int(direction)}));
    }
    m_movingDirection = direction;
    emit movingDirectionChanged();
}

SeamInterval *Seam::firstSeamInterval() const
{
    if (m_intervals.empty())
    {
        return nullptr;
    }
    return m_intervals.front();
}

void Seam::createFirstSeamInterval()
{
    if (!m_intervals.empty())
    {
        return;
    }
    auto interval = new SeamInterval{QUuid::createUuid(), this};
    interval->setName(QStringLiteral("First Seam Interval"));
    connect(interval, &SeamInterval::lengthChanged, this, &Seam::lengthChanged);
    if (isChangeTracking())
    {
        addChange(std::move(SeamIntervalCreatedChange{interval}));
    }
    m_intervals.emplace_back(interval);
    for (auto err : m_intervalErrors)
    {
        err->addInterval(interval);
    }
}

void Seam::createFilterParameterSet()
{
    if (!m_seamSeries)
    {
        return;
    }
    if (const auto product = m_seamSeries->product())
    {
        if (!graphParamSet().isNull() && product->filterParameterSet(graphParamSet()))
        {
            return;
        }
        setGraphParamSet(QUuid::createUuid());
        product->addFilterParameterSet(new ParameterSet{graphParamSet(), product});
    }
}

SeamInterval *Seam::createSeamInterval()
{
    auto interval = new SeamInterval{QUuid::createUuid(), this};
    interval->setNumber(maxSeamIntervalNumber() + 1);
    if (isChangeTracking())
    {
        addChange(std::move(SeamIntervalCreatedChange{interval}));
    }
    connect(interval, &SeamInterval::lengthChanged, this, &Seam::lengthChanged);

    m_intervals.emplace_back(interval);
    for (auto err : m_intervalErrors)
    {
        err->addInterval(interval);
    }
    emit allSeamIntervalsChanged();
    emit seamIntervalsCountChanged();
    emit lengthChanged();
    return interval;
}

int Seam::maxSeamIntervalNumber() const
{
    if (m_intervals.empty())
    {
        return -1;
    }

    return (*std::max_element(m_intervals.begin(), m_intervals.end(), [] (auto a, auto b) { return a->number() < b->number(); }))->number();
}

void Seam::destroySeamInterval(SeamInterval *seamInterval)
{
    auto it = std::find_if(m_intervals.begin(), m_intervals.end(), [seamInterval] (auto s) { return s == seamInterval; });
    if (it == m_intervals.end())
    {
        return;
    }
    if (isChangeTracking())
    {
        addChange(std::move(SeamIntervalCreatedChange{*it}));
    }
    for (auto err : m_intervalErrors)
    {
        err->removeInterval(seamInterval);
    }
    disconnect(seamInterval, &SeamInterval::lengthChanged, this, &Seam::lengthChanged);
    (*it)->deleteLater();
    m_intervals.erase(it);
    emit allSeamIntervalsChanged();
    emit seamIntervalsCountChanged();
    emit lengthChanged();
}

SeamInterval *Seam::findSeamInterval(const QUuid &id) const
{
    if (m_intervals.empty())
    {
        return nullptr;
    }
    for (auto seamInterval : m_intervals)
    {
        if (seamInterval->uuid() == id)
        {
            return seamInterval;
        }
    }
    return nullptr;
}

void Seam::setPositionInAssemblyImage(const QPointF &position)
{
    if (m_positionInAssemblyImage == position)
    {
        return;
    }
    if (isChangeTracking())
    {
        addChange(std::move(PropertyChange{QStringLiteral("positionInAssemblyImage"), m_positionInAssemblyImage, position}));
    }
    m_positionInAssemblyImage = position;
    emit positionInAssemblyImageChanged();
}

void Seam::setRoi(const QRect &roi)
{
    if (m_roi == roi)
    {
        return;
    }
    if (isChangeTracking())
    {
        addChange(std::move(PropertyChange{QStringLiteral("roi"), m_roi, roi}));
    }
    m_roi = roi;
    emit roiChanged();
}

bool Seam::isChangeTracking() const
{
    return m_seamSeries ? m_seamSeries->isChangeTracking() : false;
}

QJsonArray Seam::changes() const
{
    auto changes = AbstractMeasureTask::changes();
    QJsonArray intervalChanges;
    for (auto interval : m_intervals)
    {
        auto changesOnInterval = interval->changes();
        if (changesOnInterval.empty())
        {
            continue;
        }
        intervalChanges.push_back(QJsonObject{qMakePair(QString::number(interval->number()), changesOnInterval)});
    }
    if (!intervalChanges.empty())
    {
        changes.push_back(QJsonObject{qMakePair(QStringLiteral("interval"), intervalChanges)});
    }
    return changes;
}

QVariantList Seam::allSeamIntervals() const
{
    QVariantList seamIntervals;
    for (auto seam : m_intervals)
    {
        seamIntervals << QVariant::fromValue(seam);
    }
    return seamIntervals;
}

IntervalError *Seam::addIntervalError(const QUuid &variantId)
{
    auto *error = new IntervalError(this);
    error->setMeasureTask(this);
    error->setVariantId(variantId);
    for (auto interval : m_intervals)
    {
        error->addInterval(interval);
    }
    m_intervalErrors.push_back(error);
    return error;
}

void Seam::removeIntervalError(int index)
{
    if (index < 0 || index >= int(m_intervalErrors.size()))
    {
        return;
    }

    auto it = m_intervalErrors.begin();
    std::advance(it, index);

    if (it != m_intervalErrors.end())
    {
        (*it)->deleteLater();
        m_intervalErrors.erase(it);
    }
}

int Seam::intervalErrorCount()
{
    return m_intervalErrors.size() * m_intervals.size();
}

std::vector<IntervalError*> Seam::allIntervalErrors() const
{
    std::vector<IntervalError*> result;

    result.reserve(m_intervalErrors.size());
    for (auto intervalError : m_intervalErrors)
    {
        result.push_back(intervalError);
    }

    return result;
}

Product* Seam::product() const
{
    if (m_seamSeries)
    {
        return m_seamSeries->product();
    }
    return nullptr;
}

QVariantList Seam::allLinkedSeams() const
{
    QVariantList linkedSeams;
    std::transform(m_linkedSeams.begin(), m_linkedSeams.end(), std::back_inserter(linkedSeams), [] (auto linkedSeam) { return QVariant::fromValue(linkedSeam); });
    return linkedSeams;
}

void Seam::connectDestroySignal(Seam *linked) 
{
    connect(linked, &QObject::destroyed, this,
        [this, linked]
        {
            auto it = std::find(this->m_linkedSeams.begin(), this->m_linkedSeams.end(), linked);
            if (it != this->m_linkedSeams.end())
            {
                this->m_linkedSeams.erase(it);
                emit this->linkedSeamsChanged();
            }
        }
    );
}
}
}
