#include "changeTracker.h"
#include "parameter.h"
#include "parameterSet.h"
#include "seam.h"
#include "seamInterval.h"
#include "seamSeries.h"

#include <QDebug>

namespace precitec
{
namespace storage
{

ChangeTracker::~ChangeTracker() = default;

void ChangeTracker::setJson(QJsonObject &&json)
{
    m_json = std::move(json);
}

PropertyChange::PropertyChange(const QString &name, const QVariant &oldValue, const QVariant &newValue)
    : ChangeTracker()
    , m_name(name)
    , m_oldValue(oldValue)
    , m_newValue(newValue)
{
    setJson(toJson());
}

PropertyChange::PropertyChange(const QString &name)
    : ChangeTracker()
    , m_name(name)
{
    setJson(toJsonNameOnly());
}

QJsonObject PropertyChange::toJson() const
{
    return {
        qMakePair(QStringLiteral("description"), QStringLiteral("PropertyChange")),
        qMakePair(QStringLiteral("change"), QJsonObject{
            qMakePair(QStringLiteral("propertyName"), m_name),
            qMakePair(QStringLiteral("oldValue"), QJsonValue::fromVariant(m_oldValue)),
            qMakePair(QStringLiteral("newValue"), QJsonValue::fromVariant(m_newValue))
        })
    };
}

QJsonObject PropertyChange::toJsonNameOnly() const
{
    return {
        qMakePair(QStringLiteral("description"), QStringLiteral("PropertyChange")),
        qMakePair(QStringLiteral("change"), QJsonObject{
            qMakePair(QStringLiteral("propertyName"), m_name)
        })
    };
}

FilterParameterSetRemovedChange::FilterParameterSetRemovedChange(ParameterSet *set)
    : ChangeTracker()
    , m_json(set->toJson())
{
    setJson(toJson());
}

QJsonObject FilterParameterSetRemovedChange::toJson() const
{
    return {
        qMakePair(QStringLiteral("description"), QStringLiteral("FilterParameterSetRemovedChange")),
        qMakePair(QStringLiteral("change"), m_json)
    };
}

FilterParameterSetAddedChange::FilterParameterSetAddedChange(ParameterSet *set)
    : ChangeTracker()
    , m_json(set->toJson())
{
    setJson(toJson());
}

QJsonObject FilterParameterSetAddedChange::toJson() const
{
    return {
        qMakePair(QStringLiteral("description"), QStringLiteral("FilterParameterSetAddedChange")),
        qMakePair(QStringLiteral("change"), m_json)
    };
}

FilterParameterSetReplacedChange::FilterParameterSetReplacedChange(ParameterSet *old, ParameterSet *set)
    : ChangeTracker()
    , m_old(old->toJson())
    , m_new(set->toJson())
{
    setJson(toJson());
}

QJsonObject FilterParameterSetReplacedChange::toJson() const
{
    return {
        qMakePair(QStringLiteral("description"), QStringLiteral("FilterParameterSetReplacedChange")),
        qMakePair(QStringLiteral("change"), QJsonObject{
            qMakePair(QStringLiteral("old"), m_old),
            qMakePair(QStringLiteral("new"), m_new)
        })
    };
}

HardwareParametersCreatedChange::HardwareParametersCreatedChange(ParameterSet *set)
    : ChangeTracker()
    , m_json(set->toJson())
{
    setJson(toJson());
}

QJsonObject HardwareParametersCreatedChange::toJson() const
{
    return {
        qMakePair(QStringLiteral("description"), QStringLiteral("HardwareParametersCreatedChange")),
        qMakePair(QStringLiteral("change"), m_json)
    };
}

HardwareParameterSetReplacedChange::HardwareParameterSetReplacedChange(ParameterSet *old, ParameterSet *set)
    : ChangeTracker()
    , m_old(old ? old->toJson() : QJsonObject{})
    , m_new(set ? set->toJson() : QJsonObject{})
{
    setJson(toJson());
}

QJsonObject HardwareParameterSetReplacedChange::toJson() const
{
    return {
        qMakePair(QStringLiteral("description"), QStringLiteral("HardwareParameterSetReplacedChange")),
        qMakePair(QStringLiteral("change"), QJsonObject{
            qMakePair(QStringLiteral("old"), m_old),
            qMakePair(QStringLiteral("new"), m_new)
        })
    };
}

ErrorParamatersCreatedChange::ErrorParamatersCreatedChange(ParameterSet *set)
    : ChangeTracker()
    , m_json(set->toJson())
{
    setJson(toJson());
}

QJsonObject ErrorParamatersCreatedChange::toJson() const
{
    return {
        qMakePair(QStringLiteral("description"), QStringLiteral("ErrorParamatersCreatedChange")),
        qMakePair(QStringLiteral("change"), m_json)
    };
}

ErrorParamaterSetReplacedChange::ErrorParamaterSetReplacedChange(ParameterSet *old, ParameterSet *set)
    : ChangeTracker()
    , m_old(old->toJson())
    , m_new(set->toJson())
{
    setJson(toJson());
}

QJsonObject ErrorParamaterSetReplacedChange::toJson() const
{
    return {
        qMakePair(QStringLiteral("description"), QStringLiteral("ErrorParamaterSetReplacedChange")),
        qMakePair(QStringLiteral("change"), QJsonObject{
            qMakePair(QStringLiteral("old"), m_old),
            qMakePair(QStringLiteral("new"), m_new)
        })
    };
}

SeamRemovedChange::SeamRemovedChange(Seam* seam)
    : ChangeTracker()
    , m_seam(seam->toJson())
    , m_modifications(seam->changes())
{
    setJson(toJson());
}

QJsonObject SeamRemovedChange::toJson() const
{
    return {
        qMakePair(QStringLiteral("description"), QStringLiteral("SeamRemovedChange")),
        qMakePair(QStringLiteral("change"), QJsonObject{
                qMakePair(QStringLiteral("seam"), m_seam),
                qMakePair(QStringLiteral("modifications"), m_modifications)
            })
    };
}

SeamCreatedChange::SeamCreatedChange(Seam* seam)
    : ChangeTracker()
    , m_seam(seam->toJson())
{
    setJson(toJson());
}

QJsonObject SeamCreatedChange::toJson() const
{
    return {
        qMakePair(QStringLiteral("description"), QStringLiteral("SeamCreatedChange")),
        qMakePair(QStringLiteral("change"), m_seam)
    };
}

SeamSeriesRemovedChange::SeamSeriesRemovedChange(SeamSeries* s)
    : ChangeTracker()
    , m_seamSeries(s->toJson())
    , m_modifications(s->changes())
{
    setJson(toJson());
}

QJsonObject SeamSeriesRemovedChange::toJson() const
{
    return {
        qMakePair(QStringLiteral("description"), QStringLiteral("SeamSeriesRemovedChange")),
        qMakePair(QStringLiteral("change"), QJsonObject{
                qMakePair(QStringLiteral("seamSeries"), m_seamSeries),
                qMakePair(QStringLiteral("modifications"), m_modifications)
            })
    };
}

SeamSeriesCreatedChange::SeamSeriesCreatedChange(SeamSeries* s)
    : ChangeTracker()
    , m_seamSeries(s->toJson())
{
    setJson(toJson());
}

QJsonObject SeamSeriesCreatedChange::toJson() const
{
    return {
        qMakePair(QStringLiteral("description"), QStringLiteral("SeamSeriesCreatedChange")),
        qMakePair(QStringLiteral("change"), m_seamSeries)
    };
}

SeamIntervalRemovedChange::SeamIntervalRemovedChange(SeamInterval *seamInterval)
    : ChangeTracker()
    , m_seamInterval(seamInterval->toJson())
    , m_modifications(seamInterval->changes())
{
    setJson(toJson());
}

QJsonObject SeamIntervalRemovedChange::toJson() const
{
    return {
        qMakePair(QStringLiteral("description"), QStringLiteral("SeamIntervalRemovedChange")),
        qMakePair(QStringLiteral("change"), QJsonObject{
                qMakePair(QStringLiteral("seam"), m_seamInterval),
                qMakePair(QStringLiteral("modifications"), m_modifications)
            })
    };
}

SeamIntervalCreatedChange::SeamIntervalCreatedChange(SeamInterval *interval)
    : ChangeTracker()
    , m_seamInterval(interval->toJson())
{
    setJson(toJson());
}

QJsonObject SeamIntervalCreatedChange::toJson() const
{
    return {
        qMakePair(QStringLiteral("description"), QStringLiteral("SeamIntervalCreatedChange")),
        qMakePair(QStringLiteral("change"), m_seamInterval)
    };
}

ParameterCreatedChange::ParameterCreatedChange(Parameter *parameter)
    : ChangeTracker()
    , m_parameter(parameter->toJson())
{
    setJson(toJson());
}

QJsonObject ParameterCreatedChange::toJson() const
{
    return {
        qMakePair(QStringLiteral("description"), QStringLiteral("ParameterCreatedChange")),
        qMakePair(QStringLiteral("change"), m_parameter)
    };
}

ParameterRemovedChange::ParameterRemovedChange(Parameter *parameter)
    : ChangeTracker()
    , m_parameter(parameter->toJson())
    , m_modifications(parameter->changes())
{
    setJson(toJson());
}

QJsonObject ParameterRemovedChange::toJson() const
{
    return {
        qMakePair(QStringLiteral("description"), QStringLiteral("ParameterRemovedChange")),
        qMakePair(QStringLiteral("change"), QJsonObject{
                qMakePair(QStringLiteral("parameter"), m_parameter),
                qMakePair(QStringLiteral("modifications"), m_modifications)
            })
    };
}

}
}
