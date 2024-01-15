#include "abstractMeasureTask.h"

#include "copyMode.h"
#include "graphReferenceFunctions.h"
#include "jsonSupport.h"
#include "parameterSet.h"
#include "product.h"
#include "referenceCurve.h"
#include "seamError.h"

#include <QColor>

namespace precitec
{
namespace storage
{
using graphFunctions::doOnLinkedTarget;

AbstractMeasureTask::AbstractMeasureTask(const QUuid &uuid, QObject *parent)
    : QObject(parent)
    , m_uuid(uuid)
{
}

AbstractMeasureTask::~AbstractMeasureTask() = default;

QUuid AbstractMeasureTask::graphParamSet() const
{
    if (const auto* ref_ptr = std::get_if<LinkedGraphReference>(&m_graphReference))
    {
        return doOnLinkedTarget(*ref_ptr, product(), [](const AbstractMeasureTask& linked)
                                { return linked.graphParamSet(); });
    }
    return m_graphParamSetUuid;
}

void AbstractMeasureTask::setNumber(quint32 number)
{
    if (m_number == number)
    {
        return;
    }
    if (isChangeTracking())
    {
        addChange(std::move(PropertyChange{QStringLiteral("number"), m_number, number}));
    }
    m_number = number;
    emit numberChanged();
}

void AbstractMeasureTask::setGraph(const QUuid &uuid)
{
    setGraphReference(SingleGraphReference{uuid});
}

void AbstractMeasureTask::setGraphReference(const GraphReference& newRef)
{
    if (m_graphReference == newRef)
    {
        return;
    }

    const bool newAndOldSubGraphsAreEmpty = hasSubGraphs(newRef) && valueOrDefault<SubGraphReference>(newRef).empty() && this->subGraphs().empty();
    const bool graphWasSetBefore = !graph().isNull();

    if (!newAndOldSubGraphsAreEmpty)
    {
        if (isChangeTracking())
        {
            addChange(PropertyChange{QStringLiteral("graph"), toStringForChangeTracking(m_graphReference), toStringForChangeTracking(newRef)});
        }
        m_graphReference = newRef;
    }

    std::visit(overloaded{[&](const SingleGraphReference& singleRef)
                          {
                              // TODODa: fehlt hier ein emit subGraphChanged()?
                              emit graphChanged();
                          },
                          [&](const SubGraphReference& subGraphRef)
                          {
                              if (!subGraphRef.value.empty() && graphWasSetBefore)
                              {
                                  // Graph is replaced by new subGraph.
                                  emit graphChanged();
                              }
                              else if (newAndOldSubGraphsAreEmpty)
                              {
                                  // New and old subGraphs were empty.
                                  emit graphChanged();
                              }
                              else
                              {
                                  // SubGraph is replaced by new SubGraph
                                  emit subGraphChanged();
                              }
                          },
                          [&](const LinkedGraphReference& linkedRef)
                          {
                              m_graphParamSetUuid = QUuid{};
                              // TODODa: when connecting the UI we might want to emit a new signal here.
                          }},
               newRef);
}

void AbstractMeasureTask::setGraphParamSet(const QUuid &uuid)
{
    if (m_graphParamSetUuid == uuid || hasLinkedGraph(m_graphReference))
    {
        return;
    }
    if (isChangeTracking())
    {
        addChange(std::move(PropertyChange{QStringLiteral("graphParamSet"), m_graphParamSetUuid, uuid}));
    }
    m_graphParamSetUuid = uuid;
    emit graphParamSetChanged();
}

void AbstractMeasureTask::setHardwareParameters(ParameterSet *parameters)
{
    if (m_hardwareParameters == parameters)
    {
        return;
    }
    if (isChangeTracking())
    {
        addChange(std::move(HardwareParameterSetReplacedChange{m_hardwareParameters, parameters}));
    }
    delete m_hardwareParameters;
    m_hardwareParameters = parameters;
    emit hardwareParametersChanged();
}

void AbstractMeasureTask::setName(const QString &name)
{
    if (m_name == name)
    {
        return;
    }
    if (isChangeTracking())
    {
        addChange(std::move(PropertyChange{QStringLiteral("name"), m_name, name}));
    }
    m_name = name;
    emit nameChanged();
}

void AbstractMeasureTask::setLaserControlPreset(const QUuid &preset)
{
    if (m_laserControlPreset == preset)
    {
        return;
    }
    if (isChangeTracking())
    {
        addChange(std::move(PropertyChange{QStringLiteral("laserControlPreset"), m_laserControlPreset, preset}));
    }
    m_laserControlPreset = preset;
    emit laserControlPresetChanged();
}

void AbstractMeasureTask::fromJson(const QJsonObject &object)
{
    setNumber(json::parseNumber(object));
    setGraphReference(json::parseGraphReference(object));
    setGraphParamSet(json::parseGraphParamSet(object));
    setHardwareParameters(json::parseHardwareParameters(object, this));
    setName(json::parseName(object));
    setLaserControlPreset(json::parseLaserControl(object));
    m_referenceCurves = json::parseReferenceCurves(object, this);
    if (m_referenceCurves.empty())
    {
        // legacy
        m_referenceCurves = json::parseReferenceCurveSets(object, this);
    }
    m_errors = json::parseSeamErrors(object, this);
}

QJsonObject AbstractMeasureTask::toJson() const
{
    return QJsonObject{
        {json::nameToJson(m_name),
         json::toJson(m_uuid),
         json::graphReferenceToJson(m_graphReference),
         json::graphParamSetToJson(m_graphParamSetUuid),
         json::hardwareParametersToJson(m_hardwareParameters),
         json::numberToJson(m_number),
         json::laserControlToJson((m_laserControlPreset)),
         json::toJson(m_errors),
         json::toJson(m_referenceCurves)}};
}

void AbstractMeasureTask::copy(CopyMode mode, const AbstractMeasureTask *source)
{
    setNumber(source->number());
    setName(source->name());
    // Graph references may get adjusted when duplicating a whole product, see Product::duplicate.
    setGraphReference(source->graphReference());
    setLaserControlPreset(source->laserControlPreset());
    if (source->hardwareParameters())
    {
        m_hardwareParameters = source->hardwareParameters()->duplicate(mode, this);
    }
    copyErrorsAndReferenceCurves(mode, source);
}

void AbstractMeasureTask::copyErrorsAndReferenceCurves(CopyMode mode, const AbstractMeasureTask *source)
{
    for (auto error : m_errors)
    {
        error->deleteLater();
    }
    m_errors.clear();
    for (auto curve : m_referenceCurves)
    {
        curve->deleteLater();
    }
    m_referenceCurves.clear();
    std::map<QUuid, QUuid> refCurveMap;
    m_referenceCurves.reserve(source->referenceCurves().size());
    auto parentProduct = product();
    for (auto rc : source->referenceCurves())
    {
        const auto newRefCurve = rc->duplicate(mode, this);
        refCurveMap.emplace(std::make_pair(rc->uuid(), newRefCurve->uuid()));
        m_referenceCurves.push_back(newRefCurve);
        if (parentProduct)
        {
            parentProduct->copyReferenceCurveData(source->product(), rc, newRefCurve);
        }
    }
    m_errors.reserve(source->seamErrors().size());
    for (auto se : source->seamErrors())
    {
        auto newError = se->duplicate(mode, this);
        newError->setEnvelope((*refCurveMap.find(se->envelope())).second);
        m_errors.push_back(newError);
    }
}

void AbstractMeasureTask::createHardwareParameters()
{
    if (m_hardwareParameters)
    {
        return;
    }
    m_hardwareParameters = new ParameterSet(QUuid::createUuid(), this);
    if (isChangeTracking())
    {
        addChange(std::move(HardwareParametersCreatedChange(m_hardwareParameters)));
    }
    m_hardwareParameters->setChangeTrackingEnabled(isChangeTracking());
    emit hardwareParametersChanged();
}

void AbstractMeasureTask::addChange(ChangeTracker &&change)
{
    m_changeTracker.emplace_back(std::move(change));
}

QJsonArray AbstractMeasureTask::changes() const
{
    QJsonArray changes;
    std::transform(m_changeTracker.begin(), m_changeTracker.end(), std::back_inserter(changes), [] (const ChangeTracker &change) { return change.json(); });
    if (m_hardwareParameters)
    {
        const auto& hwChanges = m_hardwareParameters->changes();
        if (!hwChanges.empty())
        {
            changes.push_back(QJsonObject{qMakePair(QStringLiteral("hardwareParameters"), hwChanges)});
        }
    }
    if (!m_errors.empty())
    {
        QJsonArray errors;
        for (auto error : m_errors)
        {
            const auto& errorChanges = error->changes();
            if (!errorChanges.isEmpty())
            {
                errors.push_back(QJsonObject{
                    qMakePair(QStringLiteral("error"), error->toJson()),
                    qMakePair(QStringLiteral("changes"), errorChanges)
                });
            }
        }
        if (!errors.empty())
        {
            changes.push_back(QJsonObject{qMakePair(QStringLiteral("errors"), errors)});
        }
    }
    if (!m_referenceCurves.empty())
    {
        QJsonArray referenceCurves;
        for (auto curve : m_referenceCurves)
        {
            const auto& curveChanges = curve->changes();
            if (!curveChanges.isEmpty())
            {
                referenceCurves.push_back(QJsonObject{
                    qMakePair(QStringLiteral("referenceCurve"), curve->toJson()),
                    qMakePair(QStringLiteral("changes"), curveChanges)
                });
            }
        }
        if (!referenceCurves.empty())
        {
            changes.push_back(QJsonObject{qMakePair(QStringLiteral("referenceCurves"), referenceCurves)});
        }
    }
    return changes;
}

SeamError *AbstractMeasureTask::addError(const QUuid &variantId)
{
    SeamError *error = new SeamError(this);
    error->setMeasureTask(this);
    error->setVariantId(variantId);
    m_errors.push_back(error);
    return error;
}

void AbstractMeasureTask::removeError(int index)
{
    if (index < 0 || index >= int(m_errors.size()))
    {
        return;
    }

    auto it = m_errors.begin();
    std::advance(it, index);

    if (it != m_errors.end())
    {
        (*it)->deleteLater();
        m_errors.erase(it);
    }
}

ReferenceCurve* AbstractMeasureTask::createReferenceCurve(const int type)
{
    ReferenceCurve* curve = new ReferenceCurve{this};
    curve->setMeasureTask(this);
    curve->setResultType(type);
    m_referenceCurves.emplace_back(curve);
    return curve;
}

ReferenceCurve* AbstractMeasureTask::copyReferenceCurve(ReferenceCurve* curve)
{
    auto newCurve = curve->duplicate(CopyMode::WithDifferentIds, this);
    m_referenceCurves.emplace_back(newCurve);

    auto parentProduct = product();
    if (parentProduct && curve->measureTask())
    {
        parentProduct->copyReferenceCurveData(curve->measureTask()->product(), curve, newCurve);
    }

    return newCurve;
}

void AbstractMeasureTask::removeReferenceCurve(ReferenceCurve* curve)
{
    if (!curve || curve->used())
    {
        return;
    }
    auto it = std::find(m_referenceCurves.begin(), m_referenceCurves.end(), curve);
    if (it != m_referenceCurves.end())
    {
        m_referenceCurves.erase(it);
        curve->deleteLater();

        // no need to remove reference data from product, this is done automatically on save
    }
}

ReferenceCurve* AbstractMeasureTask::findReferenceCurve(const QUuid& uuid)
{
    if (uuid.isNull())
    {
        return nullptr;
    }

    const auto it = std::find_if(m_referenceCurves.begin(), m_referenceCurves.end(), [&uuid] (auto curve) {return curve->uuid() == uuid;});
    if (it == m_referenceCurves.end())
    {
        return nullptr;
    }

    return *it;
}

void AbstractMeasureTask::setSubGraphs(const std::vector<QUuid> &subGraphs)
{
    setGraphReference(SubGraphReference{subGraphs});
}

QColor AbstractMeasureTask::levelColor(uint level)
{
    switch (level)
    {
        case 0:
            return QColor("#75D480");
        case 1:
            return QColor("#E6D453");
        case 2:
            return QColor("#E68E73");
        default:
            return QColor("white");
    }
}

QVariantList AbstractMeasureTask::subGraphsAsList() const
{
    QVariantList ret;
    for (const auto &id : subGraphs())
    {
        ret << id;
    }
    return ret;
}

QVariantList AbstractMeasureTask::referenceCurveList() const
{
    QVariantList curves;

    std::transform(m_referenceCurves.begin(), m_referenceCurves.end(), std::back_inserter(curves), [] (auto curve) { return QVariant::fromValue(curve); });

    return curves;
}
}
}
