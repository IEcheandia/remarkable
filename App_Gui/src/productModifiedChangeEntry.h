#pragma once

#include <QJsonArray>
#include <QUuid>
#include <QVariant>

#include <precitec/change.h>

namespace precitec
{

namespace storage
{
class Parameter;
class ParameterSet;
class Seam;
}

namespace gui
{

class PropertyChange
{
    Q_GADGET
    Q_PROPERTY(QString name READ name CONSTANT)
    Q_PROPERTY(QVariant oldValue READ oldValue CONSTANT)
    Q_PROPERTY(QVariant newValue READ newValue CONSTANT)
    Q_PROPERTY(QString description READ description CONSTANT)
public:
    PropertyChange();
    PropertyChange(const QString &name, const QVariant &oldValue, const QVariant &newValue);

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

    QString description() const
    {
        return QStringLiteral("PropertyChange");
    }

private:
    QString m_name;
    QVariant m_oldValue;
    QVariant m_newValue;
};

class ParameterSetAddedRemovedChange
{
    Q_GADGET
    Q_PROPERTY(QString description READ description CONSTANT)
    Q_PROPERTY(precitec::storage::ParameterSet *parameterSet READ parameterSet CONSTANT)
public:
    ParameterSetAddedRemovedChange();
    ParameterSetAddedRemovedChange(const QString &description, precitec::storage::ParameterSet *parameterSet);

    QString description() const
    {
        return m_description;
    }

    precitec::storage::ParameterSet *parameterSet() const
    {
        return m_parameterSet;
    }

private:
    QString m_description;
    precitec::storage::ParameterSet *m_parameterSet = nullptr;
};

class ParameterSetReplacedChange
{
    Q_GADGET
    Q_PROPERTY(QString description READ description CONSTANT)
    Q_PROPERTY(precitec::storage::ParameterSet *oldParameterSet READ oldParameterSet CONSTANT)
    Q_PROPERTY(precitec::storage::ParameterSet *newParameterSet READ newParameterSet CONSTANT)
public:
    ParameterSetReplacedChange();
    ParameterSetReplacedChange(precitec::storage::ParameterSet *oldParameterSet, precitec::storage::ParameterSet *newParameterSet);

    QString description() const
    {
        return QStringLiteral("FilterParameterSetReplacedChange");
    }

    precitec::storage::ParameterSet *oldParameterSet() const
    {
        return m_oldParameterSet;
    }
    precitec::storage::ParameterSet *newParameterSet() const
    {
        return m_newParameterSet;
    }

private:
    precitec::storage::ParameterSet *m_oldParameterSet = nullptr;
    precitec::storage::ParameterSet *m_newParameterSet = nullptr;
};

class SeamCreatedChange
{
    Q_GADGET
    Q_PROPERTY(QString description READ description CONSTANT)
    Q_PROPERTY(precitec::storage::Seam *seam READ seam CONSTANT)
public:
    SeamCreatedChange();
    SeamCreatedChange(precitec::storage::Seam *seam);

    QString description() const
    {
        return QStringLiteral("SeamCreatedChange");
    }

    precitec::storage::Seam *seam() const
    {
        return m_seam;
    }

private:
    precitec::storage::Seam *m_seam = nullptr;
};

class SeamRemovedChange
{
    Q_GADGET
    Q_PROPERTY(QString description READ description CONSTANT)
    Q_PROPERTY(precitec::storage::Seam *seam READ seam CONSTANT)
public:
    SeamRemovedChange();
    SeamRemovedChange(precitec::storage::Seam *seam);

    QString description() const
    {
        return QStringLiteral("SeamRemovedChange");
    }

    precitec::storage::Seam *seam() const
    {
        return m_seam;
    }

private:
    precitec::storage::Seam *m_seam = nullptr;
};

class SeamModificationChange
{
    Q_GADGET
    Q_PROPERTY(int number READ number CONSTANT)
    Q_PROPERTY(QVariantList changes READ changes CONSTANT)
public:
    SeamModificationChange();
    SeamModificationChange(int number, const QVariantList &changes);

    int number() const
    {
        return m_number;
    }
    QVariantList changes() const
    {
        return m_changes;
    }

private:
    int m_number = 0;
    QVariantList m_changes;
};

class ParameterCreatedChange
{
    Q_GADGET
    Q_PROPERTY(precitec::storage::Parameter *parameter READ parameter CONSTANT)
    Q_PROPERTY(QString description READ description CONSTANT)
public:
    ParameterCreatedChange();
    ParameterCreatedChange(precitec::storage::Parameter *parameter);

    precitec::storage::Parameter *parameter() const
    {
        return m_parameter;
    }
    QString description() const
    {
        return QStringLiteral("ParameterCreatedChange");
    }

private:
    precitec::storage::Parameter *m_parameter = nullptr;
};

class ParameterModificationChange
{
    Q_GADGET
    Q_PROPERTY(precitec::storage::Parameter *parameter READ parameter CONSTANT)
    Q_PROPERTY(QVariantList changes READ changes CONSTANT)
    Q_PROPERTY(QString description READ description CONSTANT)
public:
    ParameterModificationChange();
    ParameterModificationChange(precitec::storage::Parameter *parameter, const QVariantList &changes);

    precitec::storage::Parameter *parameter() const
    {
        return m_parameter;
    }

    QVariantList changes() const
    {
        return m_changes;
    }

    QString description() const
    {
        return QStringLiteral("ParameterModificationChange");
    }

private:
    precitec::storage::Parameter *m_parameter = nullptr;
    QVariantList m_changes;
};

class HardwareParametersModificationChange
{
    Q_GADGET
    Q_PROPERTY(QVariantList changes READ changes CONSTANT)
    Q_PROPERTY(QString description READ description CONSTANT)
public:
    HardwareParametersModificationChange();
    HardwareParametersModificationChange(const QVariantList &changes);

    QVariantList changes() const
    {
        return m_changes;
    }
    QString description() const
    {
        return QStringLiteral("HardwareParametersModificationChange");
    }

private:
    QVariantList m_changes;
};

class ParameterSetModificationChange
{
    Q_GADGET
    Q_PROPERTY(QVariantList changes READ changes CONSTANT)
    Q_PROPERTY(QString description READ description CONSTANT)
    Q_PROPERTY(QUuid uuid READ uuid CONSTANT)
public:
    ParameterSetModificationChange();
    ParameterSetModificationChange(const QUuid &uuid, const QVariantList &changes);

    QVariantList changes() const
    {
        return m_changes;
    }
    QString description() const
    {
        return QStringLiteral("ParameterSetModificationChange");
    }
    QUuid uuid() const
    {
        return m_uuid;
    }

private:
    QVariantList m_changes;
    QUuid m_uuid;
};

class ProductModifiedChangeEntry : public components::userLog::Change
{
    Q_OBJECT
    Q_PROPERTY(QVariantList productChanges READ productChanges CONSTANT)
    Q_PROPERTY(QVariantList seamChanges READ seamChanges CONSTANT)
    Q_PROPERTY(QUuid uuid READ uuid CONSTANT)
    Q_PROPERTY(QString productName READ productName CONSTANT)
    Q_PROPERTY(int productType READ productType CONSTANT)
public:
    Q_INVOKABLE ProductModifiedChangeEntry(QObject *parent = nullptr);
    ProductModifiedChangeEntry(QJsonArray &&changes, QObject *parent = nullptr);
    ~ProductModifiedChangeEntry() override;

    void initFromJson(const QJsonObject &data) override;
    QJsonObject data() const override;

    QUrl detailVisualization() const override;

    QVariantList productChanges() const
    {
        return m_productChanges;
    }

    QVariantList seamChanges() const
    {
        return m_seamChanges;
    }

    QUuid uuid() const
    {
        return m_uuid;
    }

    QString productName() const
    {
        return m_productName;
    }

    int productType() const
    {
        return m_productType;
    }

private:
    void parseSeamSeries(const QJsonArray &seamSeries);
    QVariant parseChange(const QJsonObject &change);
    QVariant parsePropertyChange(const QJsonObject &change);
    QVariant parseSeamRemovedChange(const QJsonObject &change);
    QVariantList parseSeamChanges(const QJsonArray &seam);
    QVariantList parseParameterSet(const QJsonArray &parameterSet);
    QVariantList parseParameters(const QJsonArray &parameterSet);
    QJsonArray m_changes;
    QVariantList m_productChanges;
    QVariantList m_seamChanges;
    QUuid m_uuid;
    QString m_productName;
    int m_productType = 0;
};

}
}

Q_DECLARE_METATYPE(precitec::gui::PropertyChange)
Q_DECLARE_METATYPE(precitec::gui::ParameterSetAddedRemovedChange)
Q_DECLARE_METATYPE(precitec::gui::ParameterSetReplacedChange)
Q_DECLARE_METATYPE(precitec::gui::SeamRemovedChange)
Q_DECLARE_METATYPE(precitec::gui::SeamModificationChange)
Q_DECLARE_METATYPE(precitec::gui::ParameterModificationChange)
Q_DECLARE_METATYPE(precitec::gui::HardwareParametersModificationChange)
Q_DECLARE_METATYPE(precitec::gui::ParameterSetModificationChange)
Q_DECLARE_METATYPE(precitec::gui::ParameterCreatedChange)
