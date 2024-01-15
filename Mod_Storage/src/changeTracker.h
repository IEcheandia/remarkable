#pragma once
#include <QJsonArray>
#include <QJsonObject>
#include <QString>
#include <QVariant>

namespace precitec
{
namespace storage
{

class Parameter;
class ParameterSet;
class Seam;
class SeamInterval;
class SeamSeries;

class ChangeTracker
{
public:
    virtual ~ChangeTracker();

    QJsonObject json() const
    {
        return m_json;
    }

protected:
    void setJson(QJsonObject &&json);

private:
    QJsonObject m_json;
};

class PropertyChange : public ChangeTracker
{
public:
    PropertyChange(const QString &name, const QVariant &oldValue, const QVariant &newValue);
    PropertyChange(const QString &name);

    QString name() const
    {
        return m_name;
    }
    QVariant oldValue() const
    {
        return m_oldValue;
    }
    QVariant newValue() const
    {
        return m_newValue;
    }

private:
    QJsonObject toJson() const;
    QJsonObject toJsonNameOnly() const;

    QString m_name;
    QVariant m_oldValue;
    QVariant m_newValue;
};

class FilterParameterSetRemovedChange : public ChangeTracker
{
public:
    FilterParameterSetRemovedChange(ParameterSet *set);

private:
    QJsonObject toJson() const;

    QJsonObject m_json;
};

class FilterParameterSetAddedChange : public ChangeTracker
{
public:
    FilterParameterSetAddedChange(ParameterSet *set);

private:
    QJsonObject toJson() const;

    QJsonObject m_json;
};

class FilterParameterSetReplacedChange : public ChangeTracker
{
public:
    FilterParameterSetReplacedChange(ParameterSet *old, ParameterSet *set);

private:
    QJsonObject toJson() const;

    QJsonObject m_old;
    QJsonObject m_new;
};

class HardwareParametersCreatedChange : public ChangeTracker
{
public:
    HardwareParametersCreatedChange(ParameterSet *set);

private:
    QJsonObject toJson() const;

    QJsonObject m_json;
};

class HardwareParameterSetReplacedChange : public ChangeTracker
{
public:
    HardwareParameterSetReplacedChange(ParameterSet *old, ParameterSet *set);

private:
    QJsonObject toJson() const;

    QJsonObject m_old;
    QJsonObject m_new;
};

class ErrorParamatersCreatedChange : public ChangeTracker
{
public:
    ErrorParamatersCreatedChange(ParameterSet *set);

private:
    QJsonObject toJson() const;

    QJsonObject m_json;
};

class ErrorParamaterSetReplacedChange : public ChangeTracker
{
public:
    ErrorParamaterSetReplacedChange(ParameterSet *old, ParameterSet *set);

private:
    QJsonObject toJson() const;

    QJsonObject m_old;
    QJsonObject m_new;
};


class SeamRemovedChange : public ChangeTracker
{
public:
    SeamRemovedChange(Seam *seam);

private:
    QJsonObject toJson() const;

    QJsonObject m_seam;
    QJsonArray m_modifications;
};

class SeamCreatedChange : public ChangeTracker
{
public:
    SeamCreatedChange(Seam *seam);

private:
    QJsonObject toJson() const;

    QJsonObject m_seam;
};

class SeamSeriesRemovedChange : public ChangeTracker
{
public:
    SeamSeriesRemovedChange(SeamSeries *s);

private:
    QJsonObject toJson() const;

    QJsonObject m_seamSeries;
    QJsonArray m_modifications;
};

class SeamSeriesCreatedChange : public ChangeTracker
{
public:
    SeamSeriesCreatedChange(SeamSeries *s);

private:
    QJsonObject toJson() const;

    QJsonObject m_seamSeries;
};

class SeamIntervalRemovedChange : public ChangeTracker
{
public:
    SeamIntervalRemovedChange(SeamInterval *seamInterval);

private:
    QJsonObject toJson() const;

    QJsonObject m_seamInterval;
    QJsonArray m_modifications;
};

class SeamIntervalCreatedChange : public ChangeTracker
{
public:
    SeamIntervalCreatedChange(SeamInterval *interval);

private:
    QJsonObject toJson() const;

    QJsonObject m_seamInterval;
};

class ParameterCreatedChange : public ChangeTracker
{
public:
    ParameterCreatedChange(Parameter *parameter);

private:
    QJsonObject toJson() const;

    QJsonObject m_parameter;
};

class ParameterRemovedChange : public ChangeTracker
{
public:
    ParameterRemovedChange(Parameter *parameter);

private:
    QJsonObject toJson() const;

    QJsonObject m_parameter;
    QJsonArray m_modifications;
};

}
}
