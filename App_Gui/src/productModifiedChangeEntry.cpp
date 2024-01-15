#include "productModifiedChangeEntry.h"

#include "parameterSet.h"
#include "parameter.h"
#include "seam.h"

namespace precitec
{
namespace gui
{

PropertyChange::PropertyChange() = default;

PropertyChange::PropertyChange(const QString &name, const QVariant &oldValue, const QVariant &newValue)
    : m_name(name)
    , m_oldValue(oldValue)
    , m_newValue(newValue)
{
}

ParameterSetAddedRemovedChange::ParameterSetAddedRemovedChange() = default;

ParameterSetAddedRemovedChange::ParameterSetAddedRemovedChange(const QString &description, precitec::storage::ParameterSet *parameterSet)
    : m_description(description)
    , m_parameterSet(parameterSet)
{
}

ParameterSetReplacedChange::ParameterSetReplacedChange() = default;

ParameterSetReplacedChange::ParameterSetReplacedChange(precitec::storage::ParameterSet *oldParameterSet, precitec::storage::ParameterSet *newParameterSet)
    : m_oldParameterSet(oldParameterSet)
    , m_newParameterSet(newParameterSet)
{
}

SeamCreatedChange::SeamCreatedChange() = default;

SeamCreatedChange::SeamCreatedChange(precitec::storage::Seam *seam)
    : m_seam(seam)
{
}

SeamRemovedChange::SeamRemovedChange() = default;

SeamRemovedChange::SeamRemovedChange(precitec::storage::Seam *seam)
    : m_seam(seam)
{
}

SeamModificationChange::SeamModificationChange() = default;

SeamModificationChange::SeamModificationChange(int number, const QVariantList &changes)
    : m_number(number)
    , m_changes(changes)
{
}

ParameterModificationChange::ParameterModificationChange() = default;

ParameterModificationChange::ParameterModificationChange(precitec::storage::Parameter *parameter, const QVariantList &changes)
    : m_parameter(parameter)
    , m_changes(changes)
{
}

ProductModifiedChangeEntry::ProductModifiedChangeEntry(QObject *parent)
    : components::userLog::Change(parent)
{
}

HardwareParametersModificationChange::HardwareParametersModificationChange() = default;
HardwareParametersModificationChange::HardwareParametersModificationChange(const QVariantList &changes)
    : m_changes(changes)
{
}

ParameterSetModificationChange::ParameterSetModificationChange() = default;
ParameterSetModificationChange::ParameterSetModificationChange(const QUuid &uuid, const QVariantList &changes)
    : m_changes(changes)
    , m_uuid(uuid)
{
}

ParameterCreatedChange::ParameterCreatedChange() = default;
ParameterCreatedChange::ParameterCreatedChange(precitec::storage::Parameter *parameter)
    : m_parameter(parameter)
{
}

ProductModifiedChangeEntry::ProductModifiedChangeEntry(QJsonArray &&changes, QObject *parent)
    : components::userLog::Change(parent)
    , m_changes(std::move(changes))
{
    setMessage(tr("Product modified"));
}

ProductModifiedChangeEntry::~ProductModifiedChangeEntry() = default;

void ProductModifiedChangeEntry::initFromJson(const QJsonObject &data)
{
    auto it = data.find(QStringLiteral("changes"));
    if (it != data.end())
    {
        m_changes = (*it).toArray();
    }
    for (auto it = m_changes.begin(); it != m_changes.end(); it++)
    {
        const auto object = it->toObject();
        // on top level we either have changes, seamSeries, hardwareParameters or parameterSets
        if (object.contains(QLatin1String("seamSeries")))
        {
            const auto seamSeries = object.value(QLatin1String("seamSeries")).toArray();
            for (auto it = seamSeries.begin(); it != seamSeries.end(); it++)
            {
                auto seamSeries = (*it).toObject();
                for (auto keyIt = seamSeries.begin(); keyIt != seamSeries.end(); keyIt++)
                {
                    parseSeamSeries(keyIt.value().toArray());
                }
            }
        } else if (object.contains(QLatin1String("hardwareParameters")))
        {
            m_productChanges.push_back(QVariant::fromValue(HardwareParametersModificationChange{parseParameterSet(object.value(QLatin1String("hardwareParameters")).toArray())}));
        } else if (object.contains(QLatin1String("parameterSets")))
        {
            const auto parameterSets = object.value(QLatin1String("parameterSets")).toArray();
            for (auto it = parameterSets.begin(); it != parameterSets.end(); it++)
            {
                auto parameterSet = (*it).toObject();
                for (auto keyIt = parameterSet.begin(); keyIt != parameterSet.end(); keyIt++)
                {
                    m_productChanges.push_back(QVariant::fromValue(ParameterSetModificationChange{{keyIt.key()}, parseParameterSet(keyIt.value().toArray())}));
                }
            }
        } else if (object.contains(QLatin1String("product")))
        {
            const auto productObject = object.value(QLatin1String("product")).toObject();
            auto it = productObject.find(QLatin1String("name"));
            if (it != productObject.end())
            {
                m_productName = it.value().toString();
            }
            it = productObject.find(QLatin1String("uuid"));
            if (it != productObject.end())
            {
                m_uuid = QUuid{it.value().toString()};
            }
            it = productObject.find(QLatin1String("type"));
            if (it != productObject.end())
            {
                m_productType = it.value().toInt();
            }
        } else
        {
            const auto parsedChange = parseChange(object);
            if (!parsedChange.isValid())
            {
                continue;
            }
            m_productChanges.push_back(parsedChange);
        }
    }
}

QVariantList ProductModifiedChangeEntry::parseParameterSet(const QJsonArray &parameterSet)
{
    QVariantList changesList;
    for (auto object : parameterSet)
    {
        const auto jsonObject = object.toObject();
        auto change = parseChange(jsonObject);
        if (change.isValid())
        {
            changesList.push_back(change);
        }
        auto parametersIt = jsonObject.find(QLatin1String("parameters"));
        if (parametersIt != jsonObject.end())
        {
            const auto parameters = parseParameters(parametersIt->toArray());
            std::copy(parameters.begin(), parameters.end(), std::back_inserter(changesList));
        }
    }
    return changesList;
}

QVariantList ProductModifiedChangeEntry::parseParameters(const QJsonArray &array)
{
    QVariantList retList;
    for (auto parameterChange : array)
    {
        const auto parameterChangeObject = parameterChange.toObject();
        const auto changesIt = parameterChangeObject.find(QLatin1String("changes"));
        QVariantList parameterChanges;
        if (changesIt != parameterChangeObject.end())
        {
            for (auto changeValue : changesIt->toArray())
            {
                auto change = parseChange(changeValue.toObject());
                if (change.isValid())
                {
                    parameterChanges.push_back(change);
                }
            }
        }
        const auto parameterIt = parameterChangeObject.find(QLatin1String("parameter"));
        if (parameterIt != parameterChangeObject.end())
        {
            auto parameter = precitec::storage::Parameter::fromJson(parameterIt->toObject(), static_cast<storage::ParameterSet*>(nullptr));
            if (parameter)
            {
                parameter->setParent(this);
                retList << QVariant::fromValue(ParameterModificationChange{parameter, parameterChanges});
            }
        }
    }
    return retList;
}

void ProductModifiedChangeEntry::parseSeamSeries(const QJsonArray &seamSeries)
{
    for (auto it = seamSeries.begin(); it != seamSeries.end(); it++)
    {
        const auto object = it->toObject();
        if (object.contains(QLatin1String("seam")))
        {
            const auto seams = object.value(QLatin1String("seam")).toArray();
            for (auto it = seams.begin(); it != seams.end(); it++)
            {
                auto seamObject = (*it).toObject();
                for (auto keyIt = seamObject.begin(); keyIt != seamObject.end(); keyIt++)
                {
                    m_seamChanges.push_back(QVariant::fromValue(SeamModificationChange{keyIt.key().toInt(), parseSeamChanges(keyIt.value().toArray())}));
                }
            }
        } else if (object.contains(QLatin1String("change")))
        {
            const auto change = parseChange(object);
            if (change.isValid())
            {
                m_productChanges.push_back(change);
            }
        }
    }
}

QVariantList ProductModifiedChangeEntry::parseSeamChanges(const QJsonArray &seam)
{
    QVariantList retList;
    for (auto s : seam)
    {
        const auto object = s.toObject();
        const auto var = parseChange(object);
        if (var.isValid())
        {
            retList.push_back(var);
        } else
        {
            auto intervalIt = object.find(QLatin1String("interval"));
            if (intervalIt != object.end())
            {
                auto intervalArray = intervalIt.value().toArray();
                // simplification for track compact: we assume there is only one interval, so changes on interval are considered changes on seam
                for (auto it = intervalArray.begin(); it != intervalArray.end(); it++)
                {
                    auto intervalObject = (*it).toObject();
                    for (auto keyIt = intervalObject.begin(); keyIt != intervalObject.end(); keyIt++)
                    {
                        for (auto value : keyIt.value().toArray())
                        {
                            const auto var = parseChange(value.toObject());
                            if (var.isValid())
                            {
                                retList.push_back(var);
                            }
                        }
                    }
                }
            }
            // hardware parameters
            const auto hardwareParametersIt = object.find(QLatin1String("hardwareParameters"));
            if (hardwareParametersIt != object.end())
            {
                const auto hardwareParameters = parseParameterSet(hardwareParametersIt->toArray());
                if (!hardwareParameters.isEmpty())
                {
                    retList.push_back(QVariant::fromValue(HardwareParametersModificationChange{hardwareParameters}));
                }
            }
        }
    }
    return retList;
}

QVariant ProductModifiedChangeEntry::parseChange(const QJsonObject &change)
{
    auto changeIt = change.find(QLatin1String("change"));
    if (changeIt == change.end())
    {
        return {};
    }
    auto descriptionIt = change.find(QLatin1String("description"));
    if (descriptionIt != change.end())
    {
        const auto description = descriptionIt->toString();
        if (description == QLatin1String("PropertyChange"))
        {
            return parsePropertyChange(changeIt->toObject());
        } else if (description == QLatin1String("FilterParameterSetAddedChange")
                || description == QLatin1String("FilterParameterSetRemovedChange")
                || description == QLatin1String("HardwareParametersCreatedChange"))
        {
            auto parameterSet = precitec::storage::ParameterSet::fromJson(changeIt->toObject(), this);
            return QVariant::fromValue(ParameterSetAddedRemovedChange{description, parameterSet});
        } else if (description == QLatin1String("FilterParameterSetReplacedChange"))
        {
            auto object = changeIt->toObject();
            auto oldIt = object.find(QLatin1String("old"));
            auto newIt = object.find(QLatin1String("new"));
            if (oldIt != object.end() && newIt != object.end())
            {
                auto oldParameterSet = precitec::storage::ParameterSet::fromJson(oldIt->toObject(), this);
                auto newParameterSet = precitec::storage::ParameterSet::fromJson(newIt->toObject(), this);
                return QVariant::fromValue(ParameterSetReplacedChange{oldParameterSet, newParameterSet});
            }
        } else if (description == QLatin1String("SeamCreatedChange"))
        {
            auto seam = precitec::storage::Seam::fromJson(changeIt->toObject(), nullptr);
            if (seam)
            {
                seam->setParent(this);
            }
            return QVariant::fromValue(SeamCreatedChange{seam});
        } else if (description == QLatin1String("SeamRemovedChange"))
        {
            return parseSeamRemovedChange(changeIt->toObject());
        } else if (description == QLatin1String("ParameterCreatedChange"))
        {
            auto parameter = precitec::storage::Parameter::fromJson(changeIt->toObject(), static_cast<storage::ParameterSet*>(nullptr));
            if (parameter)
            {
                parameter->setParent(this);
                return QVariant::fromValue(ParameterCreatedChange{parameter});
            }
        }
        // TODO: HardwareParameterSetReplacedChange
        // TODO: SeamIntervalCreatedChange
        // TODO: ParameterRemovedChange
        // TODO: SeamSeriesRemovedChange
        // TODO: SeamSeriesCreatedChange
    }
    return {};
}

QVariant ProductModifiedChangeEntry::parsePropertyChange(const QJsonObject &change)
{
    auto propertyNameIt = change.find(QLatin1String("propertyName"));
    auto oldValueIt = change.find(QLatin1String("oldValue"));
    auto newValueIt = change.find(QLatin1String("newValue"));
    if (propertyNameIt == change.end() || oldValueIt == change.end() || newValueIt == change.end())
    {
        return {};
    }
    return QVariant::fromValue(PropertyChange{propertyNameIt->toString(), oldValueIt->toVariant(), newValueIt->toVariant()});
}

QVariant ProductModifiedChangeEntry::parseSeamRemovedChange(const QJsonObject &change)
{
    auto seamIt = change.find(QLatin1String("seam"));
    if (seamIt == change.end())
    {
        return {};
    }
    // TODO: modifications
    auto seam = precitec::storage::Seam::fromJson(seamIt->toObject(), nullptr);
    if (seam)
    {
        seam->setParent(this);
    }
    return QVariant::fromValue(SeamRemovedChange{seam});
}

QJsonObject ProductModifiedChangeEntry::data() const
{
    return {qMakePair(QStringLiteral("changes"), m_changes)};
}

QUrl ProductModifiedChangeEntry::detailVisualization() const
{
    return QStringLiteral("qrc:///resources/qml/userLog/ProductModifiedChangeEntry.qml");
}

}
}
