#pragma once

#include <QObject>


namespace precitec
{
namespace storage
{
class AttributeModel;
class Attribute;
class ParameterSet;
class Parameter;
}
namespace gui
{

class HardwareParametersModule : public QObject
{
    Q_OBJECT

    /**
     * Hardware Attribute Model
     **/
    Q_PROPERTY(precitec::storage::AttributeModel* attributeModel READ attributeModel WRITE setAttributeModel NOTIFY attributeModelChanged)

public:
    explicit HardwareParametersModule(QObject* parent = nullptr);
    ~HardwareParametersModule() override;

    precitec::storage::AttributeModel* attributeModel() const
    {
        return m_attributeModel;
    }
    void setAttributeModel(precitec::storage::AttributeModel* model);

    precitec::storage::Parameter* findParameter(precitec::storage::ParameterSet* parameterSet, const QUuid& id) const;
    void updateParameter(precitec::storage::ParameterSet* parameterSet, const QUuid& id, const QVariant& value);
    void removeParameter(precitec::storage::ParameterSet* parameterSet, const QUuid& id);

Q_SIGNALS:
    void attributeModelChanged();

private:
    precitec::storage::Attribute* findAttribute(const QUuid& id) const;

    precitec::storage::AttributeModel* m_attributeModel = nullptr;
    QMetaObject::Connection m_attributeModelDestroyed;
};

}
}

Q_DECLARE_METATYPE(precitec::gui::HardwareParametersModule*)




