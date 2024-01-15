#include "seamSeries.h"
#include "copyMode.h"
#include "jsonSupport.h"
#include "parameterSet.h"
#include "linkedSeam.h"
#include "seam.h"
#include "referenceCurve.h"
#include "seamSeriesError.h"

#include <QJsonObject>
#include <QXmlStreamReader>

#include <set>

namespace precitec
{
namespace storage
{  
    
SeamSeries::SeamSeries(const QUuid &uuid, Product* parent)
    : AbstractMeasureTask(uuid, parent)
    , m_product(parent)
{
}

SeamSeries::~SeamSeries() = default;

SeamSeries *SeamSeries::duplicate(CopyMode mode, Product *parent) const
{
    auto newUuid = duplicateUuid(mode, uuid());
    auto seamSeries = new SeamSeries{std::move(newUuid), parent};
    seamSeries->copy(mode, this);
    seamSeries->m_seams.reserve(m_seams.size());
    for (auto seam : m_seams)
    {
        if (seam->metaObject()->inherits(&LinkedSeam::staticMetaObject))
        {
            continue;
        }
        seamSeries->m_seams.push_back(seam->duplicate(mode, seamSeries));
    }
    if (mode == CopyMode::Identical)
    {
        seamSeries->copyLinkedSeamsToSeams();
    }
    for (auto error : m_overlyingErrors)
    {
        seamSeries->m_overlyingErrors.push_back(error->duplicate(mode, seamSeries));
    }
    return seamSeries;
}

SeamSeries *SeamSeries::fromJson(const QJsonObject &object, Product *parent)
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
    SeamSeries *series = new SeamSeries(uuid, parent);
    series->AbstractMeasureTask::fromJson(object);
    series->m_seams = json::parseSeams(object, series);
    series->copyLinkedSeamsToSeams();
    series->m_overlyingErrors = json::parseSeamSeriesError(object, series);
    return series;
}

SeamSeries *SeamSeries::fromXml(QXmlStreamReader &xml, Product *parent)
{
    if (xml.qualifiedName().compare(QLatin1String("Messaufgabe")) != 0)
    {
        return nullptr;
    }
    SeamSeries *series = nullptr;
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
            if (!uuid.isNull() && !series)
            {
                series = new SeamSeries(uuid, parent);
            }
            continue;
        }
        if (!series)
        {
            continue;
        }
        if (xml.qualifiedName().compare(QLatin1String("Name")) == 0)
        {
            series->setName(xml.readElementText());
            continue;
        }
        if (xml.qualifiedName().compare(QLatin1String("MessaufgabeLevels")) == 0)
        {
            const auto attributes = xml.attributes();
            series->setNumber(attributes.value(QLatin1String("seamseries")).toInt());
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
            if (auto seam = Seam::fromXml(xml, series))
            {
                series->m_seams.push_back(seam);
            }
            continue;
        }
    }
    return series;
}

ParameterSet *SeamSeries::findHardwareParameterSet(const QUuid &id) const
{
    if (hardwareParameters() && hardwareParameters()->uuid() == id)
    {
        return hardwareParameters();
    }
    for (auto seam : m_seams)
    {
        if (auto hw = seam->findHardwareParameterSet(id))
        {
            return hw;
        }
    }
    return nullptr;
}

Seam *SeamSeries::findSeam(const QUuid &id) const
{
    for (auto seam : m_seams)
    {
        if (seam->uuid() == id)
        {
            return seam;
        }
    }
    return nullptr;
}

Seam *SeamSeries::findSeam(quint32 number) const
{
    for (auto seam : m_seams)
    {
        if (seam->number() == number)
        {
            return seam;
        }
    }
    return nullptr;
}

QJsonObject SeamSeries::toJson() const
{
    auto json = AbstractMeasureTask::toJson();
    // don't persist LinkedSeams. They are part of m_seams, but get persisted as child elements of the Seam
    std::vector<Seam*> seams;
    seams.reserve(m_seams.size());
    std::copy_if(m_seams.begin(), m_seams.end(), std::back_inserter(seams), [] (auto *seam) { return !seam->metaObject()->inherits(&LinkedSeam::staticMetaObject); });

    const QJsonObject child{{
        json::toJson(m_overlyingErrors),
        json::toJson(seams)
    }};
    for (auto it = child.begin(); it != child.end(); it++)
    {
        json.insert(it.key(), it.value());
    }
    return json;
}

Seam *SeamSeries::createSeam()
{
    auto seam = new Seam{QUuid::createUuid(), this};
    seam->createFirstSeamInterval();
    seam->createFilterParameterSet();
    seam->setNumber(maxSeamNumber() + 1);
    if (isChangeTracking())
    {
        addChange(std::move(SeamCreatedChange{seam}));
    }
    m_seams.push_back(seam);
    emit seamsChanged();

    return seam;
}

Seam *SeamSeries::createSeamCopy(CopyMode mode, Seam *seamToCopy)
{
    auto seam = seamToCopy->duplicate(mode, this);
    seam->setNumber(maxSeamNumber() + 1);
    if (isChangeTracking())
    {
        addChange(std::move(SeamCreatedChange{seam}));
    }
    m_seams.push_back(seam);
    emit seamsChanged();

    return seam;
}

Seam *SeamSeries::createSeamLink(Seam *seamToLink, const QString &label)
{
    if (!seamToLink || seamToLink->seamSeries() != this)
    {
        // safety checks
        return nullptr;
    }
    auto seam = seamToLink->link(label);
    if (!seam)
    {
        return nullptr;
    }
    if (isChangeTracking())
    {
        addChange(std::move(SeamCreatedChange{seam}));
    }
    m_seams.push_back(seam);
    emit seamsChanged();

    return seam;
}

int SeamSeries::maxSeamNumber() const
{
    if (m_seams.empty())
    {
        return -1;
    }

    return (*std::max_element(m_seams.begin(), m_seams.end(), [] (auto a, auto b) { return a->number() < b->number(); }))->number();
}

void SeamSeries::destroySeam(precitec::storage::Seam *seam)
{
    auto it = std::find_if(m_seams.begin(), m_seams.end(), [seam] (auto s) { return s == seam; });
    if (it == m_seams.end())
    {
        return;
    }
    if (isChangeTracking())
    {
        addChange(std::move(SeamRemovedChange{*it}));
    }

    static bool s_recursionCheck = false;
    const auto &linkedSeams = (*it)->linkedSeams();

    (*it)->deleteLater();
    m_seams.erase(it);

    if (s_recursionCheck)
    {
        // don't recurse further and don't emit changed signal, emitted by deleting the parent seam
        return;
    }

    s_recursionCheck = true;
    for (auto linkedSeam : linkedSeams)
    {
        destroySeam(linkedSeam);
    }
    s_recursionCheck = false;
    emit seamsChanged();
}

bool SeamSeries::isChangeTracking() const
{
    return m_product->isChangeTracking();
}

QJsonArray SeamSeries::changes() const
{
    auto changes = AbstractMeasureTask::changes();
    QJsonArray seamChanges;
    for (auto seam : m_seams)
    {
        auto changesOnSeam = seam->changes();
        if (changesOnSeam.empty())
        {
            continue;
        }
        seamChanges.push_back(QJsonObject{qMakePair(QString::number(seam->number()), changesOnSeam)});
    }
    if (!seamChanges.empty())
    {
        changes.push_back(QJsonObject{qMakePair(QStringLiteral("seam"), seamChanges)});
    }
    return changes;
}

std::vector<SeamError*> SeamSeries::allSeamErrors() const
{
    std::vector<SeamError*> result;

    const auto &errors = seamErrors();
    result.reserve(errors.size());
    for (auto error : errors)
    {
        result.push_back(error);
    }

    for (auto seam : m_seams)
    {
        const auto errors = seam->seamErrors();
        result.reserve(result.size() + errors.size());
        for (auto error : errors)
        {
            result.push_back(error);
        }
    }

    return result;
}

std::vector<IntervalError*> SeamSeries::allIntervalErrors() const
{
    std::vector<IntervalError*> result;

    for (auto seam : m_seams)
    {
        const auto intervalErrors = seam->allIntervalErrors();
        result.reserve(result.size() + intervalErrors.size());
        for (auto error : intervalErrors)
        {
            result.push_back(error);
        }
    }

    return result;
}

int SeamSeries::intervalErrorCount()
{
    int summ = 0;
    for (auto seam : m_seams)
    {
        summ += seam->intervalErrorCount();
    }
    return summ;
}

SeamSeriesError *SeamSeries::addOverlyingError(const QUuid &variantId)
{
    SeamSeriesError *error = new SeamSeriesError(this);
    error->setMeasureTask(this);
    error->setVariantId(variantId);
    m_overlyingErrors.push_back(error);
    return error;
}

void SeamSeries::removeOverlyingError(int index)
{
    if (index < 0 || index >= int(m_overlyingErrors.size()))
    {
        return;
    }

    auto it = m_overlyingErrors.begin();
    std::advance(it, index);

    if (it != m_overlyingErrors.end())
    {
        (*it)->deleteLater();
        m_overlyingErrors.erase(it);
    }
}

std::vector<ReferenceCurve*> SeamSeries::allReferenceCurves() const
{
    std::vector<ReferenceCurve*> result;

    const auto& seriesCurves = referenceCurves();
    result.reserve(seriesCurves.size());
    for (auto curve : seriesCurves)
    {
        result.push_back(curve);
    }

    for (auto seam : m_seams)
    {
        const auto& seamCurves = seam->referenceCurves();
        result.reserve(result.size() + seamCurves.size());
        for (auto curve : seamCurves)
        {
            result.push_back(curve);
        }
    }

    return result;
}

QVariantList SeamSeries::allSeams() const
{
    QVariantList seams;
    std::transform(m_seams.begin(), m_seams.end(), std::back_inserter(seams), [] (auto s) { return QVariant::fromValue(s); });
    return seams;
}

Seam *SeamSeries::previousSeam(Seam *seam) const
{
    auto it = std::find(m_seams.begin(), m_seams.end(), seam);
    if (it == m_seams.end())
    {
        // not found
        return nullptr;
    }
    if (it == m_seams.begin())
    {
        // already at beginning
        return nullptr;
    }
    it--;
    return *it;
}

Seam *SeamSeries::nextSeam(Seam *seam) const
{
    auto it = std::find(m_seams.begin(), m_seams.end(), seam);
    if (it == m_seams.end())
    {
        // not found
        return nullptr;
    }
    it++;
    if (it == m_seams.end())
    {
        return nullptr;
    }
    return *it;
}

void SeamSeries::copyLinkedSeamsToSeams()
{
    const auto size = m_seams.size();
    for (std::size_t i = 0; i < size; i++)
    {
        const auto &linkedSeams = m_seams.at(i)->linkedSeams();
        if (linkedSeams.empty())
        {
            continue;
        }
        m_seams.reserve(m_seams.size() + linkedSeams.size());
        std::copy(linkedSeams.begin(), linkedSeams.end(), std::back_inserter(m_seams));
    }
    std::sort(m_seams.begin(), m_seams.end(), [] (auto seam1, auto seam2) {
        return seam1->number() < seam2->number();
    });
}


}
}
